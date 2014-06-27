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
#include "WaveletKSigmaDlg.h"


#include <QtGui/QLabel>
#include <QtGui/QLayout>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>


using namespace std;

WaveletKSigmaDlg::WaveletKSigmaDlg(QWidget* pParent) : QDialog(pParent),
   pSoftButton(NULL), pMedianButton(NULL), pHeavyButton(NULL), pLevelMenu(NULL), pThresholdEdit(NULL)
{
   setWindowTitle("Wavelet K-Sigma Threshold");

   mLevelThreshold[0] = 6;
   mLevelThreshold[1] = 5;
   mLevelThreshold[2] = 4;
   mLevelThreshold[3] = 3;
   mLevelThreshold[4] = 2;

   QGridLayout* pLayout = new QGridLayout(this);
   pLayout->setMargin(10);
   pLayout->setSpacing(5);

   QLabel* pLableMode = new QLabel("Set Mode", this);
   pLayout->addWidget(pLableMode, 0, 0);

   pSoftButton = new QRadioButton("Gentle",this);
   pLayout->addWidget(pSoftButton, 1, 0);

   pMedianButton = new QRadioButton("Normal",this);
   pLayout->addWidget(pMedianButton, 2, 0);
   pMedianButton->setChecked(true);

   pHeavyButton = new QRadioButton("Strong",this);
   pLayout->addWidget(pHeavyButton, 3, 0);

   QLabel* pLable1 = new QLabel("Select Scale", this);
   pLayout->addWidget(pLable1, 0, 1, 1, 2);

   pLevelMenu = new QComboBox(this);
   pLevelMenu->addItem("1");
   pLevelMenu->addItem("2");
   pLevelMenu->addItem("3");
   pLevelMenu->addItem("4");
   pLevelMenu->addItem("5");
   pLevelMenu->setCurrentIndex(0);
   pLayout->addWidget(pLevelMenu, 1, 1, 1, 2);
   mCurrentLevel = 0;

   QLabel* pLable2 = new QLabel("Set K-Sigma Value", this);
   pLayout->addWidget(pLable2, 2, 1, 1, 2);

   pThresholdEdit = new QLineEdit(this);
   pThresholdEdit->setText("6.00");
   pThresholdEdit->setMaxLength(10);
   pLayout->addWidget(pThresholdEdit, 3, 1, 1, 2);

   QHBoxLayout* pRespLayout = new QHBoxLayout;
   pLayout->addLayout(pRespLayout, 4, 0, 1, 3);

   QPushButton* pAccept = new QPushButton("OK", this);
   pRespLayout->addStretch();
   pRespLayout->addWidget(pAccept);

   QPushButton* pReject = new QPushButton("Cancel", this);
   pRespLayout->addWidget(pReject);

   connect(pAccept, SIGNAL(clicked()), this, SLOT(accept()));
   connect(pReject, SIGNAL(clicked()), this, SLOT(reject()));
   
   connect(pSoftButton, SIGNAL(clicked()), this, SLOT(setThresholdMode()));
   connect(pMedianButton, SIGNAL(clicked()), this, SLOT(setThresholdMode()));
   connect(pHeavyButton, SIGNAL(clicked()), this, SLOT(setThresholdMode()));

   connect(pLevelMenu, SIGNAL(currentIndexChanged(int)), this, SLOT(setCurrentLevel(int)));
   connect(pThresholdEdit, SIGNAL(editingFinished()), this, SLOT(setThreshold()));
}

void WaveletKSigmaDlg::setThresholdMode()
{
	char tempStr[100] = {0};

	if (pSoftButton->isChecked())
	{
	    mLevelThreshold[0] = 4;
		mLevelThreshold[1] = 3;
		mLevelThreshold[2] = 2;
		mLevelThreshold[3] = 1;
		mLevelThreshold[4] = 0;
	}
	else if (pMedianButton->isChecked())
	{
		mLevelThreshold[0] = 6;
		mLevelThreshold[1] = 5;
		mLevelThreshold[2] = 4;
		mLevelThreshold[3] = 3;
		mLevelThreshold[4] = 2;
	}
	else
	{
		mLevelThreshold[0] = 8;
		mLevelThreshold[1] = 7;
		mLevelThreshold[2] = 6;
		mLevelThreshold[3] = 5;
		mLevelThreshold[4] = 4;
	}

	if (mCurrentLevel < MAX_WAVELET_LEVELS)
	{
		sprintf(tempStr, "%.2f", mLevelThreshold[mCurrentLevel]);
		pThresholdEdit->setText(tempStr);
	}
}


void WaveletKSigmaDlg::setCurrentLevel(int nIndex)
{
	char tempStr[50] = {0};
	mCurrentLevel = nIndex;

	if (nIndex < MAX_WAVELET_LEVELS)
	{
		sprintf(tempStr, "%.2f", mLevelThreshold[nIndex]);
		pThresholdEdit->setText(tempStr);
	}
}

void WaveletKSigmaDlg::setThreshold()
{
	if (mCurrentLevel < MAX_WAVELET_LEVELS)
	{
		mLevelThreshold[mCurrentLevel] = pThresholdEdit->text().toDouble();
	}
}

double WaveletKSigmaDlg::getLevelThreshold(int nLevel)
{
	if (nLevel < MAX_WAVELET_LEVELS)
	{
		return mLevelThreshold[nLevel];
	}
	else
	{
		return 0;
	}
}

