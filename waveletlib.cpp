

#include "waveletlib.h"
#include "math.h"


void ColConvolution2D(double *pSrc, double *pFilter, int filterLength, int row, int col, double *pRes, bool bAddZero)
{
	double temp, a, b;
	int i,j,k;
	double *pData;

	if ((pSrc == NULL) || (pRes == NULL))
	{
		return;
	}

	pData = (double *)malloc(sizeof(double)*(row+2*(filterLength-1)));

	for (k=0; k<col; k++)
	{
		for (i=0; i<(filterLength-1); i++)
		{		
			if (!bAddZero)
			{
			    *(pData + i) = *(pSrc + (filterLength-2)*col -i*col);
			}
			else
			{
				*(pData + i) = 0;
			}
		}
		pData = pData + filterLength - 1;

		for (i=0; i<row; i++)
		{
			*(pData + i) = *(pSrc+i*col);
		}
		pData = pData + row;

		for (i=0; i<(filterLength-1); i++)
		{		
			if (!bAddZero)
			{
			    *(pData + i) = *(pSrc+col*(row-1)-i*col);
			}
			else
			{
				*(pData + i) = 0;
			}
		}

		pData = pData - row -filterLength + 1;

	    for (i=0; i<(row+filterLength-1); i++)
	    {
		    temp = 0;

		    for (j=0; j<filterLength; j++)
		    {
			    a = *(pFilter + j);
			    b = *(pData + i + j);
			    temp = temp + a*b;
		    }
		
		    *(pRes + k + i*col) = temp;
		}

		pSrc++;

	}

	free(pData);
}


void RowConvolution2D(double *pSrc, double *pFilter, int filterLength, int row, int col, double *pRes, bool bAddZero, bool bAddResult)
{
	double temp, a, b;
	double *pData;
	int i,j,k;

	if ((pSrc == NULL) || (pRes == NULL))
	{
		return;
	}

	pData = (double *)malloc(sizeof(double)*(col+2*(filterLength-1)));

	for (k=0; k<row; k++)
	{
		for (i=0; i<(filterLength-1); i++)
		{
			if (!bAddZero)
			{
			    *(pData + i) = *(pSrc + filterLength - 2 - i);
			}
			else
			{
				*(pData + i) = 0;
			}
		}
		pData = pData + filterLength - 1;

		for (i=0; i<col; i++)
		{
			*(pData + i) = *(pSrc+i);
		}
		pData = pData + col;

		for (i=0; i<(filterLength-1); i++)
		{		
			if (!bAddZero)
			{
			    *(pData + i) = *(pSrc+col-1-i);
			}
			else
			{
				*(pData + i) = 0;
			}
		}

        pData = pData - col -filterLength + 1;

	    for (i=0; i<(col+filterLength-1); i++)
	    {
		    temp = 0;

		    for (j=0; j<filterLength; j++)
		    {
			    a = *(pFilter + j);
				b = *(pData + i + j);

			    temp = temp + a*b;
		    }
		
			if (bAddResult)
			{
				*pRes = *pRes + temp;
			}
			else
			{
		        *pRes = temp;
			}
			pRes++;
		}

		pSrc = pSrc + col;
	}

	free(pData);
}




void IDWT_Partial(double *pSrc, int row, int col, int orig_row, int orig_col, double *pFilter1, double *pFilter2, int filterLen, double *pResult)
{
	int count = 0;

	double *pTemp1 = (double *)malloc(sizeof(double)*(2*row-1)*col);
	double *pTemp2 = (double *)malloc(sizeof(double)*(2*row-1+filterLen-1)*col);
	double *pTemp3 = (double *)malloc(sizeof(double)*(2*row-1+filterLen-1)*(2*col-1));
	

	//Up Sample rows
	for (int i=0; i<2*row-1; i++)
	{
		for (int j=0; j<col; j++)
		{
			if (i%2 == 1)
			{
				*(pTemp1+i*col+j) = 0;
			}
			else
			{
				*(pTemp1+i*col+j) = *(pSrc+i/2*col+j);
			}
		}
	}

	ColConvolution2D(pTemp1, pFilter1, filterLen, 2*row-1, col, pTemp2, true);

	//Up Sample cols
	count = 2*row-1+filterLen-1;
	for (int i=0; i<count; i++)
	{
		for (int j=0; j<(2*col-1); j++)
		{
			if (j%2 == 1)
			{
				*(pTemp3+i*(col*2-1)+j) = 0;
			}
			else
			{
				*(pTemp3+i*(col*2-1)+j) = *(pTemp2 + i*col + j/2);
			}
		}
	}

	RowConvolution2D(pTemp3, pFilter2, filterLen, count, (2*col-1), pResult, true, true);

	free(pTemp1);
	free(pTemp2);
	free(pTemp3);

}

