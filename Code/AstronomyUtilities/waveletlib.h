/*
 * The information in this file is
 * Copyright(c) 2011 Yiwei Zhang
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef	_WAVELET_H_
#define _WAVELET_H_

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

typedef struct _WaveletNode WaveletNode;
struct _WaveletNode
{
  struct _WaveletNode *sibling;
    
	double *pVer;
	double *pHor;
	double *pDiag;
	double *pLow;

	int coeffRow;
	int coeffCol;
	int nLayers;

};

void ShiftInvariantInverseWaveletTransform(int row, int col, double *pLoFilter, double *pHiFilter, int filterLen, int nLayer, WaveletNode *pNodeList);
void ShiftInvariantWaveletTransform(double *pSrc, int row, int col, double *pLoFilter, double *pHiFilter, int filterLen, int nLayer, WaveletNode *pNodeList);
void WaveletDenoise(WaveletNode *pNodeList, double *pBuffer, int nLayer, double *pKSigma);
void ReleaseList(WaveletNode *pNodeList, int nLayer);


#endif

