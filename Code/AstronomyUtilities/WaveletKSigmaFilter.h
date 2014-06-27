/*
 * The information in this file is
 * Copyright(c) 2011 Yiwei Zhang
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

#ifndef WAVELET_K_SIGMA_FILTER_H
#define WAVELET_K_SIGMA_FILTER_H

#include "ExecutableShell.h"

class WaveletKSigmaFilter : public ExecutableShell
{
public:
   WaveletKSigmaFilter();
   virtual ~WaveletKSigmaFilter();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);

   unsigned int rowBlocks;
   unsigned int colBlocks;
   double *pBuffer;
   
};

#endif
