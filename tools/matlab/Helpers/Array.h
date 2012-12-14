//
//  Array.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/14/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef MATLABTools_Array_h
#define MATLABTools_Array_h

#include <algorithm>
#include <string>
#include <vector>

#include <boost/container/vector.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>

#include "TypeInfo.h"


BEGIN_NAMESPACE_MW_MATLAB


class Array {
    
private:
    template<typename T>
    static ArrayPtr createMatrix(mwSize rows, mwSize cols);
    
    template<typename T>
    static T* getData(const ArrayPtr &matrix);
    
public:
    template<typename T>
    static ArrayPtr createEmpty();
    
    template<typename T>
    static ArrayPtr createScalar(T value);
    
    template<typename T>
    static ArrayPtr createVector(const T *values, std::size_t size);
    
    template<typename T>
    static ArrayPtr createVector(const std::vector<T> &values);
    
    static ArrayPtr createVector(/*non-const*/ boost::container::vector<ArrayPtr> &values);
    
    static ArrayPtr createString(const char *str);
    static ArrayPtr createString(const std::string &str);
    
};


template<typename T>
inline ArrayPtr Array::createMatrix(mwSize rows, mwSize cols) {
    BOOST_STATIC_ASSERT(TypeInfo<T>::is_numeric);
    return ArrayPtr(throw_if_null, mxCreateNumericMatrix(rows, cols, TypeInfo<T>::class_id, mxREAL));
}


template<>
inline ArrayPtr Array::createMatrix<double>(mwSize rows, mwSize cols) {
    return ArrayPtr(throw_if_null, mxCreateDoubleMatrix(rows, cols, mxREAL));
}


template<>
inline ArrayPtr Array::createMatrix<mxLogical>(mwSize rows, mwSize cols) {
    return ArrayPtr(throw_if_null, mxCreateLogicalMatrix(rows, cols));
}


template<>
inline ArrayPtr Array::createMatrix<ArrayPtr>(mwSize rows, mwSize cols) {
    return ArrayPtr(throw_if_null, mxCreateCellMatrix(rows, cols));
}


template<typename T>
inline T* Array::getData(const ArrayPtr &matrix) {
    BOOST_STATIC_ASSERT(TypeInfo<T>::is_numeric);
    return static_cast<T*>(mxGetData(matrix.get()));
}


template<>
inline double* Array::getData<double>(const ArrayPtr &matrix) {
    return mxGetPr(matrix.get());
}


template<>
inline mxLogical* Array::getData<mxLogical>(const ArrayPtr &matrix) {
    return mxGetLogicals(matrix.get());
}


template<typename T>
inline ArrayPtr Array::createEmpty() {
    return createMatrix<T>(0, 0);
}


template<typename T>
inline ArrayPtr Array::createScalar(T value) {
    ArrayPtr result = createMatrix<T>(1, 1);
    *(getData<T>(result)) = value;
    return boost::move(result);
}


template<>
inline ArrayPtr Array::createScalar(double value) {
    return ArrayPtr(throw_if_null, mxCreateDoubleScalar(value));
}


template<>
inline ArrayPtr Array::createScalar(mxLogical value) {
    return ArrayPtr(throw_if_null, mxCreateLogicalScalar(value));
}


template<typename T>
inline ArrayPtr Array::createVector(const T *values, std::size_t size) {
    if (!size) {
        return createEmpty<T>();
    }
    mxAssert(values, "values is NULL");
    ArrayPtr result = createMatrix<T>(1, size);
    std::copy(values, values+size, getData<T>(result));
    return boost::move(result);
}


template<typename T>
inline ArrayPtr Array::createVector(const std::vector<T> &values) {
    // std::vector<bool> is specialized and doesn't necessarily store its data as a contiguous
    // array of bool's, so we can't treat it like other types
    BOOST_STATIC_ASSERT(!(boost::is_same<T, bool>::value));
    return createVector(&(values.front()), values.size());
}


inline ArrayPtr Array::createVector(/*non-const*/ boost::container::vector<ArrayPtr> &values) {
    if (values.empty()) {
        return createEmpty<ArrayPtr>();
    }
    ArrayPtr result = createMatrix<ArrayPtr>(1, values.size());
    for (std::size_t i = 0; i < values.size(); i++) {
        mxSetCell(result.get(), i, values[i].release());
    }
    return boost::move(result);
}


inline ArrayPtr Array::createString(const char *str) {
    mxAssert(str, "str is NULL");
    return ArrayPtr(throw_if_null, mxCreateString(str));
}


inline ArrayPtr Array::createString(const std::string &str) {
    return createString(str.c_str());
}


END_NAMESPACE_MW_MATLAB


#endif  // !defined(MATLABTools_Array_h)



























