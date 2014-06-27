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
#include "DeconvolutionDlg.h"


#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>


using namespace std;

DeconvolutionDlg::DeconvolutionDlg(QWidget* pParent) : QDialog(pParent),
   pFilterMenu(NULL), pWindowSizeMenu(NULL), pGamaPara(NULL), pSigmaPara(NULL)
{
   setWindowTitle("Deconvolution Setting");

   mFilterType = 0; 
   mCurrentWindowSize = 7;
   mGamaVal = 0.6;
   mSigmaVal = 2;

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);

   QLabel* pLableMode = new QLabel("Filter type", this);
   pLayout->addWidget(pLableMode, 0, 0);

   pFilterMenu = new QComboBox(this);
   pFilterMenu->addItem("Van-Cittert method");
   pFilterMenu->addItem("Richardson-Lucy method");
   pFilterMenu->setCurrentIndex(0);
   pLayout->addWidget(pFilterMenu, 0, 1, 1, 2);

   QLabel* pLable1 = new QLabel("Window Size", this);
   pLayout->addWidget(pLable1, 1, 0);

   pWindowSizeMenu = new QComboBox(this);
   pWindowSizeMenu->addItem("5");
   pWindowSizeMenu->addItem("7");
   pWindowSizeMenu->addItem("9");
   pWindowSizeMenu->addItem("11");

   pWindowSizeMenu->setCurrentIndex(1);
   pLayout->addWidget(pWindowSizeMenu, 1, 1, 1, 2);

   
   QLabel* pLable3 = new QLabel("Gamma Value", this);
   pLayout->addWidget(pLable3, 2, 0, 1, 2);
   
   pGamaPara = new QDoubleSpinBox(this);
   pGamaPara->setRange(0, 1);
   pGamaPara->setSingleStep(0.05);
   pGamaPara->setValue(mGamaVal);
   pLayout->addWidget(pGamaPara, 2, 1, 1, 2);
   
   QLabel* pLable4 = new QLabel("Sigma Value", this);
   pLayout->addWidget(pLable4, 3, 0, 1, 2);
   
   pGamaPara = new QDoubleSpinBox(this);
   pGamaPara->setRange(1, 5);
   pGamaPara->setSingleStep(0.1);
   pGamaPara->setValue(mSigmaVal);
   pLayout->addWidget(pGamaPara, 3, 1, 1, 2);
   

   QHBoxLayout* pRespLayout = new QHBoxLayout;
   pLayout->addLayout(pRespLayout, 4, 0, 1, 3);

   QPushButton* pAccept = new QPushButton("OK", this);
   pRespLayout->addStretch();
   pRespLayout->addWidget(pAccept);

   QPushButton* pReject = new QPushButton("Cancel", this);
   pRespLayout->addWidget(pReject);

   connect(pAccept, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pReject, SIGNAL(clicked()), this, SLOT(reject()));
   

   connect(pFilterMenu, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentFilter(int)));
   connect(pWindowSizeMenu, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentWindowSize(int)));
   connect(pGamaPara,  SIGNAL(valueChanged(double)), this, SLOT(setGamaValue(double)));
   connect(pSigmaPara, SIGNAL(valueChanged(double)), this, SLOT(setSigmaValue(double)));
}


void DeconvolutionDlg::setGamaValue(double dVal)
{
	mGamaVal = dVal;
		
}

void DeconvolutionDlg::setSigmaValue(double dVal)
{
	mSigmaVal = dVal;
		
}

void DeconvolutionDlg::setCurrentFilter(int nIndex)
{
	mFilterType = nIndex;
		
}

void DeconvolutionDlg::setCurrentWindowSize(int nIndex)
{
	if (0 == nIndex)
	{
		mCurrentWindowSize = 5;
	}
	else if (1 == nIndex)
	{
		mCurrentWindowSize = 7;
	}
	else if (2 == nIndex)
	{
		mCurrentWindowSize = 9;
	}
	else
	{
		mCurrentWindowSize = 11;
	}
		
}

int DeconvolutionDlg::getCurrentFilterType()
{
	return mFilterType;
		
}

int DeconvolutionDlg::getCurrentWindowSize()
{
	return mCurrentWindowSize;
}

double DeconvolutionDlg::getGamaValue()
{
	return mGamaVal;
}

double DeconvolutionDlg::getSigmaValue()
{
	return mSigmaVal;
}

