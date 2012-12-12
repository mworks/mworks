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
#include <vector>

#include <boost/static_assert.hpp>

#include "TypeInfo.h"


BEGIN_NAMESPACE_MW_MATLAB


inline mxArray* createEmpty() {
    ArrayPtr result(mxCreateDoubleMatrix(0, 0, mxREAL));
    return result.release();
}


template<typename T>
mxArray* createScalar(T value) {
    BOOST_STATIC_ASSERT(TypeInfo<T>::is_numeric);
    ArrayPtr result(mxCreateNumericMatrix(1, 1, TypeInfo<T>::class_id, mxREAL));
    *(static_cast<T*>(mxGetData(result.get()))) = value;
    return result.release();
}


template<>
inline mxArray* createScalar(double value) {
    ArrayPtr result(mxCreateDoubleScalar(value));
    return result.release();
}


template<>
inline mxArray* createScalar(bool value) {
    ArrayPtr result(mxCreateLogicalScalar(value));
    return result.release();
}


template<typename T>
mxArray* createVector(const T *values, std::size_t size) {
    BOOST_STATIC_ASSERT(TypeInfo<T>::is_numeric);
    ArrayPtr result(mxCreateNumericMatrix(1, size, TypeInfo<T>::class_id, mxREAL));
    std::memcpy(mxGetData(result.get()), values, size * sizeof(T));
    return result.release();
}


template<>
mxArray* createVector(const double *values, std::size_t size) {
    ArrayPtr result(mxCreateDoubleMatrix(1, size, mxREAL));
    std::memcpy(mxGetPr(result.get()), values, size * sizeof(double));
    return result.release();
}


template<typename T>
mxArray* createVector(const std::vector<T> &values) {
    return createVector(&(values.front()), values.size());
}


END_NAMESPACE_MW_MATLAB


#endif  // !defined(MATLABTools_CreateArray_h)


























