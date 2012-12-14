//
//  TypeInfo.h
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/10/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef MATLABTools_TypeInfo_h
#define MATLABTools_TypeInfo_h

#include "Utilities.h"


BEGIN_NAMESPACE_MW_MATLAB


template<typename T>
struct TypeInfo {
    // No generic implementation.  This allows the compiler to easily catch attempts
    // to use unsupported types.
};


template<>
struct TypeInfo<mxLogical> {
    static const mxClassID class_id = mxLOGICAL_CLASS;
    static const bool is_numeric = false;
    static const char* getClassName() { return "logical"; }
};


template<>
struct TypeInfo<int8_T> {
    static const mxClassID class_id = mxINT8_CLASS;
    static const bool is_numeric = true;
    static const char* getClassName() { return "int8"; }
};


template<>
struct TypeInfo<uint8_T> {
    static const mxClassID class_id = mxUINT8_CLASS;
    static const bool is_numeric = true;
    static const char* getClassName() { return "uint8"; }
};


template<>
struct TypeInfo<int16_T> {
    static const mxClassID class_id = mxINT16_CLASS;
    static const bool is_numeric = true;
    static const char* getClassName() { return "int16"; }
};


template<>
struct TypeInfo<uint16_T> {
    static const mxClassID class_id = mxUINT16_CLASS;
    static const bool is_numeric = true;
    static const char* getClassName() { return "uint16"; }
};


template<>
struct TypeInfo<int32_T> {
    static const mxClassID class_id = mxINT32_CLASS;
    static const bool is_numeric = true;
    static const char* getClassName() { return "int32"; }
};


template<>
struct TypeInfo<uint32_T> {
    static const mxClassID class_id = mxUINT32_CLASS;
    static const bool is_numeric = true;
    static const char* getClassName() { return "uint32"; }
};


template<>
struct TypeInfo<int64_T> {
    static const mxClassID class_id = mxINT64_CLASS;
    static const bool is_numeric = true;
    static const char* getClassName() { return "int64"; }
};


template<>
struct TypeInfo<uint64_T> {
    static const mxClassID class_id = mxUINT64_CLASS;
    static const bool is_numeric = true;
    static const char* getClassName() { return "uint64"; }
};


template<>
struct TypeInfo<float> {
    static const mxClassID class_id = mxSINGLE_CLASS;
    static const bool is_numeric = true;
    static const char* getClassName() { return "single"; }
};


template<>
struct TypeInfo<double> {
    static const mxClassID class_id = mxDOUBLE_CLASS;
    static const bool is_numeric = true;
    static const char* getClassName() { return "double"; }
};


template<>
struct TypeInfo<ArrayPtr> {
    static const mxClassID class_id = mxCELL_CLASS;
    static const bool is_numeric = false;
    static const char* getClassName() { return "cell"; }
};


END_NAMESPACE_MW_MATLAB


#endif  // !defined(MATLABTools_TypeInfo_h)


























