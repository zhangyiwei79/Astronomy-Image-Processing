/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef BRIGHTNESS_MEASUREMENT_DLG_H
#define BRIGHTNESS_MEASUREMENT_DLG_H

#include <QtGui/QDialog>
#include "TypesFile.h"


class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QScrollArea;
class QScrollBar;
class QComboBox;

class BrightnessMeasurementDlg : public QDialog
{
   Q_OBJECT

public:
   BrightnessMeasurementDlg(QWidget* pParent, double *pBuffer, int imgWidth, int imgHeight, double t1, double t2, EncodingType type); 
   ~BrightnessMeasurementDlg();

private slots:
   void setInnerRadius(double radiusValue);
   void setOutterRadius(double radiusValue);
   void ComputeBrightness(void);
   
   void setMeasurementMode(int nIndex);
   void setStarIndex(int nIndex);

public:
   
   QLabel            *pStarPosition;
   QLabel            *pStarBrightness;
   QLabel            *pSkyBrightness;
   
   QComboBox         *pMode;
   QComboBox         *pStarIndex;
   
   QDoubleSpinBox    *pInRadius;
   QDoubleSpinBox    *pOutRadius;
   
   QPushButton       *pCompute;
   
   QLabel *imageLabel;
   QScrollArea *scrollArea;
   QImage      *qImage;

   double CalculateMeanValue(int x, int y, int inRadius, int outRadius, int rows, int cols, double *pBuffer, int *nPixels);
   int CalculateInnerRadius(int x, int y, int rows, int cols, double maxGrayVal, double *pBuffer);
   int CalculateOutterRadius(int x, int y, int inRadius, int rows, int cols, double maxGrayVal, double *pBuffer);

   void ModifyCenter(int *x, int *y, int rows, int cols, double *pBuffer);
   void SortData(double *pData, int len);
   double CalculateSkyBrightness(int x, int y, int inRadius, int outRadius, int rows, int cols, double *pBuffer);

private:
	double mInnerRadius;
	double mOutterRadius;
	
	double mInnerRadius_2;
	double mOutterRadius_2;
	
	int mPosX;
	int mPosY;
	
	int mPosX_2;
	int mPosY_2;
	
	int mMode;
	int mStarIndex;
	
	double mStarBrightness;
	double mSkyBrightness;
	
	double *pImage;
	unsigned char *DisplayBuffer;
	int    mWidth;
	int    mHeight;

	double mMaxGrayLevel;
	double mMinGrayValue;
	double mMaxGrayValue;

protected:
     bool eventFilter(QObject *obj, QEvent *pEvent);

};

#endif
