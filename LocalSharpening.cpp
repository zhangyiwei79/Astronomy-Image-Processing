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
#include "LocalSharpening.h"
#include "LocalSharpeningDlg.h"
#include <limits>

REGISTER_PLUGIN_BASIC(OpticksAstronomy, LocalSharpening);

namespace
{

   #define MAX_WINDOW_SIZE 7
   
   template<typename T>
   void localExtremeSharpening(T* pData, DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize, EncodingType type, int windowSize)
   {
	  int i, j, m, n;
	  double minVal = std::numeric_limits<double>::max();
      double maxVal = -minVal;
    
	  double pixelVal = 0.0;
	  double diffVal = 0.0;


	  double windowVal[MAX_WINDOW_SIZE*2+1][MAX_WINDOW_SIZE*2+1];

	  if ((col-windowSize < 0) || (col+windowSize > colSize - 1))
	  {
		  pSrcAcc->toPixel(row, col);
          VERIFYNRV(pSrcAcc.isValid());
		  pixelVal = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);
          *pData = static_cast<T>(pixelVal);
		  return;
	  }

	  if ((row-windowSize < 0) || (row+windowSize > rowSize - 1))
	  {
		  pSrcAcc->toPixel(row, col);
          VERIFYNRV(pSrcAcc.isValid());
		  pixelVal = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);
          *pData = static_cast<T>(pixelVal);
		  return;
	  }

	  //Get the pixels in the window
	  m = 0;
	  for (i=row - windowSize; i<= row + windowSize; i++)
	  {
		  n = 0;
		  for (j=col - windowSize; j<= col + windowSize; j++)
		  {
			 pSrcAcc->toPixel(i, j);
             VERIFYNRV(pSrcAcc.isValid());
			 windowVal[m][n] = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);
			 
			 if (minVal > windowVal[m][n])	
			 {
				 minVal = windowVal[m][n];	
			 }
			 	
			 	
			 if (maxVal < windowVal[m][n])	
			 {	
				 maxVal = windowVal[m][n];	
			 }
			 n++;
		  }
		  m++;
	  }
	  pixelVal = windowVal[windowSize][windowSize];

	  if (pixelVal - minVal <= maxVal - pixelVal)
	  {
		  pixelVal = minVal;
      }
      else	
	  {
		  pixelVal = maxVal;	
	  }

	  *pData = static_cast<T>(pixelVal);
   }
   
   

   template<typename T>
   void localAdaptiveSharpening(T* pData, DataAccessor pSrcAcc, int row, int col, int rowSize, int colSize, EncodingType type, int windowSize, double contrastVal)
   {
	  int i, j, m, n;
	  double meanVal = 0.0;
	  double sigmaVal = 0.0;
	  double pixelVal = 0.0;
	  double diffVal = 0.0;


	  double windowVal[MAX_WINDOW_SIZE*2+1][MAX_WINDOW_SIZE*2+1];

	  if ((col-windowSize < 0) || (col+windowSize > colSize - 1))
	  {
		  pSrcAcc->toPixel(row, col);
          VERIFYNRV(pSrcAcc.isValid());
		  pixelVal = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);
          *pData = static_cast<T>(pixelVal);
		  return;
	  }

	  if ((row-windowSize < 0) || (row+windowSize > rowSize - 1))
	  {
		  pSrcAcc->toPixel(row, col);
          VERIFYNRV(pSrcAcc.isValid());
		  pixelVal = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);
          *pData = static_cast<T>(pixelVal);
		  return;
	  }

	  //Get the pixels in the window
	  m = 0;
	  for (i=row - windowSize; i<= row + windowSize; i++)
	  {
		  n = 0;
		  for (j=col - windowSize; j<= col + windowSize; j++)
		  {
			 pSrcAcc->toPixel(i, j);
             VERIFYNRV(pSrcAcc.isValid());
			 windowVal[m][n] = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);
			 n++;
		  }
		  m++;
	  }
	  pixelVal = windowVal[windowSize][windowSize];

	  //Calculate the mean value
	  for (m = 0; m< 2*windowSize+1; m++)
	  {
		  for (n=0; n<2*windowSize+1; n++)
		  {
			  meanVal = meanVal + windowVal[m][n];		  
		  }
	  }
	  meanVal = meanVal/((2*windowSize+1)*(2*windowSize+1));
	  diffVal = pixelVal - meanVal;

	  //Calculate the sigma value
	  for (m = 0; m< 2*windowSize+1; m++)
	  {
		  for (n=0; n<2*windowSize+1; n++)
		  {
			  sigmaVal = sigmaVal + (windowVal[m][n]-meanVal)*(windowVal[m][n]-meanVal);		  
		  }
	  }
	  sigmaVal = sigmaVal/((2*windowSize+1)*(2*windowSize+1)-1);
	  sigmaVal = sqrt(sigmaVal);

	  pixelVal = pixelVal + diffVal*contrastVal/sigmaVal;

	 
	  if (type == INT1UBYTE)
	  {
		  if (pixelVal > 255)
	      {
			  pixelVal = 255;
		  }

		  if (pixelVal < 0)
		  {
			  pixelVal = 0;
		  }
	  }
	  
	  if (type == INT1SBYTE)
	  {
		  if (pixelVal > 127)
	      {
			  pixelVal = 127;
		  }

		  if (pixelVal < -127)
		  {
			  pixelVal = -127;
		  }
	  }

	  
	  if (type == INT2UBYTES)
	  {
		  if (pixelVal > 65535)
	      {
			  pixelVal = 65535;
		  }

		  if (pixelVal < 0)
		  {
			  pixelVal = 0;
		  }
	  }

	  if (type == INT2SBYTES)
	  {
		  if (pixelVal > 32767)
	      {
			  pixelVal = 32767;
		  }

		  if (pixelVal < -32767)
		  {
			  pixelVal = -32767;
		  }
	  }
	  
       *pData = static_cast<T>(pixelVal);

   }
};

