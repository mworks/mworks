//
//  MEXInputs.cpp
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/17/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#include "MEXInputs.h"


BEGIN_NAMESPACE_MW_MATLAB


void MEXInputs::needMoreInputs() {
    throwMATLABError("MWorks:NotEnoughInputs", "Not enough input arguments");
}


void MEXInputs::invalidInput() const {
    throwMATLABError("MWorks:InvalidInput", boost::format("Input argument %d is not valid") % (currentInput + 1));
}


template<>
MEXInputs& MEXInputs::operator>>(std::string& value) {
    auto arg = next();
    char *data = nullptr;
    
    if (!mxIsChar(arg) || !(data = mxArrayToUTF8String(arg))) {
        invalidInput();
    }
    
    value = data;
    mxFree(data);
    
    return (*this);
}


template<>
MEXInputs& MEXInputs::operator>>(boost::filesystem::path& value) {
    std::string path;
    (*this) >> path;
    value = path;
    return (*this);
}


END_NAMESPACE_MW_MATLAB
