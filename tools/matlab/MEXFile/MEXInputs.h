//
//  MEXInputs.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/17/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef __MATLABTools__MEXInputs__
#define __MATLABTools__MEXInputs__

#include "TypeInfo.h"


BEGIN_NAMESPACE_MW_MATLAB


class MEXInputs {
    
public:
    MEXInputs(int nrhs, const mxArray **prhs) :
        numInputs(nrhs),
        inputs(prhs),
        currentInput(-1)
    { }
    
    int count() const {
        return (numInputs - (currentInput + 1));
    }
    
    template<typename T>
    MEXInputs& operator>>(T& value);
    
    template<typename T>
    MEXInputs& operator>>(std::vector<T> &values);
    
    template<typename T>
    MEXInputs& operator>>(std::unordered_set<T> &values);
    
private:
    [[noreturn]] static void needMoreInputs();
    
    template<typename T>
    static const T* getData(const mxArray *array) {
        return TypeInfo<T>::getData(array);
    }
    
    [[noreturn]] void invalidInput() const;
    
    const mxArray* next() {
        if (count() < 1) needMoreInputs();
        currentInput++;
        return inputs[currentInput];
    }
    
    const int numInputs;
    const mxArray ** const inputs;
    int currentInput;
    
};


template<typename T>
MEXInputs& MEXInputs::operator>>(T& value) {
    auto arg = next();
    
    if ((mxGetClassID(arg) != TypeInfo<T>::classID) || mxIsComplex(arg) || (mxGetNumberOfElements(arg) != 1)) {
        invalidInput();
    }
    
    value = *(getData<T>(arg));
    
    return (*this);
}


template<>
MEXInputs& MEXInputs::operator>>(std::string& value);


template<>
MEXInputs& MEXInputs::operator>>(boost::filesystem::path& value);


template<typename T>
MEXInputs& MEXInputs::operator>>(std::vector<T> &values) {
    auto arg = next();
    
    if ((mxGetClassID(arg) != TypeInfo<T>::classID) || mxIsComplex(arg)) {
        invalidInput();
    }
    
    auto data = getData<T>(arg);
    auto size = mxGetNumberOfElements(arg);
    values.assign(data, data+size);
    
    return (*this);
}


template<typename T>
MEXInputs& MEXInputs::operator>>(std::unordered_set<T> &values) {
    std::vector<T> data;
    (*this) >> data;
    values.clear();
    values.insert(data.begin(), data.end());
    return (*this);
}


END_NAMESPACE_MW_MATLAB


#endif  // !defined(__MATLABTools__MEXInputs__)
