//
//  getEvents.cpp
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/10/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#include <mex.h>

#include "CreateArray.h"


BEGIN_NAMESPACE_MW_MATLAB


static void getEvents(int nlhs, mxArray *plhs[],
                      int nrhs, const mxArray *prhs[])
{
    mexPrintf("Hello, world!\n");
    if (nlhs > 0) {
        plhs[0] = createScalar(12);
    }
}


END_NAMESPACE_MW_MATLAB


__attribute__((visibility("default")))
void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    mw::matlab::getEvents(nlhs, plhs, nrhs, prhs);
}