void InverseWaveletTransform2D(double *pLow, double *pHor, double *pVer, double *pDiag,  int row, int col, int orig_row, int orig_col, double *pLoFilter, double *pHiFilter, int filterLen, double *pResult, int row_index, int col_index)
{
	int row_shift = 0;
	int col_shift = 0;
	int index = 0;

	double *pTemp = (double *)malloc(sizeof(double)*(2*row-1+filterLen-1)*(2*col-1+filterLen-1));
	memset(pTemp, 0, sizeof(double)*(2*row-1+filterLen-1)*(2*col-1+filterLen-1));

	IDWT_Partial(pLow,  row, col, orig_row, orig_col, pLoFilter, pLoFilter, filterLen, pTemp);
	IDWT_Partial(pHor,  row, col, orig_row, orig_col, pHiFilter, pLoFilter, filterLen, pTemp);
	IDWT_Partial(pVer,  row, col, orig_row, orig_col, pLoFilter, pHiFilter, filterLen, pTemp);
	IDWT_Partial(pDiag, row, col, orig_row, orig_col, pHiFilter, pHiFilter, filterLen, pTemp);

	if ((orig_row + filterLen - 1)%2 == 0)
	{
		if ( 0 == row_index)
		{
		    row_shift = filterLen - 1;//modify, zyw
		}
		else
		{
			row_shift = filterLen - 2;//modify, zyw
		}

	}
	else
	{
		if (row*2 < (orig_row + filterLen - 1))
		{
			row_shift = filterLen - 2;
		}
		else
		{
			row_shift = filterLen - 1;
		}
	}

	if ((orig_col + filterLen - 1)%2 == 0)
	{	
		if ( 0 == col_index)
		{
		    col_shift = filterLen - 1;//modify, zyw
		}
		else
		{
			col_shift = filterLen - 2;//modify, zyw
		}
	}
	else
	{
		if (col*2 < (orig_col + filterLen - 1))
		{
			col_shift = filterLen - 2;
		}
		else
		{
			col_shift = filterLen - 1;
		}
	}
	col = 2*col-1+filterLen-1;
	index = col*row_shift + col_shift;

	//Keep the center part as result
	for (int i=0; i<orig_row; i++)
	{
		for (int j=0; j<orig_col; j++)
		{
			*(pResult+i*orig_col+j) = *(pResult+i*orig_col+j) + *(pTemp+index);
			index++;
		}
		index = index + (col-orig_col);
	}

	free(pTemp);
}

