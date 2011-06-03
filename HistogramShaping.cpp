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
#include "HistogramShaping.h"
#include <limits>
#include <algorithm>

REGISTER_PLUGIN_BASIC(OpticksAstronomy, HistogramShaping);

namespace
{
   #define MAX_GRAY_VALUE 65535

   template<typename T>
   void calculateHistogram(T* pData, unsigned int *pHisto, unsigned int &pvMin, unsigned int &pvMax)
   {
      unsigned int pixelValue = static_cast<unsigned int>(*pData);
      pHisto[pixelValue] = pHisto[pixelValue]+1;
      
      pvMin = std::min(pvMin, pixelValue);
      pvMax = std::max(pvMax, pixelValue);
   }
   
   double GaussianFunc(unsigned int pv, unsigned int pvMax, double meanVal, double sigma)
   {
       double dpv = static_cast<double>(pv);
       double dpvMax = static_cast<double>(pvMax);
       double val = (dpv/dpvMax - meanVal)/sigma;
       val = val*val;
       val = exp(-val/2);

       return val;
   }
   
   void HistogramReshape(unsigned int *pHisto, double *pTarget, unsigned int *pLUT, unsigned int pvMax, double meanVal, double sigma)
   {
       unsigned int histoSum = 0;
       unsigned int pv = 0;
       double ratio = 0;
       
       for (pv = 0; pv < pvMax; pv++)
       {
           histoSum = histoSum + pHisto[pv];
       	   pHisto[pv] = histoSum;
       }
       
       for (pv = 0; pv < pvMax; pv++)
       {
           pTarget[pv] = GaussianFunc(pv, pvMax, meanVal, sigma);
       }
       
       double targetSum = 0;
       for (pv = 0; pv < pvMax; pv++)
       {
           targetSum = targetSum + pTarget[pv];
       	   pTarget[pv] = targetSum;
       }
       
       ratio = histoSum/targetSum;
       for (pv = 0; pv < pvMax; pv++)
       {
           pTarget[pv] = ratio*pTarget[pv];
       }
       
       unsigned int pvNew = 0;
       for (pv = 0; pv < pvMax; pv++)
       {
       	   while (pTarget[pvNew] < pHisto[pv])
       	   {
       	       pvNew++;
       	   }
       	   
       	   pLUT[pv] = pvNew;
       }
   }
   
         

   template<typename T>
   void updatePixel(T* pData, DataAccessor pSrcAcc, unsigned int *pLUT, int row, int col, EncodingType type)
   {
   	   unsigned int originalVal = 0;
       unsigned int targetVal = 0;
       
       pSrcAcc->toPixel(row, col);
       VERIFYNRV(pSrcAcc.isValid());
       
       originalVal = Service<ModelServices>()->getDataValue(type, pSrcAcc->getColumn(), COMPLEX_MAGNITUDE, 0);
       
       targetVal = *(pLUT+originalVal);

	   *pData = static_cast<T>(targetVal);

   }
};

HistogramShaping::HistogramShaping()
{
   setDescriptorId("{B28F5638-E2CD-48C3-8E83-AF08796EDA75}");
   setName("Histogram Shaping");
   setDescription("Histogram Shaping for Astronomical Image");
   setCreator("Yiwei Zhang");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Image Histogram Shaping");
   setMenuLocation("[Astronomy]/Histogram Shaping");
   setAbortSupported(true);
}

HistogramShaping::~HistogramShaping()
{
}

bool HistogramShaping::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Perform histogram shaping on this data element");
   return true;
}

bool HistogramShaping::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool HistogramShaping::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Histogram Shaping", "app", "0F2A0E79-F35C-4684-B834-17AA71570F3F");
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

   FactoryResource<DataRequest> pRequest;
   pRequest->setInterleaveFormat(BSQ);
   DataAccessor pSrcAcc = pCube->getDataAccessor(pRequest.release());

   ModelResource<RasterElement> pResultCube(RasterUtilities::createRasterElement(pCube->getName() +
      "_Histogram_Shaping_Result", pDesc->getRowCount(), pDesc->getColumnCount(), ResultType));
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
   
   unsigned int pvMin = std::numeric_limits<unsigned int>::max();
   unsigned int pvMax = 0;
   double sigma = 1.0/6.0;
   double meanVal = 0.25;
     

   Service<DesktopServices> pDesktop;
   {
   	  unsigned int *HistgramArray = (unsigned int *)calloc(sizeof(unsigned int), MAX_GRAY_VALUE);
   	  
      for (unsigned int row = 0; row < pDesc->getRowCount(); ++row)
      {
         if (pProgress != NULL)
         {
            pProgress->updateProgress("Histogram shaping ", row * 100 / pDesc->getRowCount(), NORMAL);
         }
         if (isAborted())
         {
            std::string msg = getName() + " has been aborted.";
            pStep->finalize(Message::Abort, msg);
            if (pProgress != NULL)
            {
               pProgress->updateProgress(msg, 0, ABORT);
            }
            free(HistgramArray);
            return false;
         }
         for (unsigned int col = 0; col < pDesc->getColumnCount(); ++col)
         {
             switchOnEncoding(pDesc->getDataType(), calculateHistogram, pSrcAcc->getColumn(), HistgramArray, pvMin, pvMax);
             pSrcAcc->nextColumn();
         }
         pSrcAcc->nextRow();
      }
      
      double *TargetHistogram = (double *)calloc(sizeof(double), pvMax+1);
      unsigned int *PixelMap = ( unsigned int *)calloc(sizeof(unsigned int), pvMax+1);
      
      HistogramReshape(HistgramArray, TargetHistogram, PixelMap, pvMax, meanVal, sigma);
      
      free(TargetHistogram);
      free(HistgramArray);
      
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

				       
			  switchOnEncoding(ResultType, updatePixel, pDestAcc->getColumn(), pSrcAcc, PixelMap, m, n, pDesc->getDataType());       
			  pDestAcc->nextColumn();
				   
		  }
				   
		  pDestAcc->nextRow();
		  
	  }  
		  
	  free(PixelMap);

      if (!isBatch())
      {
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
         pProgress->updateProgress("Histogram shaping compete.", 100, NORMAL);
      }

      pOutArgList->setPlugInArgValue("Histogram shaping result", pResultCube.release());

      pStep->finalize();
   }
   return true;
}
