/**
 * GenericData.cpp
 *
 * Copyright (c) 2005 MIT. All rights reserved.
 */

#include "GenericData.h"
#include "Utilities.h"

#include <sstream>

#include <boost/functional/hash.hpp>


BEGIN_NAMESPACE_MW


Datum::~Datum() {
    switch (datatype) {
        case M_STRING:
            stringValue.~string();
            break;
            
        case M_LIST:
            listValue.~list_value_type();
            break;
            
        case M_DICTIONARY:
            dictValue.~dict_value_type();
            break;
            
        default:
            // No destructor to call for other types
            break;
    }
}


Datum::Datum(const Datum &other) :
    datatype(other.datatype),
    compressible(other.compressible)
{
    copyFrom(other);
}


Datum::Datum(Datum &&other) :
    datatype(other.datatype),
    compressible(other.compressible)
{
    moveFrom(std::move(other));
}


Datum& Datum::operator=(const Datum &other) {
    if (this == &other) {
        // Self assignment -- do nothing
    } else {
        // If our current value is a container type (list, dict), and other is a reference to a
        // contained element (e.g. returned by getElement()), then we need to ensure that other
        // exists long enough for us to copy its value.  To do this, we move our current value
        // to a temporary Datum before replacing it with other's value.
        const Datum oldThis(std::move(*this));
        
        if (datatype == other.datatype) {
            copyFrom(other, true);
        } else {
            this->~Datum();
            datatype = other.datatype;
            copyFrom(other);
        }
        
        compressible = other.compressible;
    }
    
    return (*this);
}


Datum& Datum::operator=(Datum &&other) {
    if (this == &other) {
        // Self assignment -- do nothing
    } else {
        // If our current value is a container type (list, dict), and other is a reference to a
        // contained element (e.g. returned by getElement()), then we need to ensure that other
        // exists long enough for us to move its value.  To do this, we move our current value
        // to a temporary Datum before replacing it with other's value.
        const Datum oldThis(std::move(*this));
        
        if (datatype == other.datatype) {
            moveFrom(std::move(other), true);
        } else {
            this->~Datum();
            datatype = other.datatype;
            moveFrom(std::move(other));
        }
        
        compressible = other.compressible;
    }
    
    return (*this);
}


void Datum::copyFrom(const Datum &other, bool assign) {
    switch (other.datatype) {
        case M_INTEGER:
            intValue = other.intValue;
            break;
            
        case M_FLOAT:
            floatValue = other.floatValue;
            break;
            
        case M_BOOLEAN:
            boolValue = other.boolValue;
            break;
            
        case M_STRING:
            if (assign) {
                stringValue = other.stringValue;
            } else {
                new(&stringValue) std::string(other.stringValue);
            }
            break;
            
        case M_LIST:
            if (assign) {
                listValue = other.listValue;
            } else {
                new(&listValue) list_value_type(other.listValue);
            }
            break;
            
        case M_DICTIONARY:
            if (assign) {
                dictValue = other.dictValue;
            } else {
                new(&dictValue) dict_value_type(other.dictValue);
            }
            break;
            
        default:
            break;
    }
}


void Datum::moveFrom(Datum &&other, bool assign) {
    switch (other.datatype) {
        case M_INTEGER:
            intValue = other.intValue;
            break;
            
        case M_FLOAT:
            floatValue = other.floatValue;
            break;
            
        case M_BOOLEAN:
            boolValue = other.boolValue;
            break;
            
        case M_STRING:
            if (assign) {
                stringValue = std::move(other.stringValue);
            } else {
                new(&stringValue) std::string(std::move(other.stringValue));
            }
            break;
            
        case M_LIST:
            if (assign) {
                listValue = std::move(other.listValue);
            } else {
                new(&listValue) list_value_type(std::move(other.listValue));
            }
            break;
            
        case M_DICTIONARY:
            if (assign) {
                dictValue = std::move(other.dictValue);
            } else {
                new(&dictValue) dict_value_type(std::move(other.dictValue));
            }
            break;
            
        default:
            break;
    }
}


Datum::Datum(const char *string, int size) :
    Datum(std::string(string, size))
{ }


Datum::Datum(const char *string) :
    Datum(std::string(string))
{ }


Datum::Datum(const std::string &string) :
    datatype(M_STRING),
    stringValue(string)
{ }


Datum::Datum(std::string &&string) :
    datatype(M_STRING),
    stringValue(std::move(string))
{ }


