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
#include "ImageRegistration.h"
#include "StringUtilities.h"
#include "LayerList.h"
#include <limits>

REGISTER_PLUGIN_BASIC(OpticksAstronomy, ImageRegistration);

namespace
{

   #define MAX_WINDOW_SIZE 10
   #define MAX_DIFFERENCE_THRESHOLD 90
   #define MINIMUM_RADIUS_LIMIT  100
   #define MAX_STAR_NUMBERS  20

   #define MAX_MATCHING_ERROR  1000
   
   int nCountMas;
   int nStarPositionsMas[MAX_STAR_NUMBERS][2] = {0};
   long nStarListMas[MAX_STAR_NUMBERS][MAX_STAR_NUMBERS-1] = {0};
   double nStarGrayValueMas[MAX_STAR_NUMBERS] = {0};

   int nCountRef;
   int nStarPositionsRef[MAX_STAR_NUMBERS][2] = {0};
   long nStarListRef[MAX_STAR_NUMBERS][MAX_STAR_NUMBERS-1] = {0};
   double nStarGrayValueRef[MAX_STAR_NUMBERS] = {0};
   
   int nMatchingStarList[MAX_STAR_NUMBERS][3] = {0};

   
   void DrawStars(unsigned char *pBuffer, int rowSize, int colSize)
   {
   	   int i,j,index;
	   int nCount = std::min(nCountMas, nCountRef);

       for (int k=0; k<nCount; k++)
       {
           for (int m=-1; m<=1; m++)
           {
               for (int n=-1; n<=1; n++)
               {
				   index = nMatchingStarList[k][0];
                   i = nStarPositionsMas[index][0] + m;
                   j = nStarPositionsMas[index][1] + n;
               
                   if ((i >=0) && (i < rowSize) && (j >= 0) && (j < colSize))
                   {
                       pBuffer[i*colSize+j] = 255;
                   }
               }
           }
       }
   }
   
   template<typename T>
   void updatePixel(T* pData, unsigned char *pBuffer, int row, int col, int rowSize, int colSize)
   {
       
       unsigned char pixelVal = pBuffer[row*colSize+col];

	    *pData = static_cast<T>(pixelVal);

   }
   
   
   int FindMatchingStar(int nMasIndex, long *nError)
   {
       int nCount = nCountMas-1;
       int nRefIndex = -1;
       
       if (nCountRef < nCountMas)
       {
           nCount = nCountRef - 1;
       }
       
       long temp = 0;       
       long nErrorNumber = std::numeric_limits<long>::max();//MAX_MATCHING_ERROR*nCount;
       
       //Find reference star 
       for (int i=0; i<nCountRef; i++)
       {
       	   temp = 0;
       	   
       	   for (int j=0; j<nCount; j++)
       	   {
               temp = temp + abs(nStarListMas[nMasIndex][j] - nStarListRef[i][j]);
           }
           
           if (temp < nErrorNumber)
           {
               nErrorNumber = temp;
               nRefIndex = i;
           }
       }

	   *nError = nErrorNumber;
       
       return nRefIndex;
       
   }
   
   void GetMatchingStars()
   {
   	   long nError;
   	   int nRefIndex;
   	   int k, nIndex = -1;
   	   
       for (int i=0; i<nCountMas; i++)
       {
           nRefIndex = FindMatchingStar(i, &nError);
           
           k = 0;
           for (k=0; k<=nIndex; k++)
           {
               if (nMatchingStarList[k][2] > nError)
               {
                   break;
               }
           }
         
           for (int m=nIndex+1; m>k;m--)
           {
		       nMatchingStarList[m][0] = nMatchingStarList[m-1][0];
			   nMatchingStarList[m][1] = nMatchingStarList[m-1][1];
			   nMatchingStarList[m][2] = nMatchingStarList[m-1][2];
		   }
			   
		   nMatchingStarList[k][0] = i;
		   nMatchingStarList[k][1] = nRefIndex;
		   nMatchingStarList[k][2] = nError;
			     
		   nIndex++;
		     
		}
   }

   void GetNeighborStars(int nCount, int nStarPositions[][2], long nStarList[][MAX_STAR_NUMBERS-1])
   {
	   int currentRow, currentCol;
	   int nIndex;
	   int k;

	   for (int i=0; i<nCount; i++)
       {
		   currentRow = nStarPositions[i][0];
		   currentCol = nStarPositions[i][1];
		   nIndex = -1;

           for (int j=0; j<nCount; j++)
		   {
			   if (i==j)
			   {
				   continue;
			   }

			   int rowIndex = nStarPositions[j][0];
			   int colIndex = nStarPositions[j][1];
			   long  nDist = (rowIndex - currentRow)*(rowIndex - currentRow);
			   nDist += (colIndex - currentCol)*(colIndex - currentCol);

               k = 0;
               for (k=0; k<=nIndex; k++)
               {
                   if (nStarList[i][k] > nDist)
                   {
                       break;
                   }
               }
         
               for (int m=nIndex+1; m>k;m--)
               {
			       nStarList[i][m] = nStarList[i][m-1];
			   }
			   nStarList[i][k] = nDist;

			   nIndex++;
		   }
		   
       }
   }

