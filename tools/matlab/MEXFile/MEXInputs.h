//
//  MEXInputs.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/17/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef __MATLABTools__MEXInputs__
#define __MATLABTools__MEXInputs__

#include <set>
#include <string>
#include <vector>

#include <boost/filesystem/path.hpp>
#include <boost/static_assert.hpp>

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
    MEXInputs& operator>>(std::set<T> &values);
    
private:
    static void needMoreInputs();
    
    template<typename T>
    static T* getData(const mxArray *matrix) {
        BOOST_STATIC_ASSERT(TypeInfo<T>::is_numeric);
        return static_cast<T*>(mxGetData(matrix));
    }
    
    void invalidInput() const;
    
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
    const mxArray *arg = next();
    
    if ((mxGetClassID(arg) != TypeInfo<T>::class_id) || mxIsComplex(arg) || (mxGetNumberOfElements(arg) != 1)) {
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
    const mxArray *arg = next();
    
    if ((mxGetClassID(arg) != TypeInfo<T>::class_id) || mxIsComplex(arg)) {
        invalidInput();
    }
    
    const T* data = getData<T>(arg);
    std::size_t size = mxGetNumberOfElements(arg);
    values.assign(data, data+size);
    
    return (*this);
}


template<typename T>
MEXInputs& MEXInputs::operator>>(std::set<T> &values) {
    std::vector<T> data;
    (*this) >> data;
    values.clear();
    values.insert(data.begin(), data.end());
    return (*this);
}


END_NAMESPACE_MW_MATLAB


#endif  // !defined(__MATLABTools__MEXInputs__)

























