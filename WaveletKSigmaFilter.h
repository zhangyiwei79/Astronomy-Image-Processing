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
