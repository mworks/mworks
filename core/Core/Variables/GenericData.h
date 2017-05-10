/**
 * GenericData.h
 *
 * Copyright (c) 2005 MIT. All rights reserved.
 */

#ifndef _GENERIC_DATA_H
#define _GENERIC_DATA_H

#include "MWorksMacros.h"
#include "MWorksTypes.h"
#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include <boost/serialization/string.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdivision-by-zero"
#include <boost/archive/binary_oarchive.hpp>
#pragma clang diagnostic pop
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/basic_binary_oprimitive.hpp>
#include <boost/archive/basic_binary_iprimitive.hpp>
#include "Serialization.h"

using std::map;
using std::pair;
using std::string;
using std::vector;


BEGIN_NAMESPACE_MW

    
enum GenericDataType {
    M_UNDEFINED = 0,
    M_INTEGER,
    M_FLOAT,
    M_BOOLEAN,
    M_STRING,
    M_LIST,
    M_DICTIONARY,
    M_MAX_DATA_TYPE
};


class Datum {
    
public:
    using list_value_type = std::vector<Datum>;
    
    struct Hasher {
        std::size_t operator()(const Datum &d) const { return d.getHash(); }
    };
    using dict_value_type = std::unordered_map<Datum, Datum, Hasher>;
    
    /**
     * Destructor
     */
    ~Datum();
    
    /**
     * Copy and move constructors
     */
    Datum(const Datum &other);
    Datum(Datum &&other);
    
    /**
     * Copy and move assignment operators
     */
    Datum& operator=(const Datum &other);
    Datum& operator=(Datum &&other);
    
    /**
     * Constructors
     */
    
    constexpr Datum() : datatype(M_UNDEFINED), intValue(0) { }
    constexpr Datum(int value) : datatype(M_INTEGER), intValue(value) { }
    constexpr Datum(long value) : datatype(M_INTEGER), intValue(value) { }
    constexpr Datum(long long value) : datatype(M_INTEGER), intValue(value) { }
    constexpr Datum(float value) : datatype(M_FLOAT), floatValue(value) { }
    constexpr Datum(double value) : datatype(M_FLOAT), floatValue(value) { }
    constexpr Datum(bool value) : datatype(M_BOOLEAN), boolValue(value) { }
    
    Datum(const char * string, int size);
    Datum(const char * string);
    Datum(char * string) : Datum(static_cast<const char *>(string)) { }
    Datum(const std::string &string);
    explicit Datum(std::string &&string);
    
    explicit Datum(const list_value_type &list);
    explicit Datum(list_value_type &&list);
    explicit Datum(const dict_value_type &dict);
    explicit Datum(dict_value_type &&dict);
    
    Datum(GenericDataType type, int arg);
    Datum(GenericDataType type, double arg);
    
    template<typename T>
    explicit Datum(const std::vector<T> &list) {
        *this = Datum(list_value_type());
        for (auto &item : list) {
            listValue.emplace_back(item);
        }
    }
    
    template<typename T1, typename T2>
    Datum(const std::map<T1, T2> &dict) {
        *this = Datum(dict_value_type());
        for (auto &item : dict) {
            dictValue[Datum(item.first)] = Datum(item.second);
        }
    }
    
    // Prevent conversion of pointer types other than char* to bool
    template<typename T>
    Datum(T *value) {
        static_assert(!std::is_same<T, T>::value, "Illegal conversion from pointer to Datum");
    }
    
    /**
     * Type info
     */
    
    GenericDataType getDataType() const { return datatype; }
    const char * getDataTypeName() const;
    
    bool isInteger() const { return ((datatype == M_INTEGER) || isBool()); }
    bool isFloat() const { return (datatype == M_FLOAT); }
    bool isBool() const { return (datatype == M_BOOLEAN); }
    bool isNumber() const { return (isFloat() || isInteger()); }
    bool isString() const { return (datatype == M_STRING); }
    bool isList() const { return (datatype == M_LIST); }
    bool isDictionary() const { return (datatype == M_DICTIONARY); }
    bool isUndefined() const { return (datatype == M_UNDEFINED); }
    
    /**
     * Returns a hash value for the Datum object
     */
    std::size_t getHash() const;
    
    /**
     * Value getters
     */
    
    long long getInteger() const;
    double getFloat() const;
    bool getBool() const;
    
    const std::string& getString() const;
    bool stringIsCString() const;
    std::string getStringQuoted() const;
    
    const list_value_type& getList() const;
    const dict_value_type& getDict() const;
    
    std::size_t getSize() const;
    
    int getNElements() const;
    bool hasKey(const Datum &key) const;
    const Datum& getElement(const Datum &indexOrKey) const;
    const Datum& getElement(const std::vector<Datum> &indexOrKeyPath) const;
    
    /**
     * Value setters
     */
    
