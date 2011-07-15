/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "DataAccessor.h"
#include "DataAccessorImpl.h"
#include "DataRequest.h"
#include "DesktopServices.h"
#include "MessageLogResource.h"
#include "ObjectResource.h"
#include "PlugInArgList.h"
#include "PlugInManagerServices.h"
#include "PlugInRegistration.h"
#include "Progress.h"
#include "RasterDataDescriptor.h"
#include "RasterElement.h"
#include "RasterUtilities.h"
#include "SpatialDataView.h"
#include "SpatialDataWindow.h"
#include "switchOnEncoding.h"
#include "Deconvolution.h"
#include "DeconvolutionDlg.h"

#include <limits>

REGISTER_PLUGIN_BASIC(OpticksAstronomy, Deconvolution);

namespace
{
   #define CONVERGENCE_THRESHOLD 0.05
   #define MAX_WINDOW_SIZE 7
   #define MAX_ITERATION_NUMBER 20
   #define PI_VALUE 3.1415926
   
   //2 dimension Gaussian function
   double GaussianFunc2D(double x, double y, double sigmaVal)
   {
       double retVal;
       
       retVal = exp(-(x/sigmaVal)*(x/sigmaVal)/2-(y/sigmaVal)*(y/sigmaVal)/2);

       retVal = retVal/(2*PI_VALUE*sigmaVal*sigmaVal);

	   return retVal;
   }
   
   //Correction function used during deconvolution
   double CorrectFunc(double inputVal, double gamaVal, double minGrayVal, double maxGrayVal)
   {
   	   double retVal;
   	   
       if (inputVal <= minGrayVal)
       {
           return 0;
       }
       
       if (inputVal >= maxGrayVal)
       {
           return 1;
       }
       

        retVal = (inputVal - minGrayVal)/(maxGrayVal - minGrayVal);
        retVal = pow(retVal, gamaVal);
        
        return retVal;
   }
   
   void GetGrayScale(double *t1, double *t2, EncodingType type)
   {
   	double minGrayVal = 0.0;
   	double maxGrayVal = 255.0;
   	
   	if (type == INT1UBYTE)
	  {
	  	maxGrayVal = 255;
	  	minGrayVal = 0;
	  }
		
		if (type == INT1SBYTE)
	  {
	  	maxGrayVal = 127;
	  	minGrayVal = -128;
	  }
	  
	  if (type == INT2UBYTES)
	  {
	  	maxGrayVal = 65535;
	  	minGrayVal = 0;
	  }
	  
	  if (type == INT2SBYTES)
	  {
	  	maxGrayVal = 32767;
	  	minGrayVal = -32768;
	  }
	  
	  *t1 = minGrayVal;
	  *t2 = maxGrayVal;
	 }
   