Datum::Datum(const list_value_type &list) :
    datatype(M_LIST),
    listValue(list)
{ }


Datum::Datum(list_value_type &&list) :
    datatype(M_LIST),
    listValue(std::move(list))
{ }


Datum::Datum(const dict_value_type &dict) :
    datatype(M_DICTIONARY),
    dictValue(dict)
{ }


Datum::Datum(dict_value_type &&dict) :
    datatype(M_DICTIONARY),
    dictValue(std::move(dict))
{ }


Datum::Datum(GenericDataType type, int arg) :
    Datum()
{
    switch (type) {
        case M_INTEGER:
            *this = Datum(arg);
            break;
        case M_FLOAT:
            *this = Datum(float(arg));
            break;
        case M_BOOLEAN:
            *this = Datum(bool(arg));
            break;
        case M_STRING:
            *this = Datum(std::string());
            break;
        case M_LIST:
            *this = Datum(list_value_type());
            if (arg >= 1) {
                listValue.reserve(arg);
            }
            break;
        case M_DICTIONARY:
            *this = Datum(dict_value_type());
            if (arg >= 1) {
                dictValue.reserve(arg);
            }
            break;
        default:
            break;
    }
}


Datum::Datum(GenericDataType type, double arg) :
    Datum()
{
    switch (type) {
        case M_INTEGER:
            *this = Datum(int(arg));
            break;
        case M_FLOAT:
            *this = Datum(arg);
            break;
        case M_BOOLEAN:
            *this = Datum(bool(arg));
            break;
        case M_STRING:
            *this = Datum(std::string());
            break;
        case M_LIST:
            *this = Datum(list_value_type());
            if (arg >= 1.0) {
                listValue.reserve(arg);
            }
            break;
        case M_DICTIONARY:
            *this = Datum(dict_value_type());
            if (arg >= 1.0) {
                dictValue.reserve(arg);
            }
            break;
        default:
            break;
    }
}


const char * Datum::getDataTypeName(GenericDataType datatype) {
    switch (datatype) {
        case M_INTEGER:
            return "integer";
        case M_FLOAT:
            return "float";
        case M_BOOLEAN:
            return "boolean";
        case M_STRING:
            return "string";
        case M_LIST:
            return "list";
        case M_DICTIONARY:
            return "dictionary";
        default:
            return "undefined";
    }
}


static inline std::size_t hash_value(const Datum &d) {
    return d.getHash();
}


std::size_t Datum::getHash() const {
    std::size_t seed = 0;
    boost::hash_combine(seed, int(datatype));
    
    switch (datatype) {
        case M_INTEGER:
            boost::hash_combine(seed, intValue);
            break;
            
        case M_FLOAT:
            boost::hash_combine(seed, floatValue);
            break;
            
        case M_BOOLEAN:
            boost::hash_combine(seed, boolValue);
            break;
            
        case M_STRING:
            boost::hash_combine(seed, stringValue);
            break;
            
        case M_LIST:
            boost::hash_range(seed, listValue.begin(), listValue.end());
            break;
            
        case M_DICTIONARY:
            boost::hash_unordered_range(seed, dictValue.begin(), dictValue.end());
            break;
            
        default:
            break;
    }
    
    return seed;
}


long long Datum::getInteger() const {
    switch (datatype) {
        case M_INTEGER:
            return intValue;
        case M_FLOAT:
            return static_cast<long long>(floatValue);
        case M_BOOLEAN:
            return static_cast<long long>(boolValue);
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to integer", getDataTypeName());
            return 0;
    }
}


double Datum::getFloat() const {
    switch (datatype) {
        case M_INTEGER:
            return double(intValue);
        case M_FLOAT:
            return floatValue;
        case M_BOOLEAN:
            return double(boolValue);
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to float", getDataTypeName());
            return 0.0;
    }
}


bool Datum::getBool() const {
    switch (datatype) {
        case M_INTEGER:
            return bool(intValue);
        case M_FLOAT:
            return bool(floatValue);
        case M_BOOLEAN:
            return boolValue;
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to boolean", getDataTypeName());
            return false;
    }
}


const std::string& Datum::getString() const {
    switch (datatype) {
        case M_STRING:
            return stringValue;
        default: {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to string", getDataTypeName());
            static const std::string empty;
            return empty;
        }
    }
}