void WaveletTransform2D(double *pSrc, int row, int col, double *pLoFilter, double *pHiFilter, int filterLen, double *pLow, double *pVer, double *pHor, double *pDiag)
{
	int nIndex = 0;
	int coeff_row = row + filterLen -1;
	int coeff_col = col + filterLen - 1;
	
	double *pTemp1 = (double *)malloc(sizeof(double)*row*coeff_col);
	double *pTemp2 = (double *)malloc(sizeof(double)*coeff_row*coeff_col);
	
	RowConvolution2D(pSrc, pLoFilter, filterLen, row, col, pTemp1, false, false);
	
	//Get the Low frquency part
	ColConvolution2D(pTemp1, pLoFilter, filterLen, row, col+filterLen-1, pTemp2, false);
	//Downsample the coefficient
	nIndex = 0;
	for (int i=0; i<coeff_row; i++)
	{
		for(int j=0; j<coeff_col; j++)
		{
			//if ((i%2 == 0) && (j%2 == 1))
			{
		        pLow[nIndex] = pTemp2[i*coeff_col+j];
				nIndex++;
			}
		}
	}
	
	//Get Horizontal coefficients
	ColConvolution2D(pTemp1, pHiFilter, filterLen, row, col+filterLen-1, pTemp2, false);
	nIndex = 0;
	for (int i=0; i<coeff_row; i++)
	{
		for(int j=0; j<coeff_col; j++)
		{
			//if ((i%2 == 0) && (j%2 == 1))
			{
		        pHor[nIndex] = pTemp2[i*coeff_col+j];
				nIndex++;
			}
		}
	}

	RowConvolution2D(pSrc, pHiFilter, filterLen, row, col, pTemp1, false, false);
	//Get vertical coefficients
	ColConvolution2D(pTemp1, pLoFilter, filterLen, row, col+filterLen-1, pTemp2, false);
	nIndex = 0;
	for (int i=0; i<coeff_row; i++)
	{
		for(int j=0; j<coeff_col; j++)
		{
			//if ((i%2 == 0) && (j%2 == 1))
			{
		        pVer[nIndex] = pTemp2[i*coeff_col+j];
				nIndex++;
			}
		}
	}
	
	//Get the diagonal coefficients
	ColConvolution2D(pTemp1, pHiFilter, filterLen, row, col+filterLen-1, pTemp2, false);
	nIndex = 0;
	for (int i=0; i<coeff_row; i++)
	{
		for(int j=0; j<coeff_col; j++)
		{
			//if ((i%2 == 0) && (j%2 == 1))
			{
		        pDiag[nIndex] = pTemp2[i*coeff_col+j];
				nIndex++;
			}
		}
	}
	
	free(pTemp1);
	free(pTemp2);
	
	return;
}

void NodeTransform(WaveletNode *pParent,double *pLoFilter, double *pHiFilter, int filterLen, WaveletNode *pCurrentNode)
{
	double *pSrc, *pLow, *pVer, *pHor, *pDiag;
	WaveletNode *pNode = pParent;
	WaveletNode *pNewNode;
	int m, n, nIndex, row, col;

	if (NULL == pNode)
	{
		return;
	}

	while(pNode != NULL)
	{
		pSrc = (double *)malloc(sizeof(double)*(pNode->coeffRow/2+1)*(pNode->coeffCol/2+1));

		for (int k=0; k<4; k++)
		{
			pNewNode = (WaveletNode *)malloc(sizeof(WaveletNode));
			memset(pNewNode, 0, sizeof(WaveletNode));

			nIndex = 0;

			if (k == 0)
			{
				if (pNode->coeffRow % 2 == 1)
				{
					row = pNode->coeffRow/2+1;
				}
				else
				{
					row = pNode->coeffRow/2;
				}

				if (pNode->coeffCol % 2 == 1)
				{
					col = pNode->coeffCol/2+1;
				}
				else
				{
					col = pNode->coeffCol/2;
				}

				m = 0; n = 0;
			}
			if (k == 1)
			{
				if (pNode->coeffRow % 2 == 1)
				{
					row = pNode->coeffRow/2+1;
				}
				else
				{
					row = pNode->coeffRow/2;
				}

                col = pNode->coeffCol/2;

				m = 0; n = 1;
			}
            if (k == 2)
			{
				row = pNode->coeffRow/2;

				if (pNode->coeffCol % 2 == 1)
				{
					col = pNode->coeffCol/2+1;
				}
				else
				{
					col = pNode->coeffCol/2;
				}
				m = 1; n = 0;
			}

			if (k == 3)
			{
				row = pNode->coeffRow/2;
				col = pNode->coeffCol/2;
				m = 1; n = 1;
			}

			pDiag = (double *)malloc(sizeof(double)*(row+filterLen-1)*(col+filterLen-1));
			pVer = (double *)malloc(sizeof(double)*(row+filterLen-1)*(col+filterLen-1));
			pHor = (double *)malloc(sizeof(double)*(row+filterLen-1)*(col+filterLen-1));
			pLow = (double *)malloc(sizeof(double)*(row+filterLen-1)*(col+filterLen-1));

			pNewNode->pDiag = pDiag;
			pNewNode->pHor = pHor;
			pNewNode->pVer = pVer;
			pNewNode->pLow = pLow;

			pNewNode->coeffCol = col+filterLen-1;
			pNewNode->coeffRow = row+filterLen-1;

			for (int i=0; i<pNode->coeffRow; i++)
	        {
		        for(int j=0; j<pNode->coeffCol; j++)
		        {
			        if ((i%2 == m) && (j%2 == n))
			        {
						pSrc[nIndex] = pNode->pLow[i*pNode->coeffCol+j];
				        nIndex++;
			        }
		        }
			}

			WaveletTransform2D(pSrc, row, col, pLoFilter, pHiFilter, filterLen, pLow, pVer, pHor, pDiag);

			pCurrentNode->sibling = pNewNode;
			pCurrentNode = pNewNode;
		}

		pNode = pNode->sibling;
		free(pSrc);
	}
	
}
		

