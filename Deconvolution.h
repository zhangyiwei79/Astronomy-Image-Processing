#ifndef ZYW_DECONVOLUTION_H
#define ZYW_DECONVOLUTION_H

#include "ExecutableShell.h"

class Deconvolution : public ExecutableShell
{
public:
   Deconvolution();
   virtual ~Deconvolution();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   
   double *pOriginalImage;
};

#endif
