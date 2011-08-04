#ifndef ZYW_IMAGE_REGISTRATION_H
#define ZYW_IMAGE_REGISTRATION_H

#include "ExecutableShell.h"

class ImageRegistration : public ExecutableShell
{
public:
   ImageRegistration();
   virtual ~ImageRegistration();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   
};

#endif