   void GetAllNeighborStars()
   {
	   GetNeighborStars(nCountMas, nStarPositionsMas, nStarListMas);
	   GetNeighborStars(nCountRef, nStarPositionsRef, nStarListRef);
   }
   
   void ModifyCenter(DataAccessor pSrcAcc, EncodingType type, int windowSize, int nCount, int nStarPositions[][2])
   {
       int m, n, i, j;
       int row, col;
       int rowIndex, colIndex;
       
       double windowVal[MAX_WINDOW_SIZE*2+1][MAX_WINDOW_SIZE*2+1];
       
       for (int k=0; k<nCount; k++)
       {
		   double maxVal = -std::numeric_limits<double>::max();;

           row = nStarPositions[k][0];
           col = nStarPositions[k][1];
           
           m = 0;
           for (i=row - windowSize; i<= row + windowSize; i++)
	       {
		       n = 0;
		       for (j=col - windowSize; j<= col + windowSize; j++)
			   {
		           pSrcAcc->toPixel(i, j);
                   VERIFYNRV(pSrcAcc.isValid());
			 
			       windowVal[m][n] = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);
			 		 	
			       if (maxVal < windowVal[m][n])	
			       {	
				       maxVal = windowVal[m][n];
				       rowIndex = row - windowSize + m;
				       colIndex = col - windowSize + n;	
			       }
			       n++;
		       }
		       m++;
	       }
	         
	       nStarPositions[k][0] = rowIndex;
           nStarPositions[k][1] = colIndex;
	   }
   }

   
   int locateStarPosition(DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize, EncodingType type, int windowSize, int nCount, int nStarPositions[][2], double *nStarGrayValue)
   {
	  int i, j, m, n;
	  int rowIndex, colIndex;
	  double minVal = std::numeric_limits<double>::max();
      double maxVal = -minVal;
    
	  double pixelVal = 0.0;
	  double diffVal = 0.0;


	  double windowVal[MAX_WINDOW_SIZE*2+1][MAX_WINDOW_SIZE*2+1];
	  
	  if ((row < 0.2*rowSize) || (row > 0.8*rowSize))
	  	return nCount;
	  	
	  if ((col < 0.2*colSize) || (col > 0.8*colSize))
	  	return nCount;

	  //Get the pixels in the window
	  m = 0;
	  for (i=row - windowSize; i<= row + windowSize; i++)
	  {
		  n = 0;
		  for (j=col - windowSize; j<= col + windowSize; j++)
		  {
			 if ((i < 0) || (i >=rowSize))
			 {
				 continue;
			 }

			 if ((j < 0) || (j >=colSize))
			 {
				 continue;
			 }


			 pSrcAcc->toPixel(i, j);
             //VERIFYNRV(pSrcAcc.isValid());
			 windowVal[m][n] = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);
			 
			 if (minVal > windowVal[m][n])	
			 {
				 minVal = windowVal[m][n];	
			 }
			 	
			 	
			 if (maxVal < windowVal[m][n])	
			 {	
				 maxVal = windowVal[m][n];
				 rowIndex = row - windowSize + m;
				 colIndex = col - windowSize + n;	
			 }
			 n++;
		  }
		  m++;
	  }
	  
	  
	  if (abs(maxVal - minVal) > MAX_DIFFERENCE_THRESHOLD)
	  {
	      for (int k=0; k<nCount; k++)
	      {
	      	  if (k >= MAX_STAR_NUMBERS)
	      	  {
	      	  	   return nCount;
	      	  }
	      	  
	      	  double nDist = (rowIndex-nStarPositions[k][0])*(rowIndex-nStarPositions[k][0]);
	      	  nDist += (colIndex-nStarPositions[k][1])*(colIndex-nStarPositions[k][1]);
	          if (nDist < MINIMUM_RADIUS_LIMIT) 
	          {
	          	if (nStarGrayValue[k] < maxVal)
	          	{
	          	    nStarPositions[k][0] = rowIndex;
	                nStarPositions[k][1] = colIndex;
	                nStarGrayValue[k] = maxVal;
	            }	          		
	          	return nCount;
	          } 
	      }

	      
	      nStarPositions[nCount][0] = rowIndex;
	      nStarPositions[nCount][1] = colIndex;
	      nStarGrayValue[nCount] = maxVal;
	          
	      nCount++;   
	  }

	  return nCount;

   }

   void locateAllStarPosition(DataAccessor pSrcAccMas, DataAccessor pSrcAccRef, int row, int col, int rowSize, int colSize, EncodingType type, int windowSize)
   {
	   nCountMas = locateStarPosition(pSrcAccMas, row, col, rowSize, colSize, type, windowSize,  nCountMas, nStarPositionsMas, nStarGrayValueMas);

	   nCountRef = locateStarPosition(pSrcAccRef, row, col, rowSize, colSize, type, windowSize,  nCountRef, nStarPositionsRef, nStarGrayValueRef);
   }
   
};

ImageRegistration::ImageRegistration()
{
   setDescriptorId("{7F7AA7CF-428B-4D13-A69E-B0016EDFC63A}");
   setName("Image Registration");
   setDescription("Astronomical Image Registration");
   setCreator("Yiwei Zhang");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Image Registration");
   setMenuLocation("[Astronomy]/Image Registration");
   setAbortSupported(true);
}

ImageRegistration::~ImageRegistration()
{
}

bool ImageRegistration::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Perform image registration on this data element");
   return true;
}

bool ImageRegistration::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool ImageRegistration::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Image Registration", "app", "A2E0FC44-2A31-41EE-90F8-805773D01FCA");
   if (pInArgList == NULL || pOutArgList == NULL)
   {
      return false;
   }
   Progress* pProgress = pInArgList->getPlugInArgValue<Progress>(Executable::ProgressArg());
   //RasterElement* pCube = pInArgList->getPlugInArgValue<RasterElement>(Executable::DataElementArg());

   std::vector<Window*> windows;
   Service<DesktopServices>()->getWindows(SPATIAL_DATA_WINDOW, windows);
   std::vector<RasterElement*> elements;
   for (unsigned int i = 0; i < windows.size(); ++i)
   {
       SpatialDataWindow* pWindow = dynamic_cast<SpatialDataWindow*>(windows[i]);
       if (pWindow == NULL)
       {
           continue;
       }
       LayerList* pList = pWindow->getSpatialDataView()->getLayerList();
       elements.push_back(pList->getPrimaryRasterElement());
   }

   RasterElement* pCube = elements[0];
   RasterElement* pCubeRef = elements[1];
   if ((pCube == NULL) || (pCubeRef == NULL))
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

   FactoryResource<DataRequest> pRequestRef;
   pRequestRef->setInterleaveFormat(BSQ);
   DataAccessor pSrcAccRef = pCubeRef->getDataAccessor(pRequestRef.release());

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube->getName() +
      "_Image_Registration_Result", pDesc->getRowCount(), pDesc->getColumnCount(), ResultType));
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
   
   nCountMas = 0;
   nCountRef = 0;
   int windowSize = 6;
   for (unsigned int row = 0; row < pDesc->getRowCount(); ++row)
   {
       if (isAborted())
       {
          std::string msg = getName() + " has been aborted.";
          pStep->finalize(Message::Abort, msg);
          if (pProgress != NULL)
          {
             pProgress->updateProgress(msg, 0, ABORT);
          }
          return false;
       }
       for (unsigned int col = 0; col < pDesc->getColumnCount(); ++col)
       {
	       locateAllStarPosition(pSrcAcc, pSrcAccRef, row, col, pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType(), windowSize);
       }
   }

   ModifyCenter(pSrcAcc, pDesc->getDataType(), windowSize, nCountMas, nStarPositionsMas);
   ModifyCenter(pSrcAccRef, pDesc->getDataType(), windowSize, nCountRef, nStarPositionsRef);

   std::string msg = "Image Registration is compete. "+ StringUtilities::toDisplayString(pDesc->getRowCount()) + ";" + 
		                 StringUtilities::toDisplayString(pDesc->getColumnCount()) + ";" + StringUtilities::toDisplayString(nCountMas) + ";" + StringUtilities::toDisplayString(nCountRef);

   GetAllNeighborStars();
   GetMatchingStars();
   
   unsigned char *pBuffer = (unsigned char *)calloc(pDesc->getRowCount()*pDesc->getColumnCount(), sizeof(unsigned char));
   DrawStars(pBuffer, pDesc->getRowCount(), pDesc->getColumnCount());

   //Output the value 
      for (unsigned int m = 0; m < pDesc->getRowCount(); m++)
      {
	      for (unsigned int n = 0; n < pDesc->getColumnCount(); n++)
	      {
		      if (isAborted())
              {
                  std::string msg = getName() + " has been aborted.";
                  pStep->finalize(Message::Abort, msg);
                  if (pProgress != NULL)
                  {
                      pProgress->updateProgress(msg, 0, ABORT);
                  }
                  return false;
              }
				      
			  if (!pDestAcc.isValid())
              {
                  std::string msg = "Unable to access the cube data.";
                  pStep->finalize(Message::Failure, msg);
                  if (pProgress != NULL) 
                  {
                      pProgress->updateProgress(msg, 0, ERRORS);
                   }
                   return false;
               }

				       
			  switchOnEncoding(ResultType, updatePixel, pDestAcc->getColumn(), pBuffer, m, n, pDesc->getRowCount(), pDesc->getColumnCount());       
			  pDestAcc->nextColumn();
				   
		  }
				   
		  pDestAcc->nextRow();
		  
	  }
   free(pBuffer);
   
   


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
	   
       pProgress->updateProgress(msg, 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("Image Registration Result", pResultCube.release());

   pStep->finalize();


   return true;
}