bool Datum::stringIsCString() const {
    switch (datatype) {
        case M_STRING:
            // stringValue is a C string if it contains no embedded NUL's
            return (stringValue.find('\0') == std::string::npos);
        default:
            return false;
    }
}


std::string Datum::getStringQuoted() const {
    if (!isString()) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to string", getDataTypeName());
        return "";
    }
    
    std::string os = "\"";
    os.reserve(2 + stringValue.size() + stringValue.size() / 16);
    
    for (char c : stringValue) {
        switch (c) {
            case '\a':
                os += "\\a";
                break;
                
            case '\b':
                os += "\\b";
                break;
                
            case '\f':
                os += "\\f";
                break;
                
            case '\n':
                os += "\\n";
                break;
                
            case '\r':
                os += "\\r";
                break;
                
            case '\t':
                os += "\\t";
                break;
                
            case '\v':
                os += "\\v";
                break;
                
            case '\\':
                os += "\\\\";
                break;
                
            case '"':
                os += "\\\"";
                break;
                
            case '\'':
                os += "\\'";
                break;
                
            case '$':
                os += "\\$";
                break;
                
            default:
                os += c;
                break;
        }
    }
    
    os += "\"";
    return os;
}


auto Datum::getList() const -> const list_value_type& {
    switch (datatype) {
        case M_LIST:
            return listValue;
        default: {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to list", getDataTypeName());
            static const list_value_type empty;
            return empty;
        }
    }
}


auto Datum::getDict() const -> const dict_value_type& {
    switch (datatype) {
        case M_DICTIONARY:
            return dictValue;
        default: {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to dictionary", getDataTypeName());
            static const dict_value_type empty;
            return empty;
        }
    }
}


std::size_t Datum::getSize() const {
    switch (datatype) {
        case M_STRING:
            return stringValue.size();
            
        case M_LIST:
            return listValue.size();
            
        case M_DICTIONARY:
            return dictValue.size();
            
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot get size of %s", getDataTypeName());
            return 0;
    }
}


int Datum::getNElements() const {
    switch (datatype) {
        case M_INTEGER:
        case M_FLOAT:
        case M_BOOLEAN:
        case M_STRING:
            return 1;
            
        case M_LIST:
            return listValue.size();
            
        case M_DICTIONARY:
            return dictValue.size();
            
        default:
            return -1;
    }
}


bool Datum::hasKey(const Datum &key) const {
    if (!isDictionary()) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot search for keys in %s", getDataTypeName());
        return false;
    }
    
    return (dictValue.find(key) != dictValue.end());
}


const Datum * Datum::getReadableElementPtr(const Datum &indexOrKey) const {
    const Datum *result = nullptr;
    
    switch (datatype) {
        case M_LIST:
            if (!indexOrKey.isInteger()) {
                merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot index list with %s", indexOrKey.getDataTypeName());
            } else {
                int index = indexOrKey.getInteger();
                if (index < 0 || index >= listValue.size()) {
                    merror(M_SYSTEM_MESSAGE_DOMAIN, "Requested list index (%d) is out of bounds", index);
                } else {
                    result = &(listValue.at(index));
                }
            }
            break;
            
        case M_DICTIONARY: {
            auto iter = dictValue.find(indexOrKey);
            if (iter != dictValue.end()) {
                result = &(iter->second);
            }
            break;
        }
            
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot get element from %s", getDataTypeName());
            break;
    }
    
    return result;
}


const Datum& Datum::getElement(const Datum &indexOrKey) const {
    if (auto elementPtr = getReadableElementPtr(indexOrKey)) {
        return *elementPtr;
    }
    static const Datum notFound;
    return notFound;
}


const Datum& Datum::getElement(const std::vector<Datum> &indexOrKeyPath) const {
    const Datum *elementPtr = this;
    for (auto &indexOrKey : indexOrKeyPath) {
        elementPtr = elementPtr->getReadableElementPtr(indexOrKey);
        if (!elementPtr) {
            static const Datum notFound;
            return notFound;
        }
    }
    return *elementPtr;
}


void Datum::setInteger(long long newdata) {
    *this = Datum(newdata);
}


void Datum::setFloat(double newdata) {
    *this = Datum(newdata);
}


void Datum::setBool(bool newdata) {
    *this = Datum(newdata);
}


void Datum::setString(const char * newdata, int size) {
    *this = Datum(newdata, size);
}


void Datum::setString(const char * newdata) {
    *this = Datum(newdata);
}


