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
#include "WaveletKSigmaFilter.h"
#include "WaveletKSigmaDlg.h"
#include "waveletlib.h"
#include "StringUtilities.h"
#include <limits>


REGISTER_PLUGIN_BASIC(OpticksAstronomy, WaveletKSigmaFilter);

namespace
{
    #define LAYERS 4
    #define BLOCK_ROWS 128
    #define BLOCK_COLS 128
	WaveletNode NodeList[LAYERS+1];

	int filterLen = 4;
	double pLoFilter[4] = {0.4830, 0.8365, 0.2241, -0.1294};
	double pHiFilter[4] = {-0.1294,-0.2241, 0.8365, -0.4830};
	double pRecHiFilter[4] = {-0.4830, 0.8365, -0.2241, -0.1294};
	double pRecLoFilter[4] = {-0.1294,0.2241, 0.8365, 0.4830};

   void ProcessData(DataAccessor pSrcAcc, double *pBuffer, unsigned int row, unsigned int col, unsigned int rowBlocks, unsigned int colBlocks, double *pScaleKSigma, EncodingType type)
   {
	  unsigned int nCount = 0;
	  double pixelVal;

	  for (unsigned int i=0; i<rowBlocks; i++)
	  {
		  for (unsigned int j=0; j<colBlocks; j++)
		  {
			  pSrcAcc->toPixel(row+i, col+j);
              VERIFYNRV(pSrcAcc.isValid());
		      pixelVal = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);

			  *(pBuffer + nCount) = pixelVal;
			  nCount++;
		  }
	  }

	  ShiftInvariantWaveletTransform(pBuffer, rowBlocks, colBlocks, pLoFilter, pHiFilter, filterLen, LAYERS, NodeList);
	  WaveletDenoise(NodeList, pBuffer, LAYERS, pScaleKSigma);
	  ShiftInvariantInverseWaveletTransform(rowBlocks, colBlocks, pRecLoFilter, pRecHiFilter, filterLen, LAYERS, NodeList);

	  ReleaseList(NodeList, LAYERS);
   }

   template<typename T>
   void speckleNoiseRemove(T* pData, double *pSrc)
   {
	   double pixelVal;
	   if (pSrc != NULL)
	   {
		   pixelVal = *pSrc;
	       *pData = static_cast<T>(pixelVal);
	   }
   }
};

WaveletKSigmaFilter::WaveletKSigmaFilter()
{
   setDescriptorId("{28702D56-4634-4CCC-8840-C10F805C9870}");
   setName("Wavelet K-Sigma Filter");
   setDescription("Noise removal for astronomical image");
   setCreator("Yiwei Zhang");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Denoise");
   setMenuLocation("[Astronomy]/Wavelet K-Sigma Filter");
   setAbortSupported(true);

   rowBlocks = BLOCK_ROWS;
   colBlocks = BLOCK_COLS;
   pBuffer = (double *)malloc(sizeof(double)*(10+rowBlocks)*(10+colBlocks));  
}

WaveletKSigmaFilter::~WaveletKSigmaFilter()
{
	free(pBuffer);
}

bool WaveletKSigmaFilter::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Perform noise removal on this data element");
   return true;
}

bool WaveletKSigmaFilter::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool WaveletKSigmaFilter::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Wavelet K-Sigma Filter", "app", "1A4BDC34-5A95-419B-8E53-C92333AFFC3E");
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
      "_Noise_Removal_Result", pDesc->getRowCount(), pDesc->getColumnCount(), ResultType));
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
   WaveletKSigmaDlg dlg(pDesktop->getMainWidget());
   int stat = dlg.exec();
   if (stat != QDialog::Accepted)
   {
	  // pProgress->updateProgress("Level 4 " + StringUtilities::toDisplayString(dlg.getLevelThreshold(3))
       //  + " Level5 " + StringUtilities::toDisplayString(dlg.getLevelThreshold(4)), dlg.getLevelThreshold(0), NORMAL);

	   return true;
   }

   unsigned int rowLoops;
   unsigned int colLoops;
   unsigned int rowIndex = 0;
   unsigned int colIndex = 0;
   double ScaleKValue[MAX_WAVELET_LEVELS] = {0.0};
   for (int k=0; k<MAX_WAVELET_LEVELS;k++)
   {
	   ScaleKValue[k] = dlg.getLevelThreshold(k);
   }
   
   if (0 == pDesc->getRowCount()%rowBlocks)
   {
	   rowLoops = pDesc->getRowCount()/rowBlocks;
   }
   else
   {
	   rowLoops = pDesc->getRowCount()/rowBlocks + 1;
   }

   if (0 == pDesc->getColumnCount()%colBlocks)
   {
	   colLoops = pDesc->getColumnCount()/colBlocks;
   }
   else
   {
	   colLoops = pDesc->getColumnCount()/colBlocks + 1;
   }

   for (unsigned int i = 0; i < rowLoops; i++)
   {
	   if ( rowIndex + rowBlocks > pDesc->getRowCount())
	   {
		   rowIndex = pDesc->getRowCount() - rowBlocks;
	   }

	   colIndex = 0;

	   for (unsigned int j = 0; j < colLoops; j++)
	   {
		   if ( colIndex + colBlocks > pDesc->getColumnCount())
	       {
		       colIndex = pDesc->getColumnCount() - colBlocks;
	       }

		   if (pProgress != NULL)
           {
               pProgress->updateProgress("Remove result", (i*colLoops+j) / (rowLoops*colLoops), NORMAL);
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
      
           //Process the data in current block
		   ProcessData(pSrcAcc, pBuffer, rowIndex, colIndex, rowBlocks, colBlocks, ScaleKValue, pDesc->getDataType());

		   //Output the value 
           for (unsigned int m = 0; m < rowBlocks; m++)
		   {
			   for (unsigned int n = 0; n < colBlocks; n++)
			   {
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

				   pDestAcc->toPixel(rowIndex+m, colIndex+n);
				   
				   switchOnEncoding(ResultType, speckleNoiseRemove, pDestAcc->getColumn(), (pBuffer+m*colBlocks+n));
			   }
		   }
		   colIndex += colBlocks;
	   }
	   rowIndex += rowBlocks;
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
      pProgress->updateProgress("Noise removal is compete.", 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("Noise removal Result", pResultCube.release());

   pStep->finalize();
   return true;
}
