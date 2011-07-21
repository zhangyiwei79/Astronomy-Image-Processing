/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
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
#include <QtGui/QScrollArea>
#include <QtGui/QMessagebox.h>
#include <QtGui/QEvent.h>
#include <QtGui/QPainter.h>


using namespace std;

#define MAX_INNER_RADIUS 15
#define MAX_OUTTER_RADIUS 30
#define MEAN_STAR_DIFFERENCE_THRESHOLD   0.1
#define MEAN_SKY_DIFFERENCE_THRESHOLD   0.05

BrightnessMeasurementDlg::BrightnessMeasurementDlg(QWidget* pParent, double *pBuffer, int imgWidth, int imgHeight, double t1, double t2) : QDialog(pParent),
   pStarPosition(NULL), pStarBrightness(NULL), pSkyBrightness(NULL), pInRadius(NULL), pOutRadius(NULL), pCompute(NULL), imageLabel(NULL), qImage(NULL)
{
   setWindowTitle("Brightness Measurement");
   
   pImage = pBuffer;
   mWidth = imgWidth;
   mHeight = imgHeight;
   mMinGrayValue = t1;
   mMaxGrayValue = t2;
   mInnerRadius = 2;
   mOutterRadius = 8;
   mPosX = -1;
   mPosY = -1;
   mMaxGrayLevel = t2 - t1;

   DisplayBuffer = (unsigned char *)malloc(sizeof(unsigned char)*mWidth*mHeight*4);
   unsigned long nCount= 0, nIndex = 0;
   for (nIndex=0; nIndex<mWidth*mHeight; nIndex++)
   {
	   DisplayBuffer[nCount] = pImage[nIndex];
	   DisplayBuffer[nCount+1] = pImage[nIndex];
	   DisplayBuffer[nCount+2] = pImage[nIndex];
	   DisplayBuffer[nCount+3] = 0xFF;
	   nCount += 4;
   }

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);

   QLabel* pLable1 = new QLabel("Inner Radius", this);
   pLayout->addWidget(pLable1, 0, 10);

   pInRadius = new QDoubleSpinBox(this);
   pInRadius->setRange(1, MAX_INNER_RADIUS);
   pInRadius->setSingleStep(1);
   pInRadius->setValue(mInnerRadius);
   pLayout->addWidget(pInRadius, 0, 11);

   QLabel* pLable2 = new QLabel("Outter Radius", this);
   pLayout->addWidget(pLable2, 1, 10);

   pOutRadius = new QDoubleSpinBox(this);
   pOutRadius->setRange(1, MAX_OUTTER_RADIUS);
   pOutRadius->setSingleStep(1);
   pOutRadius->setValue(mOutterRadius);
   pLayout->addWidget(pOutRadius, 1, 11);

   
   pStarPosition = new QLabel("Star Position:", this);
   pLayout->addWidget(pStarPosition, 2, 10, 1, 2);
   
   pStarBrightness = new QLabel("Star brightness:", this);
   pLayout->addWidget(pStarBrightness, 3, 10, 1, 2);
   
   
   pSkyBrightness = new QLabel("Sky brightness:", this);
   pLayout->addWidget(pSkyBrightness, 4, 10, 1, 2);
   

   QHBoxLayout* pRespLayout = new QHBoxLayout;
   pLayout->addLayout(pRespLayout, 5, 10, 1, 2);

   pCompute = new QPushButton("Compute", this);
   pRespLayout->addStretch();
   pRespLayout->addWidget(pCompute);

   QPushButton* pReject = new QPushButton("Close", this);
   pRespLayout->addWidget(pReject);

   connect(pCompute, SIGNAL(clicked()), this, SLOT(ComputerBrightness()));
   connect(pReject,  SIGNAL(clicked()), this, SLOT(reject()));
   

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
    
    pLayout->addWidget(scrollArea, 0, 0, 6, 10);
   
    resize(600, 300);

}

BrightnessMeasurementDlg::~BrightnessMeasurementDlg()
{
	free(DisplayBuffer);
}


void BrightnessMeasurementDlg::setInnerRadius(double dVal)
{
	mInnerRadius = dVal;

	imageLabel->update();
	imageLabel->repaint();
		
}

void BrightnessMeasurementDlg::setOutterRadius(double dVal)
{
	mOutterRadius = dVal;
	
	imageLabel->update();
	imageLabel->repaint();
		
}

void BrightnessMeasurementDlg::ComputeBrightness(void)
{
		;
}

bool BrightnessMeasurementDlg::eventFilter(QObject* watched, QEvent* pEvent) 
{
	char strPos[100] = "";

    if ( watched != imageLabel )
        return QObject::eventFilter(watched, pEvent);

    if (( pEvent->type() != QEvent::MouseButtonPress ) && ( pEvent->type() != QEvent::Paint ))
        return QObject::eventFilter(watched, pEvent);

	if ( pEvent->type() == QEvent::MouseButtonPress )
	{
        const QMouseEvent* const me = static_cast<const QMouseEvent*>( pEvent );
    
	    mPosX = me->x();
	    mPosY = me->y();

	    sprintf(strPos, "Star position: %d, %d\0", mPosX, mPosY);

		mInnerRadius = CalculateInnerRadius(mPosX, mPosY, mHeight, mWidth, mMaxGrayLevel, pImage);
		mOutterRadius = CalculateOutterRadius(mPosX, mPosY, mInnerRadius+2, mHeight, mWidth, mMaxGrayLevel, pImage);

		pInRadius->setValue(mInnerRadius);
		pOutRadius->setValue(mOutterRadius);

	    pStarPosition->setText(strPos);
		imageLabel->update();
		imageLabel->repaint();

		return true;
	}

	if ( pEvent->type() == QEvent::Paint )
	{
		QPainter painter(imageLabel);
	        
	    painter.drawPixmap(0, 0, mWidth, mHeight, QPixmap::fromImage(*qImage));

		if (mPosX > 0 && mPosY > 0)
		{  
            QPoint pPoint(mPosX , mPosY);
            painter.setPen( QPen(Qt::white, 1, Qt::SolidLine, Qt::RoundCap));
            painter.drawEllipse(pPoint, (int)mInnerRadius , (int)mInnerRadius);
			painter.drawEllipse(pPoint, (int)mOutterRadius , (int)mOutterRadius);
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
	char strPos[100] = "";
	  
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

	sprintf(strPos, "Star brightness: %.2f, %.2f\0", prevMean, currMean);

	pStarBrightness->setText(strPos);
    
    return i+1;
}

int BrightnessMeasurementDlg::CalculateOutterRadius(int x, int y, int inRadius, int rows, int cols, double maxGrayVal, double *pBuffer)
{
	double prevMean = -1;
	double currMean = 0;
	int nPixels;
	int i;
	char strPos[100] = "";

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

	sprintf(strPos, "Sky brightness: %.2f, %.2f\0", prevMean, currMean);

	pSkyBrightness->setText(strPos);
    
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
            
            


