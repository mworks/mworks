//
//  Converters.cpp
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/14/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#include "Converters.h"

#include <cctype>

#include <boost/algorithm/cxx11/all_of.hpp>
#include <boost/foreach.hpp>

#include "Array.h"


BEGIN_NAMESPACE_MW_MATLAB


static inline bool isAlphanumericOrUnderscore(int ch) {
    return (ch == '_') || std::isalnum(ch);
}


static bool isValidStructFieldName(const Datum &key) {
    if (!(key.isString() && key.stringIsCString()))
        return false;
    
    const char *data = key.getString();
    int size = key.getStringLength() - 1;  // Exclude NUL terminator from size
    
    return ((size > 0) &&
            std::isalpha(data[0]) &&
            boost::algorithm::all_of(data+1, data+size, isAlphanumericOrUnderscore));
}


static ArrayPtr convertDictionaryToStruct(const Datum &datum, const std::vector<Datum> &keys) {
    std::vector<const char *> fieldNames;
    boost::container::vector<ArrayPtr> fieldValues;
    
    BOOST_FOREACH(const Datum &k, keys) {
        fieldNames.push_back(k.getString());
        fieldValues.push_back(convertDatumToArray(datum.getElement(k)));
    }
    
    ArrayPtr result(throw_if_null, mxCreateStructMatrix(1, 1, int(fieldNames.size()), &(fieldNames.front())));
    
    for (std::size_t i = 0; i < fieldNames.size(); i++) {
        mxSetFieldByNumber(result.get(), 0, int(i), fieldValues[i].release());
    }
    
    return boost::move(result);
}


static ArrayPtr convertDictionaryToMap(const Datum &datum, const std::vector<Datum> &datumKeys) {
    boost::container::vector<ArrayPtr> keys;
    boost::container::vector<ArrayPtr> values;
    
    BOOST_FOREACH(const Datum &k, datumKeys) {
        keys.push_back(convertDatumToArray(k));
        values.push_back(convertDatumToArray(datum.getElement(k)));
    }
    
    ArrayPtr keySet = Array::createVector(keys);
    ArrayPtr valueSet = Array::createVector(values);
    ArrayPtr result;
    
#ifdef MATLAB_MEX_FILE
    mxArray *lhs = NULL;
    mxArray *prhs[] = { keySet.get(), valueSet.get() };
    ArrayPtr error(null_ok, mexCallMATLABWithTrap(1, &lhs, 2, prhs, "containers.Map"));
    if (error.isNull()) {
        result = ArrayPtr(throw_if_null, lhs);
        keySet.destroy();
        valueSet.destroy();
    } else {
        error.destroy();
#endif  // defined(MATLAB_MEX_FILE)
        const char *fieldNames[] = {"keys", "values"};
        result = ArrayPtr(throw_if_null, mxCreateStructMatrix(1, 1, 2, fieldNames));
        mxSetFieldByNumber(result.get(), 0, 0, keySet.release());
        mxSetFieldByNumber(result.get(), 0, 1, valueSet.release());
#ifdef MATLAB_MEX_FILE
    }
#endif
    
    return boost::move(result);
}


ArrayPtr convertDatumToArray(const Datum &datum) {
    if (datum.isUndefined()) {
        return Array::createEmpty<double>();
    }
    
    switch (datum.getDataType()) {
        case M_INTEGER:
            return Array::createScalar(datum.getInteger());
            
        case M_FLOAT:
            return Array::createScalar(datum.getFloat());
            
        case M_BOOLEAN:
            return Array::createScalar(bool(datum.getBool()));
            
        case M_STRING: {
            const char *data = datum.getString();
            if (datum.stringIsCString())
                return Array::createString(data);
            return Array::createVector(reinterpret_cast<const unsigned char *>(data), datum.getStringLength());
        }
            
        case M_LIST: {
            int size = datum.getNElements();
            boost::container::vector<ArrayPtr> items;
            for (int i = 0; i < size; i++) {
                items.push_back(convertDatumToArray(datum.getElement(i)));
            }
            return Array::createVector(items);
        }
            
        case M_DICTIONARY: {
            std::vector<Datum> keys = datum.getKeys();
            if (boost::algorithm::all_of(keys.begin(), keys.end(), isValidStructFieldName))
                return convertDictionaryToStruct(datum, keys);
            return convertDictionaryToMap(datum, keys);
        }
            
        default:
            throwMATLABError("MWorks:DataConversionError",
                             boost::format("Cannot convert Datum of unknown type (%d)") % datum.getDataType());
            return ArrayPtr();  // Never reached
    }
}


END_NAMESPACE_MW_MATLAB


























