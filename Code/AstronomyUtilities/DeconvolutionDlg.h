/*
 * The information in this file is
 * Copyright(c) 2011 Yiwei Zhang
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef DECONVOLUTION_PARAMETER_DLG_H
#define DECONVOLUTION_PARAMETER_DLG_H

#include <QtGui/QDialog>

class QComboBox;
class QDoubleSpinBox;

class DeconvolutionDlg : public QDialog
{
   Q_OBJECT

public:
   DeconvolutionDlg(QWidget* pParent); 


private slots:
   void setCurrentFilter(int nIndex);
   void setCurrentWindowSize(int nIndex);
   void setGamaValue(double dVal);
   void setSigmaValue(double dVal);

public:
   QComboBox    *pFilterMenu;
   QComboBox    *pWindowSizeMenu;
   QDoubleSpinBox    *pGamaPara;
   QDoubleSpinBox    *pSigmaPara;
   
   int getCurrentFilterType();
   int getCurrentWindowSize();
   double getGamaValue();
   double getSigmaValue();

private:
	int mCurrentWindowSize;
	int mFilterType;
	double mGamaVal;
	double mSigmaVal;
};

#endif
