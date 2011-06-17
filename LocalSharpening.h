#ifndef ZYW_LOCAL_SHARPENING_H
#define ZYW_LOCAL_SHARPENING_H

#include "ExecutableShell.h"

class LocalSharpening : public ExecutableShell
{
public:
   LocalSharpening();
   virtual ~LocalSharpening();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
};

#endif
