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
#include "Statistics.h"
#include "switchOnEncoding.h"
#include "ImageRegistration.h"
#include "StringUtilities.h"
#include "LayerList.h"
#include <limits>

REGISTER_PLUGIN_BASIC(OpticksAstronomy, ImageRegistration);

namespace
{

   #define MAX_WINDOW_SIZE 10
   #define MAX_DIFFERENCE_THRESHOLD 0.35
   #define MINIMUM_RADIUS_LIMIT  100
   #define MAX_STAR_NUMBERS  20
   
   #define MAX_MATCHING_ERROR  300
   
   int nCountMas;
   int nStarPositionsMas[MAX_STAR_NUMBERS][2] = {0};
   long nStarListMas[MAX_STAR_NUMBERS][MAX_STAR_NUMBERS-1] = {0};
   double nStarGrayValueMas[MAX_STAR_NUMBERS] = {0};

   int nCountRef;
   int nStarPositionsRef[MAX_STAR_NUMBERS][2] = {0};
   long nStarListRef[MAX_STAR_NUMBERS][MAX_STAR_NUMBERS-1] = {0};
   double nStarGrayValueRef[MAX_STAR_NUMBERS] = {0};
   
   int nMatchingStarList[MAX_STAR_NUMBERS][3] = {0};
   double shiftX, shiftY, dScale;
   double matrixT[2][2];
   
   double gMinGrayValue,gMaxGrayValue;

   
   int round(double number)
   {
       double retVal = number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
	   int intVal = (int)retVal;

	   return intVal;
   }

   void DrawStars(double *pBuffer, DataAccessor pSrcAccRef, EncodingType type, double T[][2], int rows, int cols)
   {
       int i, j;
	   double pixelVal;
	   double Y[2], Z[2];
	   int rowIndex, colIndex;

	   for (i=0; i<rows; i++)
	   {
		   for (j=0; j<cols; j++)
		   {
   	           pSrcAccRef->toPixel(i, j);
	           pixelVal = Service<ModelServices>()->getDataValue(type, pSrcAccRef->getColumn(), COMPLEX_MAGNITUDE, 0);

               Y[0] = j-cols/2; Y[1] = -i+rows/2;
			   Z[0] = Y[0]*T[0][0] + Y[1]*T[1][0] + shiftX;
			   Z[1] = Y[0]*T[0][1] + Y[1]*T[1][1] + shiftY;

               //Z = Y * T + [shiftX, shiftY];
        
			   rowIndex = round(-Z[1] +rows/2);
               colIndex = round(Z[0] + cols/2);
        
               if ((rowIndex < 0) || (rowIndex >= rows))
                   continue;
        
               if ((colIndex < 0) || (colIndex >= cols))
                   continue;
        
               pBuffer[rowIndex*cols+ colIndex] += pixelVal;
			   //pBuffer[rowIndex*cols+ colIndex] = pBuffer[rowIndex*cols+ colIndex]/2;

			   //pBuffer[rowIndex*cols+ colIndex] = pixelVal;

			  
			   if (pBuffer[rowIndex*cols+ colIndex] > gMaxGrayValue)
				   pBuffer[rowIndex*cols+ colIndex] = gMaxGrayValue;

			   if (pBuffer[rowIndex*cols+ colIndex] < gMinGrayValue)
				   pBuffer[rowIndex*cols+ colIndex] = gMinGrayValue;
		   }
	   }
   }
   
   void GetGrayScale(EncodingType type)
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
	  