   void InitializeData(DataAccessor pSrcAcc, double *pBuffer, double *pOrigData, unsigned int rows, unsigned int cols, EncodingType type)
   {
	  double pixelVal;
	  unsigned int nCount = 0;


	  for (unsigned int i=0; i<rows; i++)
	  {
		  for (unsigned int j=0; j<cols; j++)
		  {
			  pSrcAcc->toPixel(i, j);
              VERIFYNRV(pSrcAcc.isValid());
		      pixelVal = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);

			  *(pBuffer + nCount) = pixelVal;
			  *(pOrigData + nCount) = pixelVal;
			  nCount++;
		  }
	  }
   }

   template<typename T>
   void restoreImageValue(T* pData, double *pSrc)
   {
	   double pixelVal = *pSrc;

	   *pData = static_cast<T>(pixelVal);
   }

   //Convolution function, using Gaussian function as point spread function
   void ConvolutionFunc(double *OrigData, double *ConvoData, int rowSize, int colSize, double sigmaVal, double minGrayVal, double maxGrayVal, int windowSize)
   {
	  int i, j, m, n, row, col;
	  int centerX = windowSize;
      int centerY = windowSize;
	  double distX, distY, pixelVal = 0.0;


	  double windowVal[MAX_WINDOW_SIZE*2+1][MAX_WINDOW_SIZE*2+1];
	  
	  for (row=0; row < rowSize; row++)
	  {
	      for (col=0; col < colSize; col++)
	      {
	          if ((col-windowSize < 0) || (col+windowSize > colSize - 1))
	          {
		          ConvoData[row*colSize + col] = OrigData[row*colSize + col];
                  continue;
              }

	          if ((row-windowSize < 0) || (row+windowSize > rowSize - 1))
	          {
		          ConvoData[row*colSize + col] = OrigData[row*colSize + col];
                  continue;
	          }

	          //Get the pixels within the window and perform convolution
	          m = 0;
	          pixelVal = 0;
	          
	          for (i=row - windowSize; i<= row + windowSize; i++)
	          {
		            n = 0;
		            for (j=col - windowSize; j<= col + windowSize; j++)
		            {
		            	  distY = abs(centerX-m);
                          distX = abs(centerY-n);
                    
			              windowVal[m][n] = OrigData[i*colSize+j];
			              windowVal[m][n] = windowVal[m][n]*GaussianFunc2D(distX, distY, sigmaVal);
			              
			              pixelVal = pixelVal + windowVal[m][n];
			              
			              n++;
		            }
		            m++;
	          }
	  
	          if (pixelVal > maxGrayVal)
	          {
	              pixelVal = maxGrayVal;
	          }
	          else if (pixelVal < minGrayVal)
	          {
	          	  pixelVal = minGrayVal;
	          }         
	           
	          ConvoData[row*colSize + col] = pixelVal; 
         }
      }
   }
   
   //Perform deconvolution
   double DeconvolutionFunc(double *OrigData, double *ImData, double *NewData, double *ConvoData, double sigmaVal, double gamaVal, 
                          int windowSize, int rows, int cols, int methodType, double maxGrayValue, double minGrayValue)
   {
   	   int i,j;
       double miniVal = -std::numeric_limits<double>::max();;
	   double temp;

       ConvolutionFunc(OrigData, ConvoData, rows, cols, sigmaVal, minGrayValue, maxGrayValue, windowSize);

       if (methodType == 0)
       {
           for (i=0; i<rows; i++)
           {
               for (j=0; j<cols; j++)
               {
                   NewData[i*cols+j] = OrigData[i*cols+j] + CorrectFunc(OrigData[i*cols+j],gamaVal,minGrayValue,maxGrayValue)*(ImData[i*cols+j] - ConvoData[i*cols+j]);

                   if (NewData[i*cols+j] > maxGrayValue)
                   {
                       NewData[i*cols+j] = maxGrayValue;
                   }
                   
                   if (NewData[i*cols+j] < minGrayValue)
                   {
                       NewData[i*cols+j] = minGrayValue;
                   }

           
                   temp = abs(NewData[i*cols+j] - OrigData[i*cols+j]);
                   if (temp > miniVal)
                   { 
                       miniVal = temp;
                   }
               }
           }
       }
       else
       {
           for (i=0; i<rows; i++)
           {
               for (j=0; j<cols; j++)
               {
                   if (ConvoData[i*cols+j] < 0.000001)
                   {
                       NewData[i*cols+j] = OrigData[i*cols+j]*(CorrectFunc(OrigData[i*cols+j],gamaVal,minGrayValue,maxGrayValue)*(ImData[i*cols+j]/0.000001-1)+1);
                   }
                   else
                   {
                       NewData[i*cols+j] = OrigData[i*cols+j]*(CorrectFunc(OrigData[i*cols+j],gamaVal,minGrayValue,maxGrayValue)*(ImData[i*cols+j]/ConvoData[i*cols+j]-1)+1);
                   }
           
                   if (NewData[i*cols+j] > maxGrayValue)
                   {
                       NewData[i*cols+j] = maxGrayValue;
                   }
                   
                   if (NewData[i*cols+j] < minGrayValue)
                   {
                       NewData[i*cols+j] = minGrayValue;
                   }

           
                   temp = abs(NewData[i*cols+j] - OrigData[i*cols+j]);
                   if (temp > miniVal)
                   { 
                       miniVal = temp;
                   }
               }
           }
       }
       
       return miniVal;
   }

};

Deconvolution::Deconvolution()
{
   setDescriptorId("{CC472EDD-2FD5-42E1-8138-DF1519A480D8}");
   setName("Deconvolution Enhancement");
   setDescription("Image Enhancement through deconvolution");
   setCreator("Yiwei Zhang");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Image Enhancement");
   setMenuLocation("[Astronomy]/Deconvolution");
   setAbortSupported(true);
   
   pOriginalImage = NULL;
}

Deconvolution::~Deconvolution()
{
	if (pOriginalImage != NULL)
		free(pOriginalImage);
}

bool Deconvolution::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Perform speckle remove on this data element");
   return true;
}

