/*
 * The information in this file is
 * Copyright(c) 2007 Ball Aerospace & Technologies Corporation
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef LOCAL_SHARPENING_DLG_H
#define LOCAL_SHARPENING_DLG_H

#include <QtGui/QDialog>

class QComboBox;
class QDoubleSpinBox;

class LocalSharpeningDlg : public QDialog
{
   Q_OBJECT

public:
   LocalSharpeningDlg(QWidget* pParent); 


private slots:
   void setCurrentFilter(int nIndex);
   void setCurrentWindowSize(int nIndex);
   void setContrastValue(double dVal);

public:
   QComboBox    *pFilterMenu;
   QComboBox    *pWindowSizeMenu;
   QDoubleSpinBox    *pContrastSlider;
   int getCurrentFilterType();
   int getCurrentWindowSize();
   double getContrastValue();

private:
	int mCurrentWindowSize;
	int mFilterType;
	double mContrastVal;
};

#endif
