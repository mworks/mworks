/**
 * GenericData.h
 *
 * You need to be careful when copying dictionaries and lists.  The under-
 * lying objects are NOT copied, so modifying a dictionary if you are not the
 * only one using use scarabDeepCopy instead to create a new Datum object.
 *
 *
 * Copyright (c) 2005 MIT. All rights reserved.
 */
#ifndef _GENERIC_DATA_H
#define _GENERIC_DATA_H

#include "MWorksMacros.h"
#include "MWorksTypes.h"
#include "ScarabServices.h"
#include "Lockable.h"
#include <string>
#include <vector>
#include <map>

#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
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

    
#define M_NO_DATA   0      // JJD added Aug 16, 2005
	
#define INTERNALLY_LOCKED_MDATA 0

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


class Datum
#if INTERNALLY_LOCKED_MDATA
    : public Lockable
#endif
{
    
private:
    GenericDataType datatype;
    ScarabDatum *data;
    
    void initScarabDatum();
    void setDataType(GenericDataType _type);
    
    void createDictionary(int intialsize);
    void createList(int size);
    
    void lockDatum() const;
    void unlockDatum() const;
    
public:
    /**
     * Constructors:  All constructors create a scarab object that can
     * be sent over the network.
     */
    Datum();
    Datum(GenericDataType type, int arg);
    Datum(GenericDataType type, double arg);
    Datum(int value);
    Datum(long value);
    Datum(long long value);
    Datum(double value);
    Datum(float value);
    Datum(bool value);
    Datum(const char * string, int size);
    Datum(const char * string);
    Datum(const std::string &string);
    
    
    Datum(const map<int, string> &dict) {
        
        int size = dict.size();
        createDictionary(size);
        
        map<int,string>::const_iterator i;
        for(i = dict.begin(); i != dict.end(); ++i){
            
            const pair<int, string> &key_val = *i;
            Datum key((long)key_val.first);
            Datum val((string)key_val.second);
            
            this->addElement(key, val);
        }
    }
    
    
    Datum(const map<string, int> &dict) {
        
        int size = dict.size();
        createDictionary(size);
        
        map<string,int>::const_iterator i;
        for(i = dict.begin(); i != dict.end(); ++i){
            
            const pair<string, int> &key_val = *i;
            Datum key(key_val.first);
            Datum val((long)key_val.second);
            
            this->addElement(key, val);
        }
    }
    
    
    /**
     * Overrides the default copy constructor to deeply copy
     * the scarab datum.
     */
    Datum(const Datum& that);
    
    /**
     * Copies the scarabdatum, deeply, used for receiving data events
     * on the network.
     */
    Datum(ScarabDatum * datum);
    
    
    
    
    /**
     * Default Destructor.
     */
    ~Datum();
    
    /**
     * Returns the data type of the Datum object.
     */
    GenericDataType getDataType() const;
    const char * getDataTypeName() const;
    
    /**
     * Returns the scarab data package that stores this object
     */
    ScarabDatum * getScarabDatum()  const;
    ScarabDatum * getScarabDatumCopy()  const;
    
    /**
     * If datatype is one of the numeric types then it will return
     * a representation for it, otherwise it will return 0.
     */
    bool getBool() const;
    double getFloat() const;
    long long getInteger() const;
    
    /**
     * Returns a string if Datum is of type M_STRING, otherwise returns 0
     * lenght of string is null terminated length or -1 otherwise
     */
    const char *getString() const;
    int getStringLength() const;
    bool stringIsCString() const;
    std::string getStringQuoted() const;
    
    // debug function to print the datum.
    void printToSTDERR() const;
    
    // Removes old data and creates a new datum with the new value.
    void setBool(bool value);
    void setFloat(double value);
    void setInteger(long long value);
    void setString(const char * string, int size);
    void setString(const char * string);
    void setString(const std::string &string);
    void setStringQuoted(const std::string &string);
    
    bool isInteger()  const;
    bool isFloat() const;
    bool isBool() const;
    bool isString() const;
    bool isDictionary() const;
    bool isList() const;
    bool isUndefined() const;
    bool isNumber() const;
    
    // this will overwrite the default asssignment operator.
    Datum& operator=(const Datum& that);
    
    operator long() const;
    operator int() const;
    operator short() const;
    operator double() const;
    operator float() const;
    operator bool() const;
    operator long long() const;
    operator std::string() const;
    
    void operator=(long long newdata);
    void operator=(long newdata);
    void operator=(int newdata);
    void operator=(short newdata);
    void operator=(bool newdata);
    void operator=(double newdata);
    void operator=(float newdata);
    void operator=(const char * newdata);
    void operator=(const std::string &newdata);
    
    //void inc();
    // prefix operators.
    void operator++();
    void operator--();
    
    bool operator==(long newdata) const;
    bool operator==(double newdata) const;
    bool operator==(bool newdata) const;
    bool operator==(const char * newdata) const;
    bool operator!=(const char * newdata) const;
    
    Datum operator-() const;
    
    Datum operator+(const Datum&) const;
    Datum operator-(const Datum&) const;
    Datum operator*(const Datum&) const;
    Datum operator/(const Datum&) const;
    Datum operator%(const Datum&) const;
    
    bool operator==(const Datum&) const;
    bool operator!=(const Datum&) const;
    bool operator>(const Datum&) const;
    bool operator>=(const Datum&) const;
    bool operator<(const Datum&) const;
    bool operator<=(const Datum&) const;
    
    int getNElements() const;
    int getMaxElements() const;
    
    bool hasKey(const Datum &key) const;
    std::vector<Datum> getKeys() const;
    
    
    void addElement(const Datum &key, const Datum &value);
    void addElement(const Datum &value);
    void setElement(int index, const Datum &value);
    std::vector<Datum> getElements() const;
    
    Datum getElement(const Datum &key) const;
    Datum getElement(int i) const;
    
    //  Datum removeElement(const char * key);
    
    // TODO: MUST IMPLEMENT THESE
    // Datum removeElement(const Datum key);
    // TODO
    //  void removeElement(int index);
    
    
    Datum operator[](const Datum &index) const;
    Datum operator[](int i) const;
    
    std::string toString(bool quoted = false) const;
    
private:
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive &ar, const unsigned int version){
        boost::serialization::split_member(ar, *this, version);
    }
    
    template<class Archive>
    void save(Archive &ar, const unsigned int version) const{
        
        long long ival;
        double fval;
        bool bval;
        int string_length;
        int nelements;
        std::vector<Datum> keys;
        
        ar << datatype;
        switch(datatype){
            case M_INTEGER:
                ival = getInteger();
                ar << ival;
                break;
            case M_FLOAT:
                fval = getFloat();
                ar << fval;
                break;
            case M_BOOLEAN:
                bval = getBool();
                ar << bval;
                break;
            case M_STRING:
                string_length = getStringLength();
                ar << string_length;
                ar.save_binary(getString(), string_length);
                break;
            case M_LIST:
                nelements = getNElements();
                ar << nelements;
                for(int i = 0; i < nelements; i++){
                    Datum val = getElement(i);
                    ar << val;
                }
                break;
            case M_DICTIONARY:
                nelements = getNElements();
                ar << nelements;
                keys = getKeys();
                for(int i = 0; i < keys.size(); i++){
                    Datum key = keys[i];
                    ar << key;
                    Datum val = getElement(key);
                    ar << val;
                }
                break;
            default:
                break;
                
        }
    }
    
    template<class Archive>
    void load(Archive &ar, const unsigned int version){
        
        long long ival;
        double fval;
        bool bval;
        int nelements;
        Datum datum, key;
        int string_length;
        //char c;
        
        ar >> datatype;
        switch(datatype){
            case M_INTEGER:
                ar >> ival;
                setInteger(ival);
                break;
            case M_FLOAT:
                ar >> fval;
                setFloat(fval);
                break;
            case M_BOOLEAN:
                ar >> bval;
                setBool(bval);
                break;
            case M_STRING:
                ar >> string_length;
                if (version > 0) {
                    char buffer[string_length];
                    ar.load_binary(&buffer, string_length);
                    setString(buffer, string_length);
                } else {
                    std::string sval;
                    ar >> sval;
                    setString(sval);
                }
                break;
            case M_LIST:
                ar >> nelements;
                createList(nelements);
                for(int i = 0; i < nelements; i++){
                    ar >> datum;
                    addElement(datum);
                }
                break;
            case M_DICTIONARY:
                ar >> nelements;
                createDictionary(nelements);
                for(int i = 0; i < nelements; i++){
                    ar >> key;
                    ar >> datum;
                    addElement(key, datum);
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
BOOST_CLASS_VERSION(mw::Datum, 1)


#endif



