void ShiftInvariantWaveletTransform(double *pSrc, int row, int col, double *pLoFilter, double *pHiFilter, int filterLen, int nLayer, WaveletNode *pNodeList)
{
	WaveletNode *pParent, *pRootNode;

	//Generate base node
	pRootNode = (WaveletNode *)malloc(sizeof(WaveletNode));
	memset(pRootNode, 0 , sizeof(WaveletNode));
	pRootNode->pLow = pSrc;
	pRootNode->coeffCol = col;
	pRootNode->coeffRow = row;
	pNodeList->sibling = pRootNode;
	
	//Generate first layer
	pRootNode = (WaveletNode *)malloc(sizeof(WaveletNode));
	memset(pRootNode, 0 , sizeof(WaveletNode));
    double *pDiag = (double *)malloc(sizeof(double)*(row+filterLen-1)*(col+filterLen-1));
	double *pVer = (double *)malloc(sizeof(double)*(row+filterLen-1)*(col+filterLen-1));
	double *pHor = (double *)malloc(sizeof(double)*(row+filterLen-1)*(col+filterLen-1));
	double *pLow = (double *)malloc(sizeof(double)*(row+filterLen-1)*(col+filterLen-1));

	pRootNode->pDiag = pDiag;
	pRootNode->pHor = pHor;
	pRootNode->pVer = pVer;
	pRootNode->pLow = pLow;
	pRootNode->coeffRow = row+filterLen-1;
	pRootNode->coeffCol = col+filterLen-1;

	WaveletTransform2D(pSrc, row, col, pLoFilter, pHiFilter, filterLen, pLow, pVer, pHor, pDiag);
	pParent = pRootNode;
	(pNodeList+1)->sibling = pParent;

	//Generate the rest layers
	for (int i=2; i<=nLayer; i++)
	{
		NodeTransform(pParent,pLoFilter, pHiFilter, filterLen, (pNodeList+i));
		pParent = (pNodeList+i)->sibling;
	}

}

