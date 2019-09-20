//
//  Converters.cpp
//  MATLABTools
//
//  Created by Christopher Stawarz on 12/14/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#include "Converters.h"

#include "Array.h"


BEGIN_NAMESPACE_MW_MATLAB


static inline bool isAlphanumericOrUnderscore(int ch) {
    return (ch == '_') || std::isalnum(ch);
}


static bool isValidStructFieldName(const Datum::dict_value_type::value_type &item) {
    auto &key = item.first;
    
    if (!(key.isString() && key.stringIsCString()))
        return false;
    
    auto &str = key.getString();
    
    return (!str.empty() &&
            std::isalpha(str.at(0)) &&
            std::all_of(str.begin()+1, str.end(), isAlphanumericOrUnderscore));
}


static ArrayPtr convertDictionaryToStruct(const Datum &datum) {
    std::vector<const char *> fieldNames;
    std::vector<ArrayPtr> fieldValues;
    
    for (auto &item : datum.getDict()) {
        fieldNames.push_back(item.first.getString().c_str());
        fieldValues.emplace_back(convertDatumToArray(item.second));
    }
    
    ArrayPtr result(throw_if_null, mxCreateStructMatrix(1, 1, int(fieldNames.size()), fieldNames.data()));
    
    for (std::size_t i = 0; i < fieldNames.size(); i++) {
        mxSetFieldByNumber(result.get(), 0, int(i), fieldValues[i].release());
    }
    
    return result;
}


static ArrayPtr convertDictionaryToMap(const Datum &datum) {
    std::vector<ArrayPtr> keys;
    std::vector<ArrayPtr> values;
    
    for (auto &item : datum.getDict()) {
        keys.emplace_back(convertDatumToArray(item.first));
        values.emplace_back(convertDatumToArray(item.second));
    }
    
    ArrayPtr keySet = Array::createVector(std::move(keys));
    ArrayPtr valueSet = Array::createVector(std::move(values));
    ArrayPtr result;
    
    mxArray *lhs = nullptr;
    mxArray *prhs[] = { keySet.get(), valueSet.get() };
    ArrayPtr error(null_ok, mexCallMATLABWithTrap(1, &lhs, 2, prhs, "containers.Map"));
    if (!error) {
        result = ArrayPtr(throw_if_null, lhs);
        keySet.destroy();
        valueSet.destroy();
    } else {
        error.destroy();
        const char *fieldNames[] = {"keys", "values"};
        result = ArrayPtr(throw_if_null, mxCreateStructMatrix(1, 1, 2, fieldNames));
        mxSetFieldByNumber(result.get(), 0, 0, keySet.release());
        mxSetFieldByNumber(result.get(), 0, 1, valueSet.release());
    }
    
    return result;
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
            auto &str = datum.getString();
            if (datum.stringIsCString())
                return Array::createString(str.c_str());
            return Array::createVector(reinterpret_cast<const unsigned char *>(str.data()), str.size());
        }
            
        case M_LIST: {
            std::vector<ArrayPtr> items;
            for (auto &item : datum.getList()) {
                items.emplace_back(convertDatumToArray(item));
            }
            return Array::createVector(std::move(items));
        }
            
        case M_DICTIONARY: {
            auto &dict = datum.getDict();
            if (std::all_of(dict.begin(), dict.end(), isValidStructFieldName))
                return convertDictionaryToStruct(datum);
            return convertDictionaryToMap(datum);
        }
            
        default:
            throwMATLABError("MWorks:DataConversionError",
                             boost::format("Cannot convert Datum of unknown type (%d)") % datum.getDataType());
    }
}


END_NAMESPACE_MW_MATLAB
