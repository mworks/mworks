//
//  CreateArray.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/10/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef MATLABTools_CreateArray_h
#define MATLABTools_CreateArray_h

#include <cstring>
#ifndef MATLAB_MEX_FILE
#  include <new>
#endif
#include <vector>

#include <boost/static_assert.hpp>

#include "TypeInfo.h"


BEGIN_NAMESPACE_MW_MATLAB


inline mxArray* createEmpty() {
    mxArray *result = mxCreateDoubleMatrix(0, 0, mxREAL);
#ifndef MATLAB_MEX_FILE
    if (!result) throw std::bad_alloc();
#endif
    return result;
}


template<typename T>
mxArray* createScalar(T value) {
    BOOST_STATIC_ASSERT(TypeInfo<T>::is_numeric);
    mxArray *result = mxCreateNumericMatrix(1, 1, TypeInfo<T>::class_id, mxREAL);
#ifndef MATLAB_MEX_FILE
    if (!result) throw std::bad_alloc();
#endif
    *(static_cast<T*>(mxGetData(result))) = value;
    return result;
}


template<>
inline mxArray* createScalar(double value) {
    mxArray *result = mxCreateDoubleScalar(value);
#ifndef MATLAB_MEX_FILE
    if (!result) throw std::bad_alloc();
#endif
    return result;
}


template<>
inline mxArray* createScalar(bool value) {
    mxArray *result = mxCreateLogicalScalar(value);
#ifndef MATLAB_MEX_FILE
    if (!result) throw std::bad_alloc();
#endif
    return result;
}


template<typename T>
mxArray* createVector(const T *values, std::size_t size) {
    BOOST_STATIC_ASSERT(TypeInfo<T>::is_numeric);
    mxArray *result = mxCreateNumericMatrix(1, size, TypeInfo<T>::class_id, mxREAL);
#ifndef MATLAB_MEX_FILE
    if (!result) throw std::bad_alloc();
#endif
    std::memcpy(mxGetData(result), values, size * sizeof(T));
    return result;
}


template<>
mxArray* createVector(const double *values, std::size_t size) {
    mxArray *result = mxCreateDoubleMatrix(1, size, mxREAL);
#ifndef MATLAB_MEX_FILE
    if (!result) throw std::bad_alloc();
#endif
    std::memcpy(mxGetPr(result), values, size * sizeof(double));
    return result;
}


template<typename T>
mxArray* createVector(const std::vector<T> &values) {
    return createVector(&(values.front()), values.size());
}


END_NAMESPACE_MW_MATLAB


#endif  // !defined(MATLABTools_CreateArray_h)


























