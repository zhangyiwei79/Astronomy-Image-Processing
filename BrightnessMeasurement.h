#ifndef ZYW_BRIGHTNESS_MEASUREMENT_H
#define ZYW_BRIGHTNESS_MEASUREMENT_H

#include "ExecutableShell.h"

class BrightnessMeasurement : public ExecutableShell
{
public:
   BrightnessMeasurement();
   virtual ~BrightnessMeasurement();

   virtual bool getInputSpecification(PlugInArgList*& pInArgList);
   virtual bool getOutputSpecification(PlugInArgList*& pOutArgList);
   virtual bool execute(PlugInArgList* pInArgList, PlugInArgList* pOutArgList);
   
   double *pOriginalImage;
};

#endif
