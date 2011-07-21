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
#include "BrightnessMeasurement.h"
#include "BrightnessMeasurementDlg.h"


#include <limits>

REGISTER_PLUGIN_BASIC(OpticksAstronomy, BrightnessMeasurement);

namespace
{
   
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
   
   void InitializeData(DataAccessor pSrcAcc, double *pBuffer, unsigned int rows, unsigned int cols, EncodingType type)
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
			  nCount++;
		  }
	  }
   }
}


BrightnessMeasurement::BrightnessMeasurement()
{
   setDescriptorId("{3111B158-B58B-4B51-8BA2-43C0EACBCADA}");
   setName("Brightness Measurement");
   setDescription("Star Brightness Measurement");
   setCreator("Yiwei Zhang");
   setVersion("Sample");
   setCopyright("Copyright (C) 2008, Ball Aerospace & Technologies Corp.");
   setProductionStatus(false);
   setType("Sample");
   setSubtype("Brightness Measurement");
   setMenuLocation("[Astronomy]/Brightness Measurement");
   setAbortSupported(true);
   
   pOriginalImage = NULL;
}

BrightnessMeasurement::~BrightnessMeasurement()
{
	if (pOriginalImage != NULL)
		free(pOriginalImage);
}

bool BrightnessMeasurement::getInputSpecification(PlugInArgList*& pInArgList)
{
   VERIFY(pInArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pInArgList->addArg<Progress>(Executable::ProgressArg(), NULL, "Progress reporter");
   pInArgList->addArg<RasterElement>(Executable::DataElementArg(), "Perform speckle remove on this data element");
   return true;
}

bool BrightnessMeasurement::getOutputSpecification(PlugInArgList*& pOutArgList)
{
   VERIFY(pOutArgList = Service<PlugInManagerServices>()->getPlugInArgList());
   pOutArgList->addArg<RasterElement>("Result", NULL);
   return true;
}

bool BrightnessMeasurement::execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList)
{
   StepResource pStep("Brightness Measurement", "app", "8543CE38-3D77-4AE4-8231-186EB83AAECE");
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

   
   if (NULL != pOriginalImage)
   {
	   free(pOriginalImage);
   }
   pOriginalImage = (double *)malloc(sizeof(double)*pDesc->getRowCount()*pDesc->getColumnCount());
   
   double minGrayValue = 0.0;
   double maxGrayValue = 255.0;
   InitializeData(pSrcAcc, pOriginalImage, pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType());
   GetGrayScale(&minGrayValue, &maxGrayValue, pDesc->getDataType());
   
   Service<DesktopServices> pDesktop;
   BrightnessMeasurementDlg dlg(pDesktop->getMainWidget(), pOriginalImage, pDesc->getColumnCount(), pDesc->getRowCount(),  minGrayValue, maxGrayValue);
   int stat = dlg.exec();
   if (stat != QDialog::Accepted)
   {
	   return true;
   }

   if (pProgress != NULL)
   {
      pProgress->updateProgress("Finished!", 100, NORMAL);
   }
   
   pStep->finalize();

   return true;
}