bool Deconvolution::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool Deconvolution::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Deconvolution Sharpening", "app", "619F3C8A-FB70-44E0-B211-B116E604EDDA");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   RasterElement* pCube = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());
   if (pCube == NULL)
   {
      std::string msg = "A raster cube must be specified.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL) 
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      return false;
   }
   RasterDataDescriptor* pDesc = static_cast<RasterDataDescriptor*>(pCube->getDataDescriptor());
   VERIFY(pDesc != NULL);
   EncodingType ResultType = pDesc->getDataType();
   if (pDesc->getDataType() == INT4SCOMPLEX)
   {
      ResultType = INT4SBYTES;
   }
   else if (pDesc->getDataType() == FLT8COMPLEX)
   {
      ResultType = FLT8BYTES;
   }

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   DataAccessor pSrcAcc = pCube->getDataAccessor(pRequest.release());

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube->getName() +
      "_Deconvolution_Sharpening_Result", pDesc->getRowCount(), pDesc->getColumnCount(), ResultType));
   if (pResultCube.get() == NULL)
   {
      std::string msg = "A raster cube could not be created.";
      pStep->finalize(Message::Failure, msg);
      if (pProgress != NULL) 
      {
         pProgress->updateProgress(msg, 0, ERRORS);
      }
      return false;
   }
   FactoryResource<DataRequest> pResultRequest;
   pResultRequest->setWritable(true);
   DataAccessor pDestAcc = pResultCube->getDataAccessor(pResultRequest.release());

   Service<DesktopServices> pDesktop;
   DeconvolutionDlg dlg(pDesktop->getMainWidget());
   int stat = dlg.exec();
   if (stat != QDialog::Accepted)
   {
	   return true;
   }

   double minGrayValue;
   double maxGrayValue;
   double deltaValue = 0.0;

   int nFilterType = dlg.getCurrentFilterType();
   int windowSize = dlg.getCurrentWindowSize();
   double sigmaVal = dlg.getSigmaValue();
   double gamaVal = dlg.getGamaValue();
   windowSize = (windowSize-1)/2;
   
   if (NULL != pOriginalImage)
   {
	   free(pOriginalImage);
   }
   pOriginalImage = (double *)malloc(sizeof(double)*pDesc->getRowCount()*pDesc->getColumnCount());
   
   double *OrigData = (double *)malloc(sizeof(double)*pDesc->getRowCount()*pDesc->getColumnCount());
   double *NewData  = (double *)malloc(sizeof(double)*pDesc->getRowCount()*pDesc->getColumnCount());
   double *ConvoData = (double *)malloc(sizeof(double)*pDesc->getRowCount()*pDesc->getColumnCount());
   double *pTempData;

   InitializeData(pSrcAcc, pOriginalImage, OrigData, pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType());
   GetGrayScale(&minGrayValue, &maxGrayValue, pDesc->getDataType());
   
   //Perform deconvolution iteratively
   for (int num = 0; num < MAX_ITERATION_NUMBER; num++)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Deconvolution process", num*100/MAX_ITERATION_NUMBER, NORMAL);
      }
      if (isAborted())
      {
         std::string msg = getName() + " has been aborted.";
         pStep->finalize(Message::Abort, msg);
         if (pProgress != NULL)
         {
            pProgress->updateProgress(msg, 0, ABORT);
         }
         
         free(OrigData);
         free(NewData);
         free(ConvoData);
         
         return false;
      }
      
      deltaValue = DeconvolutionFunc(OrigData, pOriginalImage, NewData, ConvoData, sigmaVal, gamaVal, 
                                     windowSize, pDesc->getRowCount(), pDesc->getColumnCount(), nFilterType, maxGrayValue, minGrayValue);


      pTempData = OrigData;
      OrigData = NewData;
      NewData = pTempData;

	  double errorRate = deltaValue/(maxGrayValue-minGrayValue);
	  if (errorRate < CONVERGENCE_THRESHOLD)
	  {
		  break;
	  }
   }
   
   free(NewData);
   free(ConvoData);


   //Output result
   unsigned int nCount = 0;
   for (int i = 0; i < pDesc->getRowCount(); i++)
   {
       for (int j = 0; j < pDesc->getColumnCount(); j++)		   
	   {		   
		   if (!pDestAcc.isValid())
           {       
		       std::string msg = "Unable to access the cube data.";        
			   pStep->finalize(Message::Failure, msg);
                       
			   if (pProgress != NULL)                      
			   {         
			       pProgress->updateProgress(msg, 0, ERRORS);       
			   }   
			   free(OrigData);                  
			   return false;              
		   }
			   
		   pDestAcc->toPixel(i, j);	
		   switchOnEncoding(ResultType, restoreImageValue, pDestAcc->getColumn(), (OrigData+nCount));
		   nCount++;

	   }
   }
   
   free(OrigData);  


   if (!isBatch())
   {
      Service<DesktopServices> pDesktop;

      SpatialDataWindow* pWindow = static_cast<SpatialDataWindow*>(pDesktop->createWindow(pResultCube->getName(),
         SPATIAL_DATA_WINDOW));

      SpatialDataView* pView = (pWindow == NULL) ? NULL : pWindow->getSpatialDataView();
      if (pView == NULL)
      {
         std::string msg = "Unable to create view.";
         pStep->finalize(Message::Failure, msg);
         if (pProgress != NULL) 
         {
            pProgress->updateProgress(msg, 0, ERRORS);
         }
         return false;
      }

      pView->setPrimaryRasterElement(pResultCube.get());
      pView->createLayer(RASTER, pResultCube.get());
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Deconvolution enhancement is complete.", 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("Deconvolution enhancement Result", pResultCube.release());

   pStep->finalize();


   return true;
}