void Datum::setString(const std::string &newdata) {
    *this = Datum(newdata);
}


void Datum::setString(std::string &&newdata) {
    *this = Datum(std::move(newdata));
}


void Datum::setStringQuoted(const std::string &s) {
    if (s.size() < 2 ||
        ((s.front() != '"') && (s.front() != '\'')) ||
        ((s.back() != '"') && (s.back() != '\'')))
    {
        setString(s);
        return;
    }
    
    std::string t;
    t.reserve(s.size() + s.size() / 32);
    
    for (std::size_t i = 1; i + 1 < s.size(); i++) {
        if (s[i] != '\\') {
            t += s[i];
        } else {
            i++;
            if (i == s.size() - 1) {
                setString(s);
                return;
            }
            
            switch (s[i]) {
                case 'a':	t += '\a';	break;
                case 'b':	t += '\b';	break;
                case 'f':	t += '\f';	break;
                case 'n':	t += '\n';	break;
                case 'r':	t += '\r';	break;
                case 't':	t += '\t';	break;
                case 'v':	t += '\v';	break;
                case '\'':	t += '\'';	break;
                case '"':	t += '"';	break;
                case '\\':	t += '\\';	break;
                case '?':	t += '?';	break;
                case '$':	t += '$';	break;
                    
                default:
                    t += '\\';
                    t += s[i];
                    break;
            }
        }
    }
    
    setString(t);
}


void Datum::setList(const list_value_type &list) {
    *this = Datum(list);
}


void Datum::setList(list_value_type &&list) {
    *this = Datum(std::move(list));
}


void Datum::setDict(const dict_value_type &dict) {
    *this = Datum(dict);
}


void Datum::setDict(dict_value_type &&dict) {
    *this = Datum(std::move(dict));
}


void Datum::addElement(const Datum &value) {
    if (!isList()) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot append element to %s", getDataTypeName());
        return;
    }
    
    listValue.push_back(value);
}


void Datum::addElement(const Datum &key, const Datum &value) {
    if (!isDictionary()) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot add key/value pair to %s", getDataTypeName());
        return;
    }
    
    dictValue[key] = value;
}


Datum * Datum::getWritableElementPtr(const Datum &indexOrKey) {
    Datum *result = nullptr;
    
    switch (datatype) {
        case M_LIST:
            if (!indexOrKey.isInteger()) {
                merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot index list with %s", indexOrKey.getDataTypeName());
            } else {
                int index = indexOrKey.getInteger();
                if (index < 0) {
                    merror(M_SYSTEM_MESSAGE_DOMAIN, "List index cannot be negative");
                } else {
                    // Expand list if needed
                    if (index >= listValue.size()) {
                        listValue.resize(index + 1);
                    }
                    result = &(listValue.at(index));
                }
            }
            break;
            
        case M_DICTIONARY:
            result = &(dictValue[indexOrKey]);
            break;
            
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot set element in %s", getDataTypeName());
            break;
    }
    
    return result;
}


bool Datum::setElement(const Datum &indexOrKey, const Datum &value) {
    if (auto elementPtr = getWritableElementPtr(indexOrKey)) {
        *elementPtr = value;
        return true;
    }
    return false;
}


bool Datum::setElement(const std::vector<Datum> &indexOrKeyPath, const Datum &value) {
    Datum *elementPtr = this;
    for (auto &indexOrKey : indexOrKeyPath) {
        elementPtr = elementPtr->getWritableElementPtr(indexOrKey);
        if (!elementPtr) {
            return false;
        }
    }
    *elementPtr = value;
    return true;
}


Datum::operator short() const {
    return getInteger();
}


Datum::operator int() const {
    return getInteger();
}


Datum::operator long() const {
    return getInteger();
}


Datum::operator long long() const {
    return getInteger();
}


Datum::operator float() const {
    return getFloat();
}


Datum::operator double() const {
    return getFloat();
}


Datum::operator bool() const {
    return getBool();
}


Datum::operator std::string() const {
    return getString();
}


void Datum::operator++() {
    switch (datatype) {
        case M_INTEGER:
            intValue++;
            break;
            
        case M_FLOAT:
            floatValue += 1.0;
            break;
            
        case M_BOOLEAN:
            boolValue = !boolValue;
            break;
            
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Operation ++ is undefined on %s", getDataTypeName());
            break;
    }
}