void NodeInverseTransform(WaveletNode *pParent,double *pLoFilter, double *pHiFilter, int filterLen, WaveletNode *pCurrentNode, int shift)
{
	WaveletNode *pNode = pCurrentNode;

	int m,n,s=1,t=1,nIndex = 0;
	int origRow = pParent->coeffRow/2, origCol = pParent->coeffCol/2;
	int row = pNode->coeffRow/2+1, col = pNode->coeffCol/2+1;

	if (shift == 0)
	{
	    if (pParent->coeffRow % 2 == 1)
		{
		    origRow = origRow + 1;
		}
	    if (pParent->coeffCol % 2 == 1)
		{
		    origCol = origCol + 1;
		}
		s = 0; t = 0;
	}
	if (shift == 1)
	{
	    if (pParent->coeffRow % 2 == 1)
		{
		    origRow = origRow + 1;
		}
		s = 0; t = 1;
	}
	if (shift == 2)
	{
	    if (pParent->coeffCol % 2 == 1)
		{
		    origCol = origCol + 1;
		}
		s= 1; t = 0;
	}
	
	if (shift < 0)
	{
		origRow = pParent->coeffRow;
		origCol = pParent->coeffCol;
	}

	double *pResult = (double *)malloc(sizeof(double)*origRow*origCol);
	double *pFinalResult = (double *)malloc(sizeof(double)*origRow*origCol);
	memset(pFinalResult, 0, sizeof(double)*origRow*origCol);

	double *pDiag = (double *)malloc(sizeof(double)*row*col);
    double *pVer = (double *)malloc(sizeof(double)*row*col);
	double *pHor = (double *)malloc(sizeof(double)*row*col);
	double *pLow = (double *)malloc(sizeof(double)*row*col);


	for (int k=0; k<4; k++)
	{
	    nIndex = 0;

		if (k == 0)
			{
				if (pNode->coeffRow % 2 == 1)
				{
					row = pNode->coeffRow/2+1;
				}
				else
				{
					row = pNode->coeffRow/2;
				}

				if (pNode->coeffCol % 2 == 1)
				{
					col = pNode->coeffCol/2+1;
				}
				else
				{
					col = pNode->coeffCol/2;
				}

				m = 0; n = 0;
			}
			if (k == 1)
			{
				if (pNode->coeffRow % 2 == 1)
				{
					row = pNode->coeffRow/2+1;
				}
				else
				{
					row = pNode->coeffRow/2;
				}

                col = pNode->coeffCol/2;

				m = 0; n = 1;
			}
            if (k == 2)
			{
				row = pNode->coeffRow/2;

				if (pNode->coeffCol % 2 == 1)
				{
					col = pNode->coeffCol/2+1;
				}
				else
				{
					col = pNode->coeffCol/2;
				}
				m = 1; n = 0;
			}

			if (k == 3)
			{
				row = pNode->coeffRow/2;
				col = pNode->coeffCol/2;
				m = 1; n = 1;
			}

		for (int i=0; i<pNode->coeffRow; i++)
	    {
		    for(int j=0; j<pNode->coeffCol; j++)
		    {
			    if ((i%2 == m) && (j%2 == n))
			    {
				    pLow[nIndex] = pNode->pLow[i*pNode->coeffCol+j];
					pVer[nIndex] = pNode->pVer[i*pNode->coeffCol+j];
					pHor[nIndex] = pNode->pHor[i*pNode->coeffCol+j];
					pDiag[nIndex] = pNode->pDiag[i*pNode->coeffCol+j];
				    nIndex++;
			        
		        }
			}		
		}

		memset(pResult, 0, sizeof(double)*origRow*origCol);
		InverseWaveletTransform2D(pLow, pHor, pVer, pDiag,  row, col, origRow, origCol, pLoFilter, pHiFilter, filterLen, pResult,m, n);

		for (int i=0; i<origRow; i++)
	    {
		    for(int j=0; j<origCol; j++)
		    {
				*(pFinalResult + i*origCol + j) = *(pFinalResult + i*origCol + j) + (*(pResult + i*origCol + j))/4;
			}
		}

	}

	//Restore the result
	nIndex = 0;
	if (shift >= 0)
	{
	    for (int i=0; i<pParent->coeffRow; i++)
	    {
            for(int j=0; j<pParent->coeffCol; j++)
		    {
	            if ((i%2 == s) && (j%2 == t))
			    {
				    pParent->pLow[i*pParent->coeffCol+j] = pFinalResult[nIndex];
				    nIndex++;
				}
			}		
		}
	}
	else
	{
	    for (int i=0; i<pParent->coeffRow; i++)
	    {
            for(int j=0; j<pParent->coeffCol; j++)
		    {
				pParent->pLow[i*pParent->coeffCol+j] = pFinalResult[nIndex];
				nIndex++;
			}		
		}
	}

	free(pLow);
	free(pVer);
	free(pHor);
	free(pDiag);

	free(pResult);
	free(pFinalResult);

}

	




void ShiftInvariantInverseWaveletTransform(int row, int col, double *pLoFilter, double *pHiFilter, int filterLen, int nLayer, WaveletNode *pNodeList)
{
	WaveletNode *pParent, *pCurrent, *pTemp1, *pTemp2;

	pParent = pNodeList + nLayer - 1;
	pCurrent = pNodeList + nLayer;

	for (int i=0; i< (nLayer-1); i++)
	{
		pTemp1 = pParent->sibling;
		pTemp2 = pCurrent->sibling;

		while(pTemp1 != NULL)
		{
			for (int j=0; j<4; j++)
			{
				NodeInverseTransform(pTemp1, pLoFilter, pHiFilter, filterLen, pTemp2, j);

				pTemp2 = pTemp2->sibling;
			}

			pTemp1 = pTemp1->sibling;
		}

		pParent--;
		pCurrent--;
	}

	NodeInverseTransform(pParent->sibling, pLoFilter, pHiFilter, filterLen, pCurrent->sibling, -1);

}