LocalSharpening::LocalSharpening()
{
   setDescriptorId("{B8B8B457-1C37-444C-AD76-00DCDCB712EB}");
   setName("Local Sharpening");
   setDescription("Image Enhancement through local sharpening");
   setCreator("Yiwei Zhang");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Image Enhancement");
   setMenuLocation("[Astronomy]/Local Sharpening");
   setAbortSupported(true);
}

LocalSharpening::~LocalSharpening()
{
}

bool LocalSharpening::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Perform speckle remove on this data element");
   return true;
}

bool LocalSharpening::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool LocalSharpening::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Local Sharpening", "app", "08BB9B79-5D24-4AB0-9F35-92DE77CED8E7");
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
      "_Local_Sharpening_Result", pDesc->getRowCount(), pDesc->getColumnCount(), ResultType));
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
   LocalSharpeningDlg dlg(pDesktop->getMainWidget());
   int stat = dlg.exec();
   if (stat != QDialog::Accepted)
   {
	   return true;
   }
   double contrastVal = dlg.getContrastValue();
   int nFilterType = dlg.getCurrentFilterType();
   int windowSize = dlg.getCurrentWindowSize();
   windowSize = (windowSize-1)/2;

   for (unsigned int row = 0; row < pDesc->getRowCount(); ++row)
   {
      if (pProgress != NULL)
      {
         pProgress->updateProgress("Local sharpening", row * 100 / pDesc->getRowCount(), NORMAL);
      }
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
      for (unsigned int col = 0; col < pDesc->getColumnCount(); ++col)
      {
		  if (nFilterType == 0)
		  {
			  switchOnEncoding(ResultType, localAdaptiveSharpening, pDestAcc->getColumn(), pSrcAcc, row, col,
                               pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType(), windowSize, contrastVal);
		  }
		  else
		  {
           
			  switchOnEncoding(ResultType, localExtremeSharpening, pDestAcc->getColumn(), pSrcAcc, row, col,
                               pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType(), windowSize);
		  }
		  pDestAcc->nextColumn();
      }

      pDestAcc->nextRow();
   }


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
      pProgress->updateProgress("Local sharpening is compete.", 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("Local sharpening Result", pResultCube.release());

   pStep->finalize();


   return true;
}
