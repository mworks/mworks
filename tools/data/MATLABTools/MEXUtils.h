//
//  MEXUtils.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 8/15/12.
//
//

#ifndef MATLABTools_MEXUtils_h
#define MATLABTools_MEXUtils_h

#include <string>
#include <vector>

#include <matrix.h>
#include <mex.h>

#define MWORKS_INVALID_INPUT_MSG_ID "MWorks:InvalidInput"


inline void getStringParameter(const mxArray *prhs[], int paramNumber, std::string &result) {
    const mxArray *param = prhs[paramNumber-1];
    if (!mxIsChar(param)) {
        mexErrMsgIdAndTxt(MWORKS_INVALID_INPUT_MSG_ID, "Input argument %d must be a character array.", paramNumber);
        return;
    }
    
    char *data = mxArrayToString(param);
    if (data != NULL) {
        result = data;
        mxFree(data);
    }
}


template <typename Type>
Type getNumericScalarParameter(const mxArray *prhs[], int paramNumber) {
    const mxArray *param = prhs[paramNumber-1];
    
    if (!mxIsNumeric(param) || mxIsEmpty(param) || (mxGetNumberOfElements(param) != 1)) {
        mexErrMsgIdAndTxt(MWORKS_INVALID_INPUT_MSG_ID, "Input argument %d must be a numeric scalar.", paramNumber);
        return Type(0);
    }
    
    return Type(mxGetScalar(param));
}


template <typename Type>
void getNumericArrayParameter(const mxArray *prhs[], int paramNumber, std::vector<Type> &result) {
    const mxArray *param = prhs[paramNumber-1];
    
    if (!mxIsDouble(param) || mxIsComplex(param)) {
        mexErrMsgIdAndTxt(MWORKS_INVALID_INPUT_MSG_ID,
                          "Input argument %d must be a real-valued, double-precision, floating-point array.",
                          paramNumber);
        return;
    }
    
    const size_t numElements = mxGetNumberOfElements(param);
    double *values = mxGetPr(param);
    for (size_t i = 0; i < numElements; i++) {
        result.push_back(Type(values[i]));
    }
}


#endif /* !defined(MATLABTools_MEXUtils_h) */


























