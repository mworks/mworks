//
//  mwfeval.cpp
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/10/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#include "getEvents.h"

using namespace mw::matlab;


MW_SYMBOL_PUBLIC
void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    MEXInputs inputs(nrhs, prhs);
    MEXOutputs outputs(nlhs, plhs);
    
    std::string functionName;
    inputs >> functionName;
    
    if (functionName == "getEvents") {
        getEvents(inputs, outputs);
    } else {
        throwMATLABError("MWorks:UnknownFunctionName",
                         boost::format("Unknown function name: %s") % functionName);
    }
}



























