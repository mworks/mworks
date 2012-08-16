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

#include <matrix.h>
#include <mex.h>

#define MWORKS_WRONG_NUM_INPUTS_MSG_ID  "MWorks:WrongNumberOfInputs"
#define MWORKS_WRONG_NUM_OUTPUTS_MSG_ID "MWorks:WrongNumberOfOutputs"
#define MWORKS_INVALID_INPUT_MSG_ID     "MWorks:InvalidInput"


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


inline double getNumericScalarParameter(const mxArray *prhs[], int paramNumber) {
    const mxArray *param = prhs[paramNumber-1];
    
    if (!mxIsNumeric(param) || mxIsComplex(param) || (mxGetNumberOfElements(param) != 1)) {
        mexErrMsgIdAndTxt(MWORKS_INVALID_INPUT_MSG_ID,
                          "Input argument %d must be a real-valued, numeric scalar.",
                          paramNumber);
        return 0.0;
    }
    
    return mxGetScalar(param);
}


inline const double* getNumericArrayParameter(const mxArray *prhs[], int paramNumber, size_t &numElements) {
    const mxArray *param = prhs[paramNumber-1];
    
    if (!mxIsDouble(param) || mxIsComplex(param)) {
        mexErrMsgIdAndTxt(MWORKS_INVALID_INPUT_MSG_ID,
                          "Input argument %d must be a real-valued, double-precision, floating-point array.",
                          paramNumber);
        return NULL;
    }
    
    numElements = mxGetNumberOfElements(param);
    return mxGetPr(param);
}


#endif /* !defined(MATLABTools_MEXUtils_h) */


























