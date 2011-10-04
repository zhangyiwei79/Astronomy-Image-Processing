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
#include "LocalSharpeningDlg.h"


#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>


using namespace std;

LocalSharpeningDlg::LocalSharpeningDlg(QWidget* pParent) : QDialog(pParent),
   pFilterMenu(NULL), pWindowSizeMenu(NULL), pContrastSlider(NULL)
{
   setWindowTitle("Local Sharpening");

   mFilterType = 0; 
   mCurrentWindowSize = 7;
   mContrastVal = 8.0;

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);

   QLabel* pLableMode = new QLabel("Filter Type", this);
   pLayout->addWidget(pLableMode, 0, 0);

   pFilterMenu = new QComboBox(this);
   pFilterMenu->addItem("Adaptive Sharpening");
   pFilterMenu->addItem("Extreme Value Operator");
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

   
   QLabel* pLable3 = new QLabel("Contrast", this);
   pLayout->addWidget(pLable3, 2, 0, 1, 2);
   
   pContrastSlider = new QDoubleSpinBox(this);
   pContrastSlider->setRange(1, 20);
   pContrastSlider->setSingleStep(1);
   pContrastSlider->setValue(mContrastVal);
   pLayout->addWidget(pContrastSlider, 2, 1, 1, 2);
   

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
   connect(pContrastSlider, SIGNAL(valueChanged(double)), this, SLOT(setContrastValue(double)));
}


void LocalSharpeningDlg::setContrastValue(double dVal)
{
	mContrastVal = dVal;
		
}

void LocalSharpeningDlg::setCurrentFilter(int nIndex)
{
	mFilterType = nIndex;
		
}

void LocalSharpeningDlg::setCurrentWindowSize(int nIndex)
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

int LocalSharpeningDlg::getCurrentFilterType()
{
	return mFilterType;
		
}

int LocalSharpeningDlg::getCurrentWindowSize()
{
	return mCurrentWindowSize;
}

double LocalSharpeningDlg::getContrastValue()
{
	return mContrastVal;
}

