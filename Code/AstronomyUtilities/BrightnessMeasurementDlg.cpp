/*
 * The information in this file is
 * Copyright(c) 2011 Yiwei Zhang
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#include "AppAssert.h"
#include "AppVerify.h"
#include "BrightnessMeasurementDlg.h"


#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QComboBox>
#include <QtGui/QScrollArea>
#include <QtGui/QMessagebox.h>
#include <QtGui/QEvent.h>
#include <QtGui/QPainter.h>


using namespace std;

#define MAX_INNER_RADIUS 10
#define MAX_OUTTER_RADIUS 25
#define MEAN_STAR_DIFFERENCE_THRESHOLD   0.1
#define MEAN_SKY_DIFFERENCE_THRESHOLD   0.05

BrightnessMeasurementDlg::BrightnessMeasurementDlg(QWidget* pParent, double *pBuffer, int imgWidth, int imgHeight, double t1, double t2, EncodingType type) : QDialog(pParent),
   pStarPosition(NULL), pStarBrightness(NULL), pSkyBrightness(NULL), pInRadius(NULL), pOutRadius(NULL), pCompute(NULL), imageLabel(NULL), qImage(NULL), pMode(NULL), pStarIndex(NULL)
{
   setWindowTitle("Brightness Measurement");
   
   pImage = pBuffer;
   mWidth = imgWidth;
   mHeight = imgHeight;
   
   mMode = 0;
   mStarIndex = 0;
   
   mMinGrayValue = t1;
   mMaxGrayValue = t2;
   
   mInnerRadius = 2;
   mOutterRadius = 8;
   
   mInnerRadius_2 = 2;
   mOutterRadius_2 = 8;
   
   mPosX = -1;
   mPosY = -1;
   
   mPosX_2 = -1;
   mPosY_2 = -1;
   
   mMaxGrayLevel = t2 - t1;

   DisplayBuffer = (unsigned char *)malloc(sizeof(unsigned char)*mWidth*mHeight*4);
   unsigned long nCount= 0, nIndex = 0;
   for (nIndex=0; nIndex<mWidth*mHeight; nIndex++)
   {
   	 double tempData = pImage[nIndex];
		
		 if (type == INT1SBYTE)
	   {
	       tempData = tempData + 128;
	   }
	  
	   if (type == INT2UBYTES)
	   {
	  	 tempData = tempData/255;
	   }
	  
	   if (type == INT2SBYTES)
	   {
	  	  tempData = (tempData + 32768)/255;
	   }
	   
	   DisplayBuffer[nCount] = tempData;
	   DisplayBuffer[nCount+1] = tempData;
	   DisplayBuffer[nCount+2] = tempData;
	   DisplayBuffer[nCount+3] = 0xFF;
	   nCount += 4;
   }

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);
   
   QLabel* pLableMode = new QLabel("Mode:", this);
   pLayout->addWidget(pLableMode, 0, 10);

   pMode = new QComboBox(this);
   pMode->addItem("Single star measurement");
   pMode->addItem("Relative brightness meaurement");
   pMode->setCurrentIndex(0);
   pLayout->addWidget(pMode, 0, 11);
   
   QLabel* pLableStarIndex = new QLabel("Select Star:", this);
   pLayout->addWidget(pLableStarIndex, 1, 10);

   pStarIndex = new QComboBox(this);
   pStarIndex->addItem("1st star");
   pStarIndex->addItem("2nd star");
   pStarIndex->setCurrentIndex(0);
   pStarIndex->setEnabled (false);
   pLayout->addWidget(pStarIndex, 1, 11);

   QLabel* pLable1 = new QLabel("Inner Radius", this);
   pLayout->addWidget(pLable1, 2, 10);

   pInRadius = new QDoubleSpinBox(this);
   pInRadius->setRange(1, MAX_INNER_RADIUS);
   pInRadius->setSingleStep(1);
   pInRadius->setValue(mInnerRadius);
   pLayout->addWidget(pInRadius, 2, 11);

   QLabel* pLable2 = new QLabel("Outer Radius", this);
   pLayout->addWidget(pLable2, 3, 10);

   pOutRadius = new QDoubleSpinBox(this);
   pOutRadius->setRange(1, MAX_OUTTER_RADIUS);
   pOutRadius->setSingleStep(1);
   pOutRadius->setValue(mOutterRadius);
   pLayout->addWidget(pOutRadius, 3, 11);

   
   pStarPosition = new QLabel("Star Position:", this);
   pLayout->addWidget(pStarPosition, 4, 10, 1, 2);
   
   pStarBrightness = new QLabel("Star Brightness:", this);
   pLayout->addWidget(pStarBrightness, 5, 10, 1, 2);
   
   
   pSkyBrightness = new QLabel(" ", this);
   pLayout->addWidget(pSkyBrightness, 6, 10, 1, 2);
   

   QHBoxLayout* pRespLayout = new QHBoxLayout;
   pLayout->addLayout(pRespLayout, 7, 10, 1, 2);

   pCompute = new QPushButton("Compute", this);
   pRespLayout->addStretch();
   pRespLayout->addWidget(pCompute);

   QPushButton* pReject = new QPushButton("Close", this);
   pRespLayout->addWidget(pReject);

   connect(pCompute, SIGNAL(clicked()), this, SLOT(ComputeBrightness()));
   connect(pReject,  SIGNAL(clicked()), this, SLOT(reject()));
   
   connect(pMode, SIGNAL(currentIndexChanged(int)), this, SLOT(setMeasurementMode(int)));
   connect(pStarIndex, SIGNAL(currentIndexChanged(int)), this, SLOT(setStarIndex(int)));

   connect(pInRadius,  SIGNAL(valueChanged(double)), this, SLOT(setInnerRadius(double)));
   connect(pOutRadius, SIGNAL(valueChanged(double)), this, SLOT(setOutterRadius(double)));
   
   imageLabel = new QLabel;
   imageLabel->setBackgroundRole(QPalette::Base);
   imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
   imageLabel->installEventFilter(this);
   	

   qImage = new QImage(DisplayBuffer, mWidth, mHeight, QImage::Format_RGB32);
   
    imageLabel->setPixmap(QPixmap::fromImage(*qImage));
    imageLabel->adjustSize();
   	
   	scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    
    pLayout->addWidget(scrollArea, 0, 0, 8, 10);
   
    resize(600, 300);

}

BrightnessMeasurementDlg::~BrightnessMeasurementDlg()
{
	free(DisplayBuffer);
}

void BrightnessMeasurementDlg::setMeasurementMode(int nIndex)
{

	if (mMode == nIndex)
	{
		return;
	}
	else
	{
		mInnerRadius = 2;
        mOutterRadius = 8;
   
        mInnerRadius_2 = 2;
        mOutterRadius_2 = 8;
   
        mPosX = -1;
        mPosY = -1;
   
        mPosX_2 = -1;
        mPosY_2 = -1;

		imageLabel->update();
	    imageLabel->repaint();

		pStarPosition->setText("Star Position:");
		if (nIndex == 0)
		{
		    pStarBrightness->setText("Star Brightness:");
		}
		else
		{
			pStarBrightness->setText("Relative Brightness:");
		}
		//pSkyBrightness->setText("Sky Brightness:");

		pInRadius->setValue(2);
		pOutRadius->setValue(8);
	}

	mMode = nIndex;
	if (mMode == 0)
	{
		pStarIndex->setEnabled (false);
	}
	else
	{
		pStarIndex->setEnabled (true);;
	}
}
   
void BrightnessMeasurementDlg::setStarIndex(int nIndex)
{
	if (mStarIndex != nIndex)
	{
		pStarPosition->setText("Star Position:");
	}

    mStarIndex = nIndex;
}

void BrightnessMeasurementDlg::setInnerRadius(double dVal)
{
	if (mMode == 0)
	{
		mInnerRadius = dVal;
	}
	else
	{
	    if (mStarIndex == 0)
	    {
	        mInnerRadius = dVal;
	    }
	    else
	    {
		    mInnerRadius_2 = dVal;
	    }
	}

	imageLabel->update();
	imageLabel->repaint();
		
}

void BrightnessMeasurementDlg::setOutterRadius(double dVal)
{
	if (mMode == 0)
	{
		mOutterRadius = dVal;
	}
	else
	{
	    if (mStarIndex == 0)
	    {
	        mOutterRadius = dVal;
	    }
	    else
	    {
		    mOutterRadius_2 = dVal;
	    }
	}
	
	imageLabel->update();
	imageLabel->repaint();
		
}

void BrightnessMeasurementDlg::ComputeBrightness(void)
{
	char strPos[100] = "";

	if ((mPosX < 0) || (mPosY < 0))
		return;

	if (mMode == 0)
	{
		double dSkyBrightness;
		double dStarBrightness;
		int nCount = 0;
		
		dStarBrightness = CalculateMeanValue(mPosX, mPosY, 0, mInnerRadius, mHeight, mWidth, pImage, &nCount);
		dStarBrightness = (dStarBrightness-mMinGrayValue)/(mMaxGrayValue - mMinGrayValue)*nCount;

		dSkyBrightness = CalculateSkyBrightness(mPosX, mPosY, mInnerRadius, mOutterRadius, mHeight, mWidth, pImage);
		dStarBrightness = dStarBrightness - (dSkyBrightness-mMinGrayValue)/(mMaxGrayValue - mMinGrayValue)*nCount;
		dStarBrightness = 2.5*log10(dStarBrightness);

		sprintf(strPos, "Star Brightness: %.2f\0",dStarBrightness);
	}
	else
	{
		double dSkyBrightness;
		double dStarBrightness;
		int nCount = 0;

		if ((mPosX_2 < 0) || (mPosY_2 < 0))
		    return;
		
		dStarBrightness = CalculateMeanValue(mPosX, mPosY, 0, mInnerRadius, mHeight, mWidth, pImage, &nCount);
		dStarBrightness = (dStarBrightness-mMinGrayValue)/(mMaxGrayValue - mMinGrayValue)*nCount;

		dSkyBrightness = CalculateSkyBrightness(mPosX, mPosY, mInnerRadius, mOutterRadius, mHeight, mWidth, pImage);
		dStarBrightness = dStarBrightness - (dSkyBrightness-mMinGrayValue)/(mMaxGrayValue - mMinGrayValue)*nCount;

		double dSkyBrightness2;
		double dStarBrightness2;
		int nCount2 = 0;
		
		dStarBrightness2 = CalculateMeanValue(mPosX_2, mPosY_2, 0, mInnerRadius_2, mHeight, mWidth, pImage, &nCount2);
		dStarBrightness2 = (dStarBrightness2-mMinGrayValue)/(mMaxGrayValue - mMinGrayValue)*nCount2;
		
		dSkyBrightness2 = CalculateSkyBrightness(mPosX_2, mPosY_2, mInnerRadius_2, mOutterRadius_2, mHeight, mWidth, pImage);
		dStarBrightness2 = dStarBrightness2 - (dSkyBrightness2-mMinGrayValue)/(mMaxGrayValue - mMinGrayValue)*nCount2;

        dStarBrightness = 2.5*log10(dStarBrightness/dStarBrightness2);
		sprintf(strPos, "Relative Brightness: %.2f\0",dStarBrightness);
	}

	pStarBrightness->setText(strPos);

}

bool BrightnessMeasurementDlg::eventFilter(QObject* watched, QEvent* pEvent) 
{
	char strPos[100] = "";
	int x, y;

    if ( watched != imageLabel )
        return QObject::eventFilter(watched, pEvent);

    if (( pEvent->type() != QEvent::MouseButtonPress ) && ( pEvent->type() != QEvent::Paint ))
        return QObject::eventFilter(watched, pEvent);

	if ( pEvent->type() == QEvent::MouseButtonPress )
	{
        const QMouseEvent* const me = static_cast<const QMouseEvent*>( pEvent );
		x = me->x();
		y = me->y();

		ModifyCenter(&x, &y, mHeight, mWidth, pImage);
    
		if (mMode == 0)
		{
	        mPosX = x;
	        mPosY = y;

			mInnerRadius = CalculateInnerRadius(x, y, mHeight, mWidth, mMaxGrayLevel, pImage);
		    mOutterRadius = CalculateOutterRadius(x, y, mInnerRadius+2, mHeight, mWidth, mMaxGrayLevel, pImage);

		    pInRadius->setValue(mInnerRadius);
		    pOutRadius->setValue(mOutterRadius);
		}
		else
		{
			if (mStarIndex == 0)
			{
				mPosX = x;
	            mPosY = y;

				mInnerRadius = CalculateInnerRadius(x, y, mHeight, mWidth, mMaxGrayLevel, pImage);
		        mOutterRadius = CalculateOutterRadius(x, y, mInnerRadius+2, mHeight, mWidth, mMaxGrayLevel, pImage);

		        pInRadius->setValue(mInnerRadius);
		        pOutRadius->setValue(mOutterRadius);
			}
			else
			{
				mPosX_2 = x;
	            mPosY_2 = y;

				mInnerRadius_2 = CalculateInnerRadius(x, y, mHeight, mWidth, mMaxGrayLevel, pImage);
		        mOutterRadius_2 = CalculateOutterRadius(x, y, mInnerRadius_2+2, mHeight, mWidth, mMaxGrayLevel, pImage);

		        pInRadius->setValue(mInnerRadius_2);
		        pOutRadius->setValue(mOutterRadius_2);
			}
		}

	    sprintf(strPos, "Star position: %d, %d\0",x, y);
	

	    pStarPosition->setText(strPos);
		imageLabel->update();
		imageLabel->repaint();

		return true;
	}

	if ( pEvent->type() == QEvent::Paint )
	{
		QPainter painter(imageLabel);
	        
	    painter.drawPixmap(0, 0, mWidth, mHeight, QPixmap::fromImage(*qImage));

		if (mMode == 0)
		{
		    if (mPosX > 0 && mPosY > 0)
		    {  
                QPoint pPoint(mPosX , mPosY);
                painter.setPen( QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap));
                painter.drawEllipse(pPoint, (int)mInnerRadius , (int)mInnerRadius);
			    painter.drawEllipse(pPoint, (int)mOutterRadius , (int)mOutterRadius);
			}
		}
		else
		{
			if (mPosX > 0 && mPosY > 0)
		    {  
                QPoint pPoint(mPosX , mPosY);
                painter.setPen( QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap));
                painter.drawEllipse(pPoint, (int)mInnerRadius , (int)mInnerRadius);
			    painter.drawEllipse(pPoint, (int)mOutterRadius , (int)mOutterRadius);
			}

			if (mPosX_2 > 0 && mPosY_2 > 0)
		    {  
                QPoint pPoint(mPosX_2 , mPosY_2);
                painter.setPen( QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap));
                painter.drawEllipse(pPoint, (int)mInnerRadius_2 , (int)mInnerRadius_2);
			    painter.drawEllipse(pPoint, (int)mOutterRadius_2 , (int)mOutterRadius_2);
			}
		}
			
	}

    return true;
}

int BrightnessMeasurementDlg::CalculateInnerRadius(int x, int y, int rows, int cols, double maxGrayVal, double *pBuffer)
{
	double prevMean = -1;
	double currMean = 0;
	int nPixels;
	int i;
	  
    for (i=1; i<MAX_INNER_RADIUS; i++)
    {
        currMean = CalculateMeanValue(x, y, 0, i, rows, cols, pBuffer, &nPixels);
        
        if (i > 1)
        {
            if (abs(prevMean - currMean)/maxGrayVal > MEAN_STAR_DIFFERENCE_THRESHOLD)
            {
                break;
            }
        }
        
        prevMean = currMean;
        
    }
    
    return i+1;
}

int BrightnessMeasurementDlg::CalculateOutterRadius(int x, int y, int inRadius, int rows, int cols, double maxGrayVal, double *pBuffer)
{
	double prevMean = -1;
	double currMean = 0;
	int nPixels;
	int i;

    for (i=inRadius; i<MAX_OUTTER_RADIUS; i+=3)
    {
        currMean = CalculateMeanValue(x, y, inRadius, i, rows, cols, pBuffer, &nPixels);
        
        if (i > inRadius)
        {
            if (abs(prevMean - currMean)/maxGrayVal > MEAN_SKY_DIFFERENCE_THRESHOLD)
            {
                break;
            }
        }
        
        prevMean = currMean;
        
    }
    
    return i;
}    	

            
double BrightnessMeasurementDlg::CalculateMeanValue(int x, int y, int inRadius, int outRadius, int rows, int cols, double *pBuffer, int *nPixels)
{
    int currentX = x - outRadius;
    int currentY = y - outRadius;
    int currentDist;
    int nCount = 0;
    double meanValue = 0.0;
    
    for (int i=0; i<outRadius*2+1; i++)
    {
    	currentY = y - outRadius;
    	  
        for (int j=0; j<outRadius*2+1; j++)
        {   
            if ((currentX <0) || (currentX >= cols))
            {
                continue;
            }
            
            if ((currentY <0) || (currentY >= rows))
            {
                continue;
            }
            
            currentDist = (currentX - x)*(currentX - x) + (currentY - y)*(currentY - y);
            
            if ((currentDist < inRadius*inRadius) || (currentDist > outRadius*outRadius))
            {
                continue;
            }
            
            meanValue = meanValue + pBuffer[cols*currentY + currentX];
            nCount++;
            
            currentY++;
            
        }
        
        currentX++;
    }
    
    *nPixels = nCount;
    meanValue = meanValue/nCount;
    
    return meanValue;
    
}

void BrightnessMeasurementDlg::ModifyCenter(int *x, int *y, int rows, int cols, double *pBuffer)
{
	int outRadius = 2;
	int currentX = *x - outRadius;
    int currentY = *y - outRadius;
	int finalX = *x;
	int finalY = *y;

    double maxValue = -std::numeric_limits<double>::max();;
	double temp;
    
    for (int i=0; i<outRadius*2+1; i++)
    {
    	currentY = *y - outRadius;
    	  
        for (int j=0; j<outRadius*2+1; j++)
        {   
            if ((currentX <0) || (currentX >= cols))
            {
                continue;
            }
            
            if ((currentY <0) || (currentY >= rows))
            {
                continue;
            }
            
            temp = pBuffer[cols*currentY + currentX];
            if (temp > maxValue)
			{
				maxValue = temp;
				finalX = currentX;
				finalY = currentY;
			}
            
            currentY++;
            
        }
        
        currentX++;
    }

	*x = finalX;
	*y = finalY;
    
    return ;
}

void BrightnessMeasurementDlg::SortData(double *pData, int len)
{
	  double temp;
	  int nIndex;
	  
	  for (int i=0; i<len; i++)
	  {
	  	  temp = pData[i];
	  	  nIndex = i;
	  	  
	      for (int j=i+1; j< len; j++)
	      {
	          if (pData[j] < temp)
	          {
	              temp = pData[j];
	              nIndex = j;
	          }
	      }
	      
	      if (i != nIndex)
	      {
	          pData[nIndex] = pData[i];
	          pData[i] = temp;
	      }
	  }
}


double BrightnessMeasurementDlg::CalculateSkyBrightness(int x, int y, int inRadius, int outRadius, int rows, int cols, double *pBuffer)
{
    int currentX = x - outRadius;
    int currentY = y - outRadius;
    int currentDist;
    int nCount = 0;
    double meanValue = 0.0;
    double pData[(2*MAX_OUTTER_RADIUS+1)*(2*MAX_OUTTER_RADIUS+1)];
    
    for (int i=0; i<outRadius*2+1; i++)
    {
    	currentY = y - outRadius;
    	  
        for (int j=0; j<outRadius*2+1; j++)
        {   
            if ((currentX <0) || (currentX >= cols))
            {
                continue;
            }
            
            if ((currentY <0) || (currentY >= rows))
            {
                continue;
            }
            
            currentDist = (currentX - x)*(currentX - x) + (currentY - y)*(currentY - y);
            
            if ((currentDist < inRadius*inRadius) || (currentDist > outRadius*outRadius))
            {
                continue;
            }
            
            pData[nCount] = pBuffer[cols*currentY + currentX];
            nCount++;
            
            currentY++;
            
        }
        
        currentX++;
    }
    
    SortData(pData, nCount);
    
    int t1 = 0.2*nCount;
    int t2 = 0.8*nCount;
    
    nCount = 0;
    for (int i=t1; i<=t2; i++)
    {
        meanValue = meanValue + pData[i];
        nCount++;
    }
    
    meanValue = meanValue/nCount;
    
    return meanValue;
    
}