/*
 * The information in this file is
 * Copyright(c) 2011 Yiwei Zhang
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef HISTOGRAM_SHAPING_DLG_H
#define HISTOGRAM_SHAPING_DLG_H

#include <QtGui/QDialog>


class QDoubleSpinBox;

class HistogramShapingDlg : public QDialog
{
   Q_OBJECT

public:
   HistogramShapingDlg(QWidget* pParent, double peakValue); 


private slots:
   void setMeanValue(double t);
   void setSigmaValueBox(double t);

public:
   QDoubleSpinBox* mMeanValueBox;
   QDoubleSpinBox* mSigmaValueBox;

   double getMeanValue();
   double getSigmaValue();

private:
   double mMeanValue;
   double mSigmaValue;
};

#endif