    void setInteger(long long value);
    void setFloat(double value);
    void setBool(bool value);
    
    void setString(const char * string, int size);
    void setString(const char * string);
    void setString(const std::string &string);
    void setString(std::string &&string);
    void setStringQuoted(const std::string &string);
    
    void setList(const list_value_type &list);
    void setList(list_value_type &&list);
    void setDict(const dict_value_type &dict);
    void setDict(dict_value_type &&dict);
    
    void addElement(const Datum &value);
    void addElement(const Datum &key, const Datum &value);
    void setElement(const Datum &indexOrKey, const Datum &value);
    void setElement(const std::vector<Datum> &indexOrKeyPath, const Datum &value);
    
    /**
     * Operators
     */
    
    operator short() const;
    operator int() const;
    operator long() const;
    operator long long() const;
    operator float() const;
    operator double() const;
    operator bool() const;
    operator std::string() const;
    
    void operator++();
    void operator--();
    
    Datum operator-() const;
    
    Datum operator+(const Datum&) const;
    Datum operator-(const Datum&) const;
    Datum operator*(const Datum&) const;
    Datum operator/(const Datum&) const;
    Datum operator%(const Datum&) const;
    
    bool operator==(const Datum&) const;
    bool operator!=(const Datum&) const;
    bool operator<(const Datum&) const;
    bool operator<=(const Datum&) const;
    bool operator>(const Datum&) const;
    bool operator>=(const Datum&) const;
    
    const Datum& operator[](const Datum &indexOrKey) const;
    
    std::string toString(bool quoted = false) const;
    
private:
    GenericDataType datatype;
    
    union {
        long long intValue;
        double floatValue;
        bool boolValue;
        std::string stringValue;
        list_value_type listValue;
        dict_value_type dictValue;
    };
    
    void copyFrom(const Datum &other, bool assign = false);
    void moveFrom(Datum &&other, bool assign = false);
    
    const Datum * getReadableElementPtr(const Datum &indexOrKey) const;
    Datum * getWritableElementPtr(const Datum &indexOrKey);
    
    template<template<typename T> class Op>
    bool compareOrdering(const Datum&) const;
    
    friend class boost::serialization::access;
    
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version){
        boost::serialization::split_member(ar, *this, version);
    }
    
    template<class Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar << datatype;
        
        switch (datatype) {
            case M_INTEGER:
                ar << intValue;
                break;
                
            case M_FLOAT:
                ar << floatValue;
                break;
                
            case M_BOOLEAN:
                ar << boolValue;
                break;
                
            case M_STRING: {
                int string_length = stringValue.size();
                ar << string_length;
                ar.save_binary(stringValue.c_str(), string_length);
                break;
            }
                
            case M_LIST:
                ar << listValue;
                break;
                
            case M_DICTIONARY:
                ar << dictValue;
                break;
                
            default:
                break;
        }
    }
    
    template<class Archive>
    void load(Archive &ar, const unsigned int version) {
        GenericDataType archived_datatype;
        ar >> archived_datatype;
        
        switch (archived_datatype) {
            case M_INTEGER: {
                long long ival;
                ar >> ival;
                *this = Datum(ival);
                break;
            }
                
            case M_FLOAT: {
                double fval;
                ar >> fval;
                *this = Datum(fval);
                break;
            }
                
            case M_BOOLEAN: {
                bool bval;
                ar >> bval;
                *this = Datum(bval);
                break;
            }
                
            case M_STRING: {
                int string_length;
                ar >> string_length;
                if (version > 0) {
                    std::vector<char> buffer(string_length);
                    ar.load_binary(buffer.data(), buffer.size());
                    setString(buffer.data(), buffer.size());
                } else {
                    std::string sval;
                    ar >> sval;
                    setString(sval);
                }
                break;
            }
                
            case M_LIST:
                if (version > 1) {
                    list_value_type lval;
                    ar >> lval;
                    *this = Datum(std::move(lval));
                } else {
                    *this = Datum(list_value_type());
                    int nelements;
                    ar >> nelements;
                    for (int i = 0; i < nelements; i++) {
                        Datum datum;
                        ar >> datum;
                        addElement(datum);
                    }
                }
                break;
                
            case M_DICTIONARY:
                if (version > 1) {
                    dict_value_type dval;
                    ar >> dval;
                    *this = Datum(std::move(dval));
                } else {
                    *this = Datum(dict_value_type());
                    int nelements;
                    ar >> nelements;
                    for (int i = 0; i < nelements; i++) {
                        Datum datum, key;
                        ar >> key;
                        ar >> datum;
                        addElement(key, datum);
                    }
                }
                break;
                
            default:
                break;
        }
    }
};


std::ostream& operator<<(std::ostream &stream, const Datum &d);


END_NAMESPACE_MW


// This needs to be outside of namespace mw
BOOST_CLASS_VERSION(mw::Datum, 2)


#endif



























