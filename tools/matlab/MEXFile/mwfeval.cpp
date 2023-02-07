//
//  mwfeval.cpp
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/10/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#include "MWKFile.hpp"


BEGIN_NAMESPACE_MW_MATLAB


extern "C"
MW_SYMBOL_PUBLIC
void mexFunction(int nlhs, mxArray *plhs[],
                 int nrhs, const mxArray *prhs[])
{
    MEXInputs inputs(nrhs, prhs);
    MEXOutputs outputs(nlhs, plhs);
    
    std::string functionName;
    inputs >> functionName;
    
    if (functionName == "MWKFile/open") {
        MWKFile::open(inputs, outputs);
    } else if (functionName == "MWKFile/close") {
        MWKFile::close(inputs, outputs);
    } else if (functionName == "MWKFile/getNumEvents") {
        MWKFile::getNumEvents(inputs, outputs);
    } else if (functionName == "MWKFile/getMinTime") {
        MWKFile::getMinTime(inputs, outputs);
    } else if (functionName == "MWKFile/getMaxTime") {
        MWKFile::getMaxTime(inputs, outputs);
    } else if (functionName == "MWKFile/selectEvents") {
        MWKFile::selectEvents(inputs, outputs);
    } else if (functionName == "MWKFile/nextEvent") {
        MWKFile::nextEvent(inputs, outputs);
    } else if (functionName == "MWKFile/getEvents") {
        MWKFile::getEvents(inputs, outputs);
    } else {
        throwMATLABError("MWorks:UnknownFunctionName",
                         boost::format("Unknown function name: %s") % functionName);
    }
}


END_NAMESPACE_MW_MATLAB
