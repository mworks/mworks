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
    static constexpr mxClassID classID = mxLOGICAL_CLASS;
    static constexpr bool isNumeric = false;
    static constexpr const char * getClassName() { return "logical"; }
    static mxLogical * getData(const mxArray *array) { return mxGetLogicals(array); };
};


template<>
struct TypeInfo<mxInt8> {
    static constexpr mxClassID classID = mxINT8_CLASS;
    static constexpr bool isNumeric = true;
    static constexpr const char * getClassName() { return "int8"; }
    static mxInt8 * getData(const mxArray *array) { return mxGetInt8s(array); };
};


template<>
struct TypeInfo<mxUint8> {
    static constexpr mxClassID classID = mxUINT8_CLASS;
    static constexpr bool isNumeric = true;
    static constexpr const char * getClassName() { return "uint8"; }
    static mxUint8 * getData(const mxArray *array) { return mxGetUint8s(array); };
};


template<>
struct TypeInfo<mxInt16> {
    static constexpr mxClassID classID = mxINT16_CLASS;
    static constexpr bool isNumeric = true;
    static constexpr const char * getClassName() { return "int16"; }
    static mxInt16 * getData(const mxArray *array) { return mxGetInt16s(array); };
};


template<>
struct TypeInfo<mxUint16> {
    static constexpr mxClassID classID = mxUINT16_CLASS;
    static constexpr bool isNumeric = true;
    static constexpr const char * getClassName() { return "uint16"; }
    static mxUint16 * getData(const mxArray *array) { return mxGetUint16s(array); };
};


template<>
struct TypeInfo<mxInt32> {
    static constexpr mxClassID classID = mxINT32_CLASS;
    static constexpr bool isNumeric = true;
    static constexpr const char * getClassName() { return "int32"; }
    static mxInt32 * getData(const mxArray *array) { return mxGetInt32s(array); };
};


template<>
struct TypeInfo<mxUint32> {
    static constexpr mxClassID classID = mxUINT32_CLASS;
    static constexpr bool isNumeric = true;
    static constexpr const char * getClassName() { return "uint32"; }
    static mxUint32 * getData(const mxArray *array) { return mxGetUint32s(array); };
};


template<>
struct TypeInfo<mxInt64> {
    static constexpr mxClassID classID = mxINT64_CLASS;
    static constexpr bool isNumeric = true;
    static constexpr const char * getClassName() { return "int64"; }
    static mxInt64 * getData(const mxArray *array) { return mxGetInt64s(array); };
};


template<>
struct TypeInfo<mxUint64> {
    static constexpr mxClassID classID = mxUINT64_CLASS;
    static constexpr bool isNumeric = true;
    static constexpr const char * getClassName() { return "uint64"; }
    static mxUint64 * getData(const mxArray *array) { return mxGetUint64s(array); };
};


template<>
struct TypeInfo<mxSingle> {
    static constexpr mxClassID classID = mxSINGLE_CLASS;
    static constexpr bool isNumeric = true;
    static constexpr const char * getClassName() { return "single"; }
    static mxSingle * getData(const mxArray *array) { return mxGetSingles(array); };
};


template<>
struct TypeInfo<mxDouble> {
    static constexpr mxClassID classID = mxDOUBLE_CLASS;
    static constexpr bool isNumeric = true;
    static constexpr const char * getClassName() { return "double"; }
    static mxDouble * getData(const mxArray *array) { return mxGetDoubles(array); };
};


END_NAMESPACE_MW_MATLAB


#endif  // !defined(MATLABTools_TypeInfo_h)
