/*
 * The information in this file is
 * Copyright(c) 2011 Yiwei Zhang
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELET_K_SIGMA_THRESHOLD_DLG_H
#define WAVELET_K_SIGMA_THRESHOLD_DLG_H

#include <QtGui/QDialog>

#define MAX_WAVELET_LEVELS 5

class QRadioButton;
class QComboBox;
class QLineEdit;

class WaveletKSigmaDlg : public QDialog
{
   Q_OBJECT

public:
   WaveletKSigmaDlg(QWidget* pParent); 


private slots:
   void setThresholdMode();
   void setThreshold();
   void setCurrentLevel(int);

public:
   QRadioButton *pSoftButton;
   QRadioButton *pMedianButton;
   QRadioButton *pHeavyButton;
   QComboBox    *pLevelMenu;
   QLineEdit    *pThresholdEdit;
   double getLevelThreshold(int nLevel);
   

private:
	double mLevelThreshold[MAX_WAVELET_LEVELS];
    int mCurrentLevel;
};


#endif