void ReleaseList(WaveletNode *pNodeList, int nLayer)
{
	WaveletNode *pNode, *pTemp;

	for (int i=0; i<nLayer; i++)
	{
		pNode = (pNodeList+i+1)->sibling;

		while(pNode!= NULL)
		{
			free(pNode->pDiag);
			free(pNode->pLow);
			free(pNode->pHor);
			free(pNode->pVer);

			pTemp = pNode;
		    pNode= pNode->sibling;

			free(pTemp);
		}
	}

	free(pNodeList->sibling);
}

int PartitionSequence(double *pData, int left, int right, int pivotIndex)
{
	double TempVal;
	int storeIndex = left;
    double pivotValue = pData[pivotIndex];

    //  swap list[pivotIndex] and list[right] , Move pivot to end
    pData[pivotIndex] = pData[right];
	pData[right] = pivotValue;

     for (int i = left; i <= (right-1); i++)
	 {
         if (pData[i] < pivotValue)
		 {
             //swap list[storeIndex] and list[i]
             TempVal = pData[storeIndex];
	         pData[storeIndex] = pData[i];
             pData[i] = TempVal;

             storeIndex = storeIndex + 1;
		 }
	 }

     //swap list[right] and list[storeIndex]  // Move pivot to its final place
     TempVal = pData[storeIndex];
	 pData[storeIndex] = pData[right];
     pData[right] = TempVal;

     return storeIndex;
}

double SelectMedian(double *list, int left, int right, int k)
{
    int pivotNewIndex;
	int pivotIndex;

    while(1)
	{
         pivotIndex = left;
         
		 pivotNewIndex = PartitionSequence(list, left, right, pivotIndex);
         
		 if (k == pivotNewIndex)
             return list[k];
         else if (k < pivotNewIndex)
             right = pivotNewIndex-1;
         else
             left = pivotNewIndex+1;
	}
}


double CalculateNoiseSigma(double *pData, int nLength)
{
	double noiseSigma = 0;
	int k;
	
	if (k%2 == 0)
	    k= nLength/2-1;
	else
		k= nLength/2;

	noiseSigma = SelectMedian(pData, 0, (nLength-1), k);
	noiseSigma = noiseSigma/0.674;

	return noiseSigma;
}

void SoftThreshold(double *pData, double thresHold, int nLength)
{
	for (int i=0; i<nLength; i++)
	{
		if (pData[i] > thresHold)
		{
            pData[i] = pData[i];// - thresHold;
		}
		else if (pData[i] < -thresHold)
		{
            pData[i] = pData[i];// + thresHold;
		}
		else
		{
			pData[i] = 0;
		}
	}
}


void WaveletDenoise(WaveletNode *pNodeList, double *pBuffer, int nLayer, double *pScaleKSigma)
{
	WaveletNode *pNode, *pTemp;
	double thresHold;
	double globalSigma;
	double weightSigma[5] = {0.8, 0.27, 0.12, 0.058, 0.029};
	int nCount;

	pNode = pNodeList + 1;
	nCount = pNode->sibling->coeffRow*pNode->sibling->coeffCol;

	for (int i=0; i<nCount; i++)
	{
		pBuffer[i] = abs(pNode->sibling->pDiag[i]);
	}
	
	globalSigma = CalculateNoiseSigma(pBuffer, nCount);

	for (int i=0; i< nLayer; i++)
	{
		pTemp = pNode->sibling;
		thresHold = globalSigma*pScaleKSigma[i]*weightSigma[i];

		while(pTemp != NULL)
		{
			SoftThreshold(pTemp->pVer, thresHold, pTemp->coeffRow*pTemp->coeffCol);
			SoftThreshold(pTemp->pHor, thresHold, pTemp->coeffRow*pTemp->coeffCol);
			SoftThreshold(pTemp->pDiag, thresHold, pTemp->coeffRow*pTemp->coeffCol);

			pTemp = pTemp->sibling;
		}

		pNode++;
	}
}
