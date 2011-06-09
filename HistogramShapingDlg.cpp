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
#include "HistogramShapingDlg.h"

#include <QtGui/QDoubleSpinBox>
#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>

using namespace std;

HistogramShapingDlg::HistogramShapingDlg(QWidget* pParent, double peakValue) : QDialog(pParent),
   mMeanValueBox(NULL), mSigmaValueBox(NULL)
{
   setWindowTitle("Gaussian Histogram Shaping");
   
   mMeanValue  = peakValue;
   mSigmaValue = 3.0;

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);

   QLabel* pLable1 = new QLabel("Peak Skew: ", this);
   pLayout->addWidget(pLable1, 0, 0);

   mMeanValueBox = new QDoubleSpinBox(this);
   mMeanValueBox->setRange(0, 1);
   mMeanValueBox->setSingleStep(0.01);
   mMeanValueBox->setValue(mMeanValue);
   pLayout->addWidget(mMeanValueBox, 0, 1, 1, 2);


   QLabel* pLable2 = new QLabel("Sigma Value: ", this);
   pLayout->addWidget(pLable2, 1, 0);

   mSigmaValueBox = new QDoubleSpinBox(this);
   mSigmaValueBox->setRange(0.1, 6);
   mSigmaValueBox->setSingleStep(0.1);
   mSigmaValueBox->setValue(mSigmaValue);
   pLayout->addWidget(mSigmaValueBox, 1, 1, 1, 2);


   QHBoxLayout* pRespLayout = new QHBoxLayout;
   pLayout->addLayout(pRespLayout, 2, 0, 1, 3);

   QPushButton* pAccept = new QPushButton("OK", this);
   pRespLayout->addStretch();
   pRespLayout->addWidget(pAccept);

   QPushButton* pReject = new QPushButton("Cancel", this);
   pRespLayout->addWidget(pReject);

   connect(pAccept, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pReject, SIGNAL(clicked()), this, SLOT(reject()));
   
   connect(mMeanValueBox, SIGNAL(valueChanged(double)), this, SLOT(setMeanValue(double)));
   connect(mSigmaValueBox, SIGNAL(valueChanged(double)), this, SLOT(setSigmaValueBox(double)));
  
}




void HistogramShapingDlg::setMeanValue(double t)
{
	mMeanValue = t;
}

void HistogramShapingDlg::setSigmaValueBox(double t)
{
	mSigmaValue = t;
}



double HistogramShapingDlg::getMeanValue()
{
	return mMeanValue;
}

double HistogramShapingDlg::getSigmaValue()
{
	return mSigmaValue;
}



