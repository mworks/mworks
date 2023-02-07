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


static bool isValidStructFieldName(const Datum::dict_value_type::value_type &item) {
    auto &key = item.first;
    
    if (!(key.isString() && key.stringIsCString()))
        return false;
    
    auto &str = key.getString();
    
    return (!str.empty() &&
            std::isalpha(str.front()) &&
            std::all_of(std::next(str.begin()),
                        str.end(),
                        [](int ch) { return (ch == '_') || std::isalnum(ch); }));
}


static ArrayPtr convertDictionaryToStruct(const Datum::dict_value_type &dict) {
    std::vector<const char *> fieldNames;
    std::vector<ArrayPtr> fieldValues;
    
    for (auto &item : dict) {
        fieldNames.push_back(item.first.getString().c_str());
        fieldValues.emplace_back(convertDatumToArray(item.second));
    }
    
    return Array::createStruct(fieldNames, std::move(fieldValues));
}


static ArrayPtr convertDictionaryToMap(const Datum::dict_value_type &dict) {
    std::vector<ArrayPtr> keys;
    std::vector<ArrayPtr> values;
    
    for (auto &item : dict) {
        keys.emplace_back(convertDatumToArray(item.first));
        values.emplace_back(convertDatumToArray(item.second));
    }
    
    auto keySet = Array::createCell(std::move(keys));
    auto valueSet = Array::createCell(std::move(values));
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
        result = Array::createStruct({ "keys", "values" }, std::move(keySet), std::move(valueSet));
    }
    
    return result;
}


ArrayPtr convertDatumToArray(const Datum &datum) {
    switch (datum.getDataType()) {
        case M_UNDEFINED:
            return Array::createEmpty<double>();
            
        case M_INTEGER:
            return Array::createScalar(datum.getInteger());
            
        case M_FLOAT:
            return Array::createScalar(datum.getFloat());
            
        case M_BOOLEAN:
            return Array::createScalar(datum.getBool());
            
        case M_STRING: {
            auto &str = datum.getString();
            if (datum.stringIsCString())
                return Array::createString(str);
            return Array::createVector(reinterpret_cast<const unsigned char *>(str.data()), str.size());
        }
            
        case M_LIST: {
            std::vector<ArrayPtr> items;
            for (auto &item : datum.getList()) {
                items.emplace_back(convertDatumToArray(item));
            }
            return Array::createCell(std::move(items));
        }
            
        case M_DICTIONARY: {
            auto &dict = datum.getDict();
            if (std::all_of(dict.begin(), dict.end(), isValidStructFieldName))
                return convertDictionaryToStruct(dict);
            return convertDictionaryToMap(dict);
        }
            
        default:
            throwMATLABError("MWorks:DataConversionError",
                             boost::format("Cannot convert Datum of unknown type (%d)") % datum.getDataType());
    }
}


END_NAMESPACE_MW_MATLAB