	  gMinGrayValue = minGrayVal;
	  gMaxGrayValue = maxGrayVal;
	 }
   
   template<typename T>

   void updatePixel(T* pData, double *pBuffer, int row, int col, int rowSize, int colSize)
   {
       
       double pixelVal = pBuffer[row*colSize+col];
	   pixelVal = pixelVal;

	    *pData = static_cast<T>(pixelVal);

   }
    
   
   double svd2D(double A[][2], double T[][2])
   {
       double B[2][2] = {0.0};
       double C[2][2] = {0.0};
       
       B[0][0] = A[0][0]*A[0][0]+A[1][0]*A[1][0]; 
       B[0][1] = A[0][0]*A[0][1]+A[1][0]*A[1][1]; 
       B[1][1] = A[0][1]*A[0][1]+A[1][1]*A[1][1];
        
       C[0][0] = A[0][0]*A[0][0]+A[0][1]*A[0][1]; 
       C[0][1] = A[0][0]*A[1][0]+A[0][1]*A[1][1]; 
       	
       double t = B[0][0]+B[1][1]; 
       double d = B[0][0]*B[1][1]-B[0][1]*B[0][1]; 
       double r1 = (t+sqrt(t*t-4*d))/2; 
       double r2 = d/r1; 
       double h = sqrt(B[0][1]*B[0][1]+(r1-B[0][0])*(r1-B[0][0])); 
       
       double Q1[2][2] = {0.0};
       double Q2[2][2] = {0.0};
       
       Q2[0][0] = B[0][1]/h; 
       Q2[1][1] = Q2[0][0]; 
       Q2[1][0] = (r1-B[0][0])/h; 
       Q2[0][1] = -Q2[1][0]; 

       double k = sqrt(C[0][1]*C[0][1]+(r1-C[0][0])*(r1-C[0][0])); 
       Q1[0][0] = C[0][1]/k; 
       Q1[1][1] = Q1[0][0]; 
       Q1[1][0] = (r1-C[0][0])/k; 
       Q1[0][1] = -Q1[1][0]; 
       	
       double temp = Q1[0][1];
       Q1[0][1] = Q1[1][0];
       Q1[1][0] = temp;
       T[0][0] = Q2[0][0]*Q1[0][0] + Q2[0][1]*Q1[1][0];
       T[1][1] = Q2[1][0]*Q1[0][1] + Q2[1][1]*Q1[1][1];
       T[0][1] = Q2[0][0]*Q1[0][1] + Q2[0][1]*Q1[1][1];
       T[1][0] = Q2[1][0]*Q1[0][0] + Q2[1][1]*Q1[1][0];

       return (sqrt(r1)+ sqrt(r2));
   }
   
   void procrustes(double X[][2], double Y[][2], int len)
   {
       int n = len;
       double muX1 = 0, muX2 = 0;
       double muY1 = 0, muY2 = 0;
       double ssqX1 = 0, ssqX2 = 0;
       double ssqY1 = 0, ssqY2 = 0;
       
       double matrixA[2][2] = {0.0};
       
       double X0[MAX_STAR_NUMBERS][2] = {0.0};
       double Y0[MAX_STAR_NUMBERS][2] = {0.0};

       //center at the origin
       for (int i=0; i<n; i++)
       { 
           muX1 = muX1 + X[i][0];
           muX2 = muX2 + X[i][1];
           muY1 = muY1 + Y[i][0];
           muY2 = muY2 + Y[i][1];
       }
       
       muX1 = muX1/n;
       muX2 = muX2/n;
       muY1 = muY1/n;
       muY2 = muY2/n;      

       for (int i=0; i<n; i++)
       {
           X0[i][0] = X[i][0] - muX1;
           X0[i][1] = X[i][1] - muX2;
           Y0[i][0] = Y[i][0] - muY1;
           Y0[i][1] = Y[i][1] - muY2;
       }
       
       for (int i=0; i<n; i++)
       {
           ssqX1 += X0[i][0]*X0[i][0];
           ssqX2 += X0[i][1]*X0[i][1];
           ssqY1 += Y0[i][0]*Y0[i][0];
           ssqY2 += Y0[i][1]*Y0[i][1];
       }


       // the "centered" Frobenius norm
       double normX = sqrt((ssqX1 + ssqX2));
       double normY = sqrt((ssqY1 + ssqY2));

       //scale to equal (unit) norm
       for (int i=0; i<n; i++)
       {
           X0[i][0] = X0[i][0] / normX;
           X0[i][1] = X0[i][1] / normX;
           Y0[i][0] = Y0[i][0] / normY;
           Y0[i][1] = Y0[i][1] / normY;
       }

       //optimum rotation matrix of Y
       for (int i=0; i<n; i++)
       {
           matrixA[0][0] += X0[i][0]*Y0[i][0];
           matrixA[0][1] += X0[i][0]*Y0[i][1];
           matrixA[1][0] += X0[i][1]*Y0[i][0];
           matrixA[1][1] += X0[i][1]*Y0[i][1];
       }
       
       double trsAA = svd2D(matrixA, matrixT);
     
       dScale = trsAA * normX / normY;

       shiftX = muX1 - dScale*(muY1*matrixT[0][0] + muY2*matrixT[1][0]);
       shiftY = muX2 - dScale*(muY1*matrixT[0][1] + muY2*matrixT[1][1]);

   }
   
   void GetParameters(int rows, int cols)
   {
       double X[MAX_STAR_NUMBERS][2];
       double Y[MAX_STAR_NUMBERS][2];
	   int k;
	   int nIndex = 0;

	   int nCount  = std::min(nCountMas, nCountRef);

       for (nIndex=0; nIndex<nCount; nIndex++)
	   {
		   if (nMatchingStarList[nIndex][2] > MAX_MATCHING_ERROR*nCount)
		       break;
	   }

	   nCount = std::max(3, nIndex);

       for (int i=0; i<nCount; i++)
       {
       	   k = nMatchingStarList[i][0];
           X[i][0] = nStarPositionsMas[k][1]-cols/2;
           X[i][1] = -nStarPositionsMas[k][0]+rows/2;
           
           k = nMatchingStarList[i][1];
           Y[i][0] = nStarPositionsRef[k][1]-cols/2;
           Y[i][1] = -nStarPositionsRef[k][0]+rows/2;
       }
       
       procrustes(X, Y, nCount);
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
	  
	  
	  if (abs(maxVal - minVal) > MAX_DIFFERENCE_THRESHOLD*(gMaxGrayValue - gMinGrayValue))
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

   void locateAllStarPosition(DataAccessor pSrcAccMas, DataAccessor pSrcAccRef, int row, int col, int rowSize, int colSize, EncodingType type, int windowSize, double *pBuffer)
   {
	   pSrcAccMas->toPixel(row, col);
      
	   pBuffer[row*colSize+col] = Service<ModelServices>()->getDataValue(type, pSrcAccMas->getColumn(), COMPLEX_MAGNITUDE, 0);

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

   std::vector<int> badValues = pDesc->getBadValues();
   //badValues.push_back(0);
   RasterDataDescriptor* pDescriptor = dynamic_cast<RasterDataDescriptor*>(pResultCube->getDataDescriptor());
   Statistics* pStatistics = pResultCube->getStatistics(pDescriptor->getActiveBand(0));
   pStatistics->setBadValues(badValues);

   FactoryResource<DataRequest> pResultRequest;
   pResultRequest->setWritable(true);
   DataAccessor pDestAcc = pResultCube->getDataAccessor(pResultRequest.release());
   
   nCountMas = 0;
   nCountRef = 0;
   int windowSize = 6;
   double *pBuffer = (double *)calloc(pDesc->getRowCount()*pDesc->getColumnCount(), sizeof(double));

   GetGrayScale(pDesc->getDataType());

   
   for (unsigned int row = 0; row < pDesc->getRowCount(); ++row)
   {
	   if (pProgress != NULL)
       {
           pProgress->updateProgress("Image registration", row * 100 / pDesc->getRowCount(), NORMAL);
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
       for (unsigned int col = 0; col < pDesc->getColumnCount(); ++col)
       {
	       locateAllStarPosition(pSrcAcc, pSrcAccRef, row, col, pDesc->getRowCount(), pDesc->getColumnCount(), pDesc->getDataType(), windowSize, pBuffer);
       }
   }

   ModifyCenter(pSrcAcc, pDesc->getDataType(), windowSize, nCountMas, nStarPositionsMas);
   ModifyCenter(pSrcAccRef, pDesc->getDataType(), windowSize, nCountRef, nStarPositionsRef);

   GetAllNeighborStars();
   GetMatchingStars();
   
   GetParameters(pDesc->getRowCount(), pDesc->getColumnCount());
  
   DrawStars(pBuffer, pSrcAccRef, pDesc->getDataType(), matrixT, pDesc->getRowCount(), pDesc->getColumnCount());

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

   double theta = std::acos(matrixT[0][0])*180.0/3.1415926;

   std::string msg = "Image Registration is complete.\n Translation x = " +  StringUtilities::toDisplayString(round(shiftX)) + ", y = " + 
	                 StringUtilities::toDisplayString(round(shiftY)) + ", rotation = " + StringUtilities::toDisplayString(round(theta)) + " degree";
   if (pProgress != NULL)
   {
	   
       pProgress->updateProgress(msg, 100, NORMAL);
   }

   pOutArgList->setPlugInArgValue("Image Registration Result", pResultCube.release());

   pStep->finalize();


   return true;
}
