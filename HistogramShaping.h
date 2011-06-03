#ifndef ZYW_HISTOGRAM_SHAPING_H
#define ZYW_HISTOGRAM_SHAPING_H

#include "ExecutableShell.h"

class HistogramShaping : public ExecutableShell
{
public:
   HistogramShaping();
   virtual ~HistogramShaping();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif
