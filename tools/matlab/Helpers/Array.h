//
//  Array.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/14/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef MATLABTools_Array_h
#define MATLABTools_Array_h

#include "TypeInfo.h"


BEGIN_NAMESPACE_MW_MATLAB


class Array {
    
private:
    template<typename T>
    static ArrayPtr createMatrix(mwSize rows, mwSize cols);
    
    template<typename T>
    static ArrayPtr createVector(mwSize size);
    
    template<typename T>
    static T * getData(const ArrayPtr &array);
    
public:
    template<typename T>
    static ArrayPtr createEmpty();
    
    template<typename T>
    static ArrayPtr createScalar(T value);
    
    template<typename T>
    static ArrayPtr createVector(const T *values, std::size_t size);
    
    template<typename T>
    static ArrayPtr createVector(const std::vector<T> &values);
    
    static ArrayPtr createString(const std::string &str);
    static ArrayPtr createCell(std::vector<ArrayPtr> &&values);
    
    using FieldNames = std::vector<const char *>;
    using FieldValues = std::vector<ArrayPtr>;
    static ArrayPtr createStruct(mwSize size, const FieldNames &fieldNames);
    static ArrayPtr createStruct(const FieldNames &fieldNames, FieldValues &&fieldValues);
    
    template<typename... Args>
    static ArrayPtr createStruct(const FieldNames &fieldNames, Args&&... args);
    
private:
    template<typename... Args>
    static ArrayPtr createStruct(const FieldNames &fieldNames,
                                 FieldValues &&fieldValues,
                                 ArrayPtr &&array,
                                 Args&&... args);
    
};


template<typename T>
inline ArrayPtr Array::createMatrix(mwSize rows, mwSize cols) {
    static_assert(TypeInfo<T>::isNumeric);
    return ArrayPtr(throw_if_null, mxCreateUninitNumericMatrix(rows, cols, TypeInfo<T>::classID, mxREAL));
}


template<>
inline ArrayPtr Array::createMatrix<mxLogical>(mwSize rows, mwSize cols) {
    return ArrayPtr(throw_if_null, mxCreateLogicalMatrix(rows, cols));
}


template<typename T>
inline ArrayPtr Array::createVector(mwSize size) {
    return createMatrix<T>((size ? 1 : 0), size);
}


template<typename T>
inline T * Array::getData(const ArrayPtr &array) {
    return TypeInfo<T>::getData(array.get());
}


template<typename T>
inline ArrayPtr Array::createEmpty() {
    return createVector<T>(0);
}


template<typename T>
inline ArrayPtr Array::createScalar(T value) {
    auto result = createVector<T>(1);
    *(getData<T>(result)) = value;
    return result;
}


template<>
inline ArrayPtr Array::createScalar(mxLogical value) {
    return ArrayPtr(throw_if_null, mxCreateLogicalScalar(value));
}


template<>
inline ArrayPtr Array::createScalar(mxDouble value) {
    return ArrayPtr(throw_if_null, mxCreateDoubleScalar(value));
}


template<typename T>
inline ArrayPtr Array::createVector(const T *values, std::size_t size) {
    if (!size) {
        return createEmpty<T>();
    }
    mxAssert(values, "values is NULL");
    auto result = createVector<T>(size);
    std::copy(values, values+size, getData<T>(result));
    return result;
}


template<typename T>
inline ArrayPtr Array::createVector(const std::vector<T> &values) {
    // std::vector<bool> is specialized and doesn't necessarily store its data as a contiguous
    // array of bool's, so we can't treat it like other types
    static_assert(!std::is_same_v<T, bool>);
    return createVector(values.data(), values.size());
}


inline ArrayPtr Array::createString(const std::string &str) {
    return ArrayPtr(throw_if_null, mxCreateString(str.c_str()));
}


inline ArrayPtr Array::createCell(std::vector<ArrayPtr> &&values) {
    auto result = ArrayPtr(throw_if_null, mxCreateCellMatrix((values.empty() ? 0 : 1), values.size()));
    for (std::size_t i = 0; i < values.size(); i++) {
        mxSetCell(result.get(), i, values[i].release());
    }
    values.clear();
    return result;
}


inline ArrayPtr Array::createStruct(mwSize size, const FieldNames &fieldNames) {
    return ArrayPtr(throw_if_null, mxCreateStructMatrix((size ? 1 : 0),
                                                        size,
                                                        int(fieldNames.size()),
                                                        const_cast<const char **>(fieldNames.data())));
}


inline ArrayPtr Array::createStruct(const FieldNames &fieldNames, FieldValues &&fieldValues) {
    if (fieldValues.empty()) {
        // If there are field names but not field values, create an empty (0x0) struct array.  If there are
        // neither field names nor field values, create a scalar (1x1) struct with no fields.
        return createStruct((fieldNames.empty() ? 1 : 0), fieldNames);
    }
    mxAssert(fieldNames.size() == fieldValues.size(), "fieldNames and fieldValues have different sizes");
    auto result = createStruct(1, fieldNames);
    for (std::size_t i = 0; i < fieldValues.size(); i++) {
        mxSetFieldByNumber(result.get(), 0, int(i), fieldValues[i].release());
    }
    fieldValues.clear();
    return result;
}


template<typename... Args>
inline ArrayPtr Array::createStruct(const FieldNames &fieldNames, Args&&... args) {
    return createStruct(fieldNames, FieldValues{}, std::forward<Args>(args)...);
}


template<typename... Args>
inline ArrayPtr Array::createStruct(const FieldNames &fieldNames,
                                    FieldValues &&fieldValues,
                                    ArrayPtr &&array,
                                    Args&&... args)
{
    if (array) {
        fieldValues.emplace_back(std::move(array));
    }
    return createStruct(fieldNames, std::move(fieldValues), std::forward<Args>(args)...);
}


END_NAMESPACE_MW_MATLAB


#endif  // !defined(MATLABTools_Array_h)