void Datum::operator--() {
    switch (datatype) {
        case M_INTEGER:
            intValue--;
            break;
            
        case M_FLOAT:
            floatValue -= 1.0;
            break;
            
        case M_BOOLEAN:
            boolValue = !boolValue;
            break;
            
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Operation -- is undefined on %s", getDataTypeName());
            break;
    }
}


Datum Datum::operator-() const {
    if (isInteger()) {
        return -getInteger();
    } else if (isFloat()) {
        return -getFloat();
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot negate %s", getDataTypeName());
    return 0;
}


Datum Datum::operator+(const Datum& other) const {
    if (isInteger()) {
        if (other.isInteger()) {
            return getInteger() + (long long)other;
        } else if (other.isFloat()) {
            return getInteger() + (double)other;
        }
    } else if (isFloat()) {
        if (other.isInteger()) {
            return getFloat() + (long long)other;
        } else if (other.isFloat()) {
            return getFloat() + (double)other;
        }
    } else if (isString()) {
        if (other.isString()) {
            return Datum(getString() + other.getString());
        }
    } else if (isList()) {
        if (other.isList()) {
            Datum result(*this);
            result.listValue.insert(result.listValue.end(), other.listValue.begin(), other.listValue.end());
            return result;
        }
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot add %s and %s", getDataTypeName(), other.getDataTypeName());
    return 0;
}


Datum Datum::operator-(const Datum& other) const {
    if (isInteger()) {
        if (other.isInteger()) {
            return getInteger() - (long long)other;
        } else if (other.isFloat()) {
            return getInteger() - (double)other;
        }
    } else if (isFloat()) {
        if (other.isInteger()) {
            return getFloat() - (long long)other;
        } else if (other.isFloat()) {
            return getFloat() - (double)other;
        }
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot subtract %s and %s", getDataTypeName(), other.getDataTypeName());
    return 0;
}


static Datum multiplyStringAndInteger(const std::string &stringValue, long long intValue) {
    if (intValue < 0) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
               "Cannot multiply %s by negative %s",
               Datum::getDataTypeName(M_STRING),
               Datum::getDataTypeName(M_INTEGER));
        return 0;
    }
    
    std::string result;
    for (long long i = 0; i < intValue; i++) {
        result.append(stringValue);
    }
    
    return Datum(result);
}


static Datum multiplyListAndInteger(const Datum::list_value_type &listValue, long long intValue) {
    if (intValue < 0) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
               "Cannot multiply %s by negative %s",
               Datum::getDataTypeName(M_LIST),
               Datum::getDataTypeName(M_INTEGER));
        return 0;
    }
    
    Datum::list_value_type result;
    for (long long i = 0; i < intValue; i++) {
        result.insert(result.end(), listValue.begin(), listValue.end());
    }
    
    return Datum(result);
}


