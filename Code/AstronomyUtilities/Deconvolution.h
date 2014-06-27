/*
 * The information in this file is
 * Copyright(c) 2011 Yiwei Zhang
 * and is subject to the terms and conditions of the
 * GNU Lesser General Public License Version 2.1
 * The license text is available from   
 * http://www.gnu.org/licenses/lgpl.html
 */

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