Datum Datum::operator*(const Datum& other) const {
    if (isInteger()) {
        if (other.isInteger()) {
            return getInteger() * (long long)other;
        } else if (other.isFloat()) {
            return getInteger() * (double)other;
        } else if (other.isString()) {
            return multiplyStringAndInteger(other.getString(), getInteger());
        } else if (other.isList()) {
            return multiplyListAndInteger(other.getList(), getInteger());
        }
    } else if (isFloat()) {
        if (other.isInteger()) {
            return getFloat() * (long long)other;
        } else if (other.isFloat()) {
            return getFloat() * (double)other;
        }
    } else if (isString()) {
        if (other.isInteger()) {
            return multiplyStringAndInteger(getString(), other.getInteger());
        }
    } else if (isList()) {
        if (other.isInteger()) {
            return multiplyListAndInteger(getList(), other.getInteger());
        }
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot multiply %s and %s", getDataTypeName(), other.getDataTypeName());
    return 0;
}


Datum Datum::operator/(const Datum& other) const {
    if (isNumber() && other.isNumber()) {
        double divisor = other.getFloat();
        if (divisor == 0.0) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Division by zero");
            return 0;
        }
        return getFloat() / divisor;
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot divide %s and %s", getDataTypeName(), other.getDataTypeName());
    return 0;
}


static inline long long getModulusOperand(const Datum &datum) {
    auto intValue = datum.getInteger();
    if (datum.isFloat()) {
        auto floatValue = datum.getFloat();
        if (double(intValue) != floatValue) {
            mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Modulus operator will truncate non-integer operand %g", floatValue);
        }
    }
    return intValue;
}


Datum Datum::operator%(const Datum& other) const {
    if (isNumber() && other.isNumber()) {
        auto dividend = getModulusOperand(*this);
        auto divisor = getModulusOperand(other);
        if (divisor == 0) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Division by zero");
            return 0;
        }
        return dividend % divisor;
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot mod %s and %s", getDataTypeName(), other.getDataTypeName());
    return 0;
}


bool Datum::operator==(const Datum& other) const {
    if (isInteger()) {
        if (other.isInteger()) {
            return (getInteger() == (long long)other);
        } else if (other.isFloat()) {
            return (getInteger() == (double)other);
        }
    } else if (isFloat()) {
        if (other.isInteger()) {
            return (getFloat() == (long long)other);
        } else if (other.isFloat()) {
            return (getFloat() == (double)other);
        }
    } else if (isString()) {
        if (other.isString()) {
            return stringValue == other.stringValue;
        }
    } else if (isDictionary()) {
        if (other.isDictionary()) {
            return dictValue == other.dictValue;
        }
    } else if (isList()) {
        if (other.isList()) {
            return listValue == other.listValue;
        }
    } else if (isUndefined()) {
        if (other.isUndefined()) {
            return true;
        }
    }
    
    return false;
}


bool Datum::operator!=(const Datum& other) const {
    return !(*this == other);
}


template<template<typename T> class Op>
inline bool Datum::compareOrdering(const Datum& other) const {
    if (isInteger()) {
        if (other.isInteger()) {
            return Op<long long>()(getInteger(), other.getInteger());
        } else if (other.isFloat()) {
            return Op<double>()(getFloat(), other.getFloat());
        }
    } else if (isFloat()) {
        if (other.isNumber()) {
            return Op<double>()(getFloat(), other.getFloat());
        }
    } else if (isString()) {
        if (other.isString()) {
            return Op<std::string>()(stringValue, other.stringValue);
        }
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot test ordering of %s and %s", getDataTypeName(), other.getDataTypeName());
    return false;
}


bool Datum::operator<(const Datum& other) const {
    return compareOrdering<std::less>(other);
}


bool Datum::operator<=(const Datum& other) const {
    return compareOrdering<std::less_equal>(other);
}


bool Datum::operator>(const Datum& other) const {
    return compareOrdering<std::greater>(other);
}


bool Datum::operator>=(const Datum& other) const {
    return compareOrdering<std::greater_equal>(other);
}


const Datum& Datum::operator[](const Datum &indexOrKey) const {
    auto &result = getElement(indexOrKey);
    
    if (isDictionary() && result.isUndefined()) {
        merror(M_SYSTEM_MESSAGE_DOMAIN,
               "Dictionary has no element for requested key (%s)",
               indexOrKey.toString(true).c_str());
    }
    
    return result;
}


std::string Datum::toString(bool quoted) const {
    if (quoted && isString()) {
        return getStringQuoted();
    }
	std::ostringstream buf;
    buf << *this;
    return buf.str();
}


std::ostream& operator<<(std::ostream &buf, const Datum &d) {
    switch (d.getDataType()) {
        case M_INTEGER:
            buf << d.getInteger();
            break;
            
        case M_FLOAT:
            buf << d.getFloat();
            break;
            
        case M_BOOLEAN:
            buf << (d.getBool() ? "true" : "false");
            break;
            
        case M_STRING:
            buf << d.getString();
            break;
            
        case M_LIST: {
            buf << "[";
            int i = 0;
            for (auto &item : d.getList()) {
                if (i > 0) {
                    buf << ", ";
                }
                
                if (item.isString()) {
                    buf << item.getStringQuoted();
                } else {
                    buf << item;
                }
                
                i++;
            }
            buf << "]";
            break;
        }
            
        case M_DICTIONARY: {
            buf << "{";
            int i = 0;
            for (auto &item : d.getDict()) {
                if (i > 0) {
                    buf << ", ";
                }
                
                auto &key = item.first;
                if (key.isString()) {
                    buf << key.getStringQuoted();
                } else {
                    buf << key;
                }
                
                buf << ": ";
                
                auto &value = item.second;
                if (value.isString()) {
                    buf << value.getStringQuoted();
                } else {
                    buf << value;
                }
                
                i++;
            }
            buf << "}";
            break;
        }
            
        default:
            break;
    }
    
	return buf;
}


END_NAMESPACE_MW
