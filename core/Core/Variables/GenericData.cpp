/**
 * GenericData.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 01/28/05 - Added constructor to create Datum objects from
 *                      ScarabDatums used in network transfers. Fixed spacing
 *
 * Paul Jankunas on 4/06/05 - Changing the way scarab objects are generated.
 *
 * Copyright (c) 2005 MIT. All rights reserved.
 */

#include "GenericData.h"
#include "Utilities.h"
#include "ScarabServices.h"
#include <string>
#include <sstream>

#include "ExpressionParser.h"


BEGIN_NAMESPACE_MW


void Datum::initScarabDatum(){
  // TODO, this could cause a shitload of memory leaks
  // when setting a new Datum it null's the ScarabDatum pointer and then writes a new one
  //  it never deletes the old one
  //  DDC: not really... doesn't get called except in constructors
  //       we should phase it out nonetheless
  data = NULL;
}

Datum::Datum() {
  initScarabDatum();
  //setInteger(-1);
  setDataType(M_UNDEFINED);
}


Datum::Datum(GenericDataType type, int arg) {
  
  initScarabDatum();

  switch(type) {
  case M_INTEGER:
    setInteger(arg);
    break;
  case M_FLOAT:
    setFloat((float)arg);
    break;
  case M_BOOLEAN:
    setBool((bool)arg);
    break;
  case M_STRING:
	  // TODO string arg
	  setString("", 0);
	  break;
  case M_LIST:
    createList(arg);
    break;
  case M_DICTIONARY:
    createDictionary(arg);
    break;
  default:
    setInteger(-1);
    setDataType(M_UNDEFINED);
    break;    
  }
}

Datum::Datum(GenericDataType type, double arg) {
  
  
  initScarabDatum();

  switch(type) {
  case M_INTEGER:
    setInteger((int)arg);
    break;
  case M_FLOAT:
    setFloat(arg);
    break;
  case M_BOOLEAN:
    setBool((bool)arg);
    break;
  case M_STRING:
    // TODO string arg
    setString("", 0);
    break;
  case M_LIST:
    createList((int)arg);
    break;
  case M_DICTIONARY:
    createDictionary((int)arg);
    break;
  default:
    setInteger(-1);
    setDataType(M_UNDEFINED);
    break;    
  }
}

Datum::Datum(double newdata) {
  initScarabDatum();
  setDataType(M_FLOAT);
  setFloat(newdata);
}


Datum::Datum(float newdata) {
  initScarabDatum();
  setDataType(M_FLOAT);
  setFloat(newdata);
}


Datum::Datum(int newdata){
    initScarabDatum();
    setDataType(M_INTEGER);
    setInteger(newdata);
}


Datum::Datum(long newdata){
  initScarabDatum();
  setDataType(M_INTEGER);
  setInteger(newdata);
}

Datum::Datum(long long newdata){
  initScarabDatum();
  setDataType(M_INTEGER);
  setInteger(newdata);
}

Datum::Datum(const char * string, int size) {
    initScarabDatum();
    setString(string, size);
}

Datum::Datum(const char * string) {
  initScarabDatum();
  setString(string);
}

Datum::Datum(const std::string &string){
  initScarabDatum();
  setString(string);
}

Datum::Datum(bool newdata) {
  initScarabDatum();
  setDataType(M_BOOLEAN);
  setBool(newdata);
}

Datum::Datum(const Datum& that) {
  
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif
  
  data = NULL;

  if(that.getDataType() == M_UNDEFINED) {
	setDataType(M_UNDEFINED);      
  } else {
      
    if(that.data == NULL){
        setDataType(M_UNDEFINED);
        return;
    }
    data = scarab_copy_datum(that.data);
      
	GenericDataType datatype = that.getDataType();
	int scarabtype = data->type;
	
	if(scarabtype == SCARAB_INTEGER && datatype != M_INTEGER && datatype != M_BOOLEAN){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				"Mismatched data types during data copy (internal datum is integer, but datatype == %d)",
				datatype);
		datatype = M_INTEGER;
	} 

	if(scarabtype == SCARAB_FLOAT && datatype != M_FLOAT){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				"Mismatched data types during data copy (internal datum is float, but datatype == %d)",
				datatype);
		datatype = M_FLOAT;
	} 
		
	if(scarabtype == SCARAB_OPAQUE && datatype != M_STRING){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				"Mismatched data types during data copy (internal datum is string, but datatype == %d)",
				datatype);
		datatype = M_STRING;
	} 
	
	if(scarabtype == SCARAB_DICT && datatype != M_DICTIONARY){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				"Mismatched data types during data copy (internal datum is dictionary, but datatype == %d)",
				datatype);
		datatype = M_DICTIONARY;
	} 
	
	if(scarabtype == SCARAB_LIST && datatype != M_LIST){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				"Mismatched data types during data copy..." 
				 "(internal datum is list, but datatype == %d)",
				datatype);
		datatype = M_LIST;
	} 
	
	setDataType(that.getDataType());
  }
  
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
}

Datum::Datum(ScarabDatum * datum) {  
  
  data = 0;
  
  if(datum == NULL){
    //mwarning( M_SYSTEM_MESSAGE_DOMAIN,
	//      "Attempt to create an Datum object from a NULL ScarabDatum");
    //setInteger(-1);
    setDataType(M_UNDEFINED);
    return;
  }
  
  switch(datum->type) {
  case SCARAB_INTEGER:
	// TODO do we want a separate class for bools?
    setDataType(M_INTEGER);
    break;
  case SCARAB_FLOAT:
    setDataType(M_FLOAT);
    break;
  case SCARAB_OPAQUE:
	// TODO do we want a separate class for string?
    setDataType(M_STRING);
    break;
  case SCARAB_DICT:
    setDataType(M_DICTIONARY);
    break;
  case SCARAB_LIST:
    setDataType(M_LIST);
    break;
  default:
    setInteger(-1);
    setDataType(M_UNDEFINED);
    return;

  }

  data = scarab_copy_datum(datum);
}


Datum::~Datum() {
  scarab_free_datum(data);
  data = NULL;
}


void Datum::lockDatum()  const{
	ScarabDatum *theDatum = getScarabDatum();
	scarab_lock_datum(theDatum);
}

void Datum::unlockDatum()  const{
	ScarabDatum *theDatum = getScarabDatum();
	scarab_unlock_datum(theDatum);
}

GenericDataType Datum::getDataType() const {
  return datatype;
}

const char * Datum::getDataTypeName() const {
    switch (datatype) {
        case M_BOOLEAN:
            return "boolean";
        case M_INTEGER:
            return "integer";
        case M_FLOAT:
            return "float";
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

void Datum::setDataType(GenericDataType type){
  datatype = type;
}

ScarabDatum * Datum::getScarabDatum() const{ 
  return data;
}

ScarabDatum * Datum::getScarabDatumCopy() const{
  ScarabDatum *new_datum = scarab_copy_datum(data);
  
  return new_datum;
}

bool Datum::getBool() const{
	
	if(data == NULL){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Attempt to access a broken Datum object");
		return false;
	}

	bool result = false;
  
	switch (datatype) {
		case M_INTEGER:
		case M_BOOLEAN:
			result = bool(data->data.integer);
			break;
		case M_FLOAT:
			result = bool(data->data.floatp);
			break;
		default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to boolean", getDataTypeName());
			break;
	}      
	
	return result;      
}

double Datum::getFloat() const {
	
	#if INTERNALLY_LOCKED_MDATA
		lock();
	#endif

	if(data == NULL){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Attempt to access a broken Datum object");
		#if INTERNALLY_LOCKED_MDATA
			unlock();
		#endif
			return 0.0;
	}
  
	double result = 0.0;
  
	switch (datatype) {
		case M_INTEGER:
		case M_BOOLEAN:
			result = (double)(data->data.integer);
			break;
		case M_FLOAT:
			result = data->data.floatp;
			break;
		default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to float", getDataTypeName());
			break;
	}
  
	#if INTERNALLY_LOCKED_MDATA
		unlock();
	#endif
	return result;
}

long long Datum::getInteger() const{
#if INTERNALLY_LOCKED_MDATA
    lock();
#endif
    
    if(data == NULL){
        mwarning(M_SYSTEM_MESSAGE_DOMAIN,
                 "Attempt to access a broken Datum object");
#if INTERNALLY_LOCKED_MDATA
        unlock();
#endif
        return 0;
    }
    
    long long result = 0;
    
    switch (datatype) {
        case M_INTEGER:
        case M_BOOLEAN:
            result = data->data.integer;
            break;
        case M_FLOAT:
            result = (long long)(data->data.floatp);
            break;
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to integer", getDataTypeName());
            break;
    }
    
#if INTERNALLY_LOCKED_MDATA
    unlock();
#endif
    return result;
}

const char * Datum::getString() const{
#if INTERNALLY_LOCKED_MDATA
    lock();
#endif
    
    const char *result = nullptr;
    
    switch (datatype) {
        case M_STRING:
            result = (const char *)data->data.opaque.data;
            break;
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to string", getDataTypeName());
            break;
    }
    
#if INTERNALLY_LOCKED_MDATA
    unlock();
#endif
    return result;
}

int Datum::getStringLength()  const{
  
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif
  
  int result = 0;
  
  switch (datatype) {
  case M_STRING:
    result = data->data.opaque.size;
	break;
  default:                
    result = -1;
	break;
  }
  
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
  return result;
}

bool Datum::stringIsCString() const {
#if INTERNALLY_LOCKED_MDATA
    lock();
#endif
    bool result;
    
    switch (datatype) {
        case M_STRING:
            result = scarab_opaque_is_string(data);
            break;
        default:
            result = false;
            break;
    }
    
#if INTERNALLY_LOCKED_MDATA
    unlock();
#endif
    return result;
}


std::string Datum::getStringQuoted() const {
    if (!isString()) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot convert %s to string", getDataTypeName());
        return "";
    }
    
    const std::string str = std::string(*this);
    std::string os = "\"";
    
    os.reserve(2 + str.size() + str.size() / 16);
    
    for (char c : str) {
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
                
            default:
                os += c;
                break;
        }
    }
    
    os += "\"";
    return os;
}


void Datum::setBool(bool newdata) {
  
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif
  
  datatype = M_BOOLEAN;
  if(data != NULL){
    scarab_free_datum(data);
  }
  data = scarab_new_integer( newdata);
  
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
}

void Datum::setInteger(long long newdata) {
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif
  datatype = M_INTEGER;
  if(data != NULL){
    scarab_free_datum(data);
  }
  data = scarab_new_integer( newdata );
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
}

void Datum::setFloat(double newdata) {
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif
  datatype = M_FLOAT;
  
  if(data != NULL){
    scarab_free_datum(data);
  }
  data = scarab_new_float( newdata);
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
}


void Datum::setString(const char * newdata, int size) {
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif
  datatype = M_STRING;

  if(data != NULL){
	scarab_free_datum(data); // TODO: why isn't this safe?
  }
  
  data = scarab_new_opaque(newdata, size);

  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
}


void Datum::setString(const char * newdata) {
    setString(newdata, strlen(newdata)+1);
}


void Datum::setString(const std::string &newdata) {
    setString(newdata.c_str(), newdata.size()+1);
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
                    
                default:
                    t += '\\';
                    t += s[i];
                    break;
            }
        }
    }
    
    setString(t);
}


bool Datum::isInteger()  const{ return (datatype == M_BOOLEAN) || (datatype == M_INTEGER); }

bool Datum::isBool()  const{ return (datatype == M_BOOLEAN); }
    
bool Datum::isFloat()  const{ return (datatype == M_FLOAT); }
	
bool Datum::isString()  const{ return (datatype == M_STRING); }

bool Datum::isDictionary()  const{ return (datatype == M_DICTIONARY); }

bool Datum::isList() const { return (datatype == M_LIST); }

bool Datum::isUndefined() const { return (datatype == M_UNDEFINED || data == NULL); }

bool Datum::isNumber() const {
	return (!isUndefined() && (isFloat() || isInteger() || isBool()));
}

Datum& Datum::operator=(const Datum& that) {

  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif
  
  setDataType(that.getDataType());

  if(that.getDataType() == M_UNDEFINED) {
    //setInteger(-1);
    setDataType(M_UNDEFINED);
	if(data != NULL){
		scarab_free_datum(data);
		data = NULL;
	}
  } else {
    setDataType(that.getDataType());
	scarab_free_datum(data); // DDC added (leaky leaky)
    data = scarab_copy_datum(that.data);    
  }

  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
  return *this;
}

void Datum::operator=(long long newdata) {
  setInteger(newdata);
}

void Datum::operator=(long newdata) {
  setInteger(newdata);
}

void Datum::operator=(int newdata) {
  setInteger(newdata);
}

void Datum::operator=(short newdata) {
  setInteger(newdata);
}

void Datum::operator=(bool newdata) {
  setBool(newdata);
}

void Datum::operator=(double newdata) {
  setFloat(newdata);
}

void Datum::operator=(float newdata) {
  setFloat(newdata);
}

void Datum::operator=(const char * newdata) {
  setString(newdata);
}

void Datum::operator=(const std::string &newdata){
  setString(newdata);
}


void Datum::operator++() {
  switch(datatype) {
  case M_INTEGER:
    setInteger(getInteger() + 1);
    break;
  case M_FLOAT:
    setFloat(getFloat() + 1);
    break;
  case M_BOOLEAN:
    if(getBool()){
      setBool(false);
    } else {
      setBool(true);
    }
    break;
  default:
    mwarning(M_SYSTEM_MESSAGE_DOMAIN,
	     "Operation ++ is undefined on data type %d", datatype);
  }
}

void Datum::operator--() {
  switch(datatype) {
  case M_INTEGER:
    setInteger(getInteger() - 1);
    break;
  case M_FLOAT:
    setFloat(getFloat() - 1);
    break;
  case M_BOOLEAN:
    if(getBool()){
      setBool(false);
    } else {
      setBool(true);
    }
    break;
  default:
    mwarning(M_SYSTEM_MESSAGE_DOMAIN,
	     "Operation -- is undefined on data type %d", datatype);
  }
}

bool Datum::operator==(long newdata)  const {
  switch(datatype) {
  case M_INTEGER:
    return (getInteger() == newdata);
    break;
  case M_FLOAT:
    return (getFloat() == newdata);
    break;
  case M_BOOLEAN:
    return (getBool() == newdata);
    break;
  default:
    mprintf("Operation == is undefined on this data type");
    return false; // to shut up the compiler
  } 
}


bool Datum::operator==(const char * newdata)  const {
  int eq;
  switch(datatype) {
  case M_STRING:
    eq = strncmp(newdata, getString(), getStringLength()); 
    return (eq == 0);
    break;
  default:
    mprintf("Operation == is undefined on this data type");
    return false; // to shut up the compiler
  } 
}

bool Datum::operator!=(const char * newdata) const {
  int eq;
  switch(datatype) {
  case M_STRING:
    eq = strncmp(newdata, getString(), getStringLength()); 
    return (eq != 0);
    break;
  default:
    mprintf("Operation != is undefined on this data type");
    return false; // to shut up the compiler
  } 
}

bool Datum::operator==(double newdata) const {
  switch(datatype) {
  case M_INTEGER:
    return (getInteger() == newdata);
    break;
  case M_FLOAT:
    return (getFloat() == newdata);
    break;
  case M_BOOLEAN:
    return (getBool() == newdata);
    break;
  default:
    mprintf("Operation == is undefined on this data type");
    return false;
  } 
}

bool Datum::operator==(bool newdata) const {
  switch(datatype) {
  case M_INTEGER:
    return (getInteger() == newdata);
    break;
  case M_FLOAT:
    return (getFloat() == newdata);
    break;
  case M_BOOLEAN:
    return (getBool() == newdata);
    break;
  default:
    mprintf("Operation == is undefined on this data type");
    return false;
  } 
} 


Datum::operator long() const{
  return getInteger();
}

Datum::operator long long() const{
  return getInteger();
}

Datum::operator int() const{
  return getInteger();
}

Datum::operator short() const{
  return getInteger();
}

Datum::operator double() const{
  return getFloat();
}

Datum::operator float() const{
  return getFloat();
}

Datum::operator bool() const{
  return getBool();
}

Datum::operator std::string() const {
  std::string returnval = getString();
  return returnval;
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


Datum Datum::operator+(const Datum& other)  const{
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
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot add %s and %s", getDataTypeName(), other.getDataTypeName());
    return 0;
}


Datum Datum::operator-(const Datum& other)  const{
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


Datum Datum::operator*(const Datum& other)  const{
    if (isInteger()) {
        if (other.isInteger()) {
            return getInteger() * (long long)other;
        } else if (other.isFloat()) {
            return getInteger() * (double)other;
        }
    } else if (isFloat()) {
        if (other.isInteger()) {
            return getFloat() * (long long)other;
        } else if (other.isFloat()) {
            return getFloat() * (double)other;
        }
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot multiply %s and %s", getDataTypeName(), other.getDataTypeName());
    return 0;
}


Datum Datum::operator/(const Datum& other)  const{
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


Datum Datum::operator%(const Datum& other)  const{
    if (isNumber() && other.isNumber()) {
        long long divisor = other.getInteger();
        if (divisor == 0) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Division by zero");
            return 0;
        }
        return getInteger() % divisor;
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
            return (other == getString());
        }
    } else if (isDictionary()) {
        if (other.isDictionary() && (getNElements() == other.getNElements())) {
            std::vector<Datum> keys = getKeys();
            for (int i = 0; i < keys.size(); i++) {
                Datum key(keys[i]);
                
                Datum val1 = getElement(key);
                Datum val2 = other.getElement(key);
                
                if (val1 != val2) {
                    return false;
                }
            }
            return true;
        }
    } else if (isList()) {
        if (other.isList() && (getMaxElements() == other.getMaxElements())) {
            for(int i=0; i<getMaxElements(); i++) {
                Datum val1, val2;
                val1 = getElement(i);
                val2 = other.getElement(i);
                
                if(val1 != val2) {
                    return false;
                }		
            }
            return true;
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


bool Datum::operator>(const Datum& other) const {
    if (isInteger()) {
        if (other.isInteger()) {
            return getInteger() > (long long)other;
        } else if (other.isFloat()) {
            return getInteger() > (double)other;
        }
    } else if (isFloat()) {
        if (other.isInteger()) {
            return getFloat() > (long long)other;
        } else if (other.isFloat()) {
            return getFloat() > (double)other;
        }
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot test ordering of %s and %s", getDataTypeName(), other.getDataTypeName());
    return false;
}


bool Datum::operator>=(const Datum& other)  const {
    if (isInteger()) {
        if (other.isInteger()) {
            return getInteger() >= (long long)other;
        } else if (other.isFloat()) {
            return getInteger() >= (double)other;
        }
    } else if (isFloat()) {
        if (other.isInteger()) {
            return getFloat() >= (long long)other;
        } else if (other.isFloat()) {
            return getFloat() >= (double)other;
        }
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot test ordering of %s and %s", getDataTypeName(), other.getDataTypeName());
    return false;
}


bool Datum::operator<(const Datum& other) const {
    if (isInteger()) {
        if (other.isInteger()) {
            return getInteger() < (long long)other;
        } else if (other.isFloat()) {
            return getInteger() < (double)other;
        }
    } else if (isFloat()) {
        if (other.isInteger()) {
            return getFloat() < (long long)other;
        } else if (other.isFloat()) {
            return getFloat() < (double)other;
        }
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot test ordering of %s and %s", getDataTypeName(), other.getDataTypeName());
    return false;
}


bool Datum::operator<=(const Datum& other) const {
    if (isInteger()) {
        if (other.isInteger()) {
            return getInteger() <= (long long)other;
        } else if (other.isFloat()) {
            return getInteger() <= (double)other;
        }
    } else if (isFloat()) {
        if (other.isInteger()) {
            return getFloat() <= (long long)other;
        } else if (other.isFloat()) {
            return getFloat() <= (double)other;
        }
    }
    
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot test ordering of %s and %s", getDataTypeName(), other.getDataTypeName());
    return false;
}


Datum Datum::operator[](const Datum &index) const {
    Datum result;
    
    switch (datatype) {
        case M_LIST:
            result = getElement(int(index.getInteger()));
            break;
            
        case M_DICTIONARY:
            result = getElement(index);
            if (result.isUndefined()) {
                merror(M_SYSTEM_MESSAGE_DOMAIN,
                       "Dictionary has no element for requested key (%s)",
                       index.toString(true).c_str());
            }
            break;
            
        default:
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot subscript %s", getDataTypeName());
            break;
    }
    
    return result;
}


Datum Datum::operator[](int i) const {
  if(getDataType() != M_LIST) {
    fprintf(stderr, "mData is not of type M_LIST -- Type => %d\n", getDataType());

 Datum undefined;
    return undefined;
  }
	
  return getElement(i);
}


void Datum::printToSTDERR() const {
	fprintf(stderr, "Datatype => %d\n", datatype);
	switch (datatype) {
		case M_INTEGER:
			fprintf(stderr, "Value is => %lld\n", getInteger());
			break;
		case M_BOOLEAN:
			fprintf(stderr, "Value is => %d\n", getBool());
			break;
		case M_FLOAT:
			fprintf(stderr, "Value is => %G\n", getFloat());
			break;
		case M_STRING:
			fprintf(stderr, "Value is => %s\n", getString());
			break;    
		case M_DICTIONARY:
		case M_LIST:
			fprintf(stderr, "Number of elements => %d\n", getNElements());
			break;    
		default:
			fprintf(stderr, "Undefined Type\n");
			break;
	}
	fflush(stderr);
}


void Datum::addElement(const Datum &key,  const Datum &value) {  
	if(getDataType() != M_DICTIONARY) {
		fprintf(stderr, "mData is not of type M_DICTIONARY -- Type => %d\n", 
				getDataType());
		return;
	}

	if(value.isUndefined() || key.isUndefined()) {
		fprintf(stderr, "Can't add undefined data or key type to M_DICTIONARY\n");
		return;    
	}

	ScarabDatum *whats_already_there;

	if(whats_already_there = scarab_dict_put(data, 
											 key.getScarabDatum(), 
											 value.getScarabDatum())) {
		scarab_free_datum(whats_already_there); // already decremented?
	}  
}


bool Datum::hasKey(const Datum &key)  const{
  if(getDataType() != M_DICTIONARY) {
    fprintf(stderr, 
			"Can't search for keys in something other than M_DICTIONARY -- Type => %d\n", 
			getDataType());
    return false;
  }

  ScarabDatum *datum = scarab_dict_get(data, key.getScarabDatum());
  
  bool doesit = (datum != NULL);

  return doesit;
}

Datum Datum::getElement(const Datum &key)  const{

  
  if(getDataType() != M_DICTIONARY) {
    fprintf(stderr, "Can't search for elements with keys in something other than M_DICTIONARY -- Type => %d\n", getDataType());
    Datum undefined;
    return undefined;
  }

  ScarabDatum *datum = scarab_dict_get(data, key.getScarabDatum());
  return Datum(datum);
}

int Datum::getNElements() const {
	int returnval = -1;
	
	switch(getDataType()) {
		case M_INTEGER:
		case M_FLOAT:
		case M_BOOLEAN:
		case M_STRING:
			returnval =  1;
			break;
		case M_DICTIONARY:	
			returnval = data->data.dict->size;
			break;
		case M_LIST:
			{
				int size = 0;
			
				for(int i=0; i<data->data.list->size; ++i) {
					if(data->data.list->values[i])
						size++;
				}
				
				returnval = size;
			}
			break;
		default:
			fprintf(stderr, "Can't identify type -- Type => %d\n", getDataType());
			returnval = -1;
			break;
	}

	
	return returnval;
}	

int Datum::getMaxElements()  const{
	int returnval = -1;
	
	switch(getDataType()) {
		case M_INTEGER:
		case M_FLOAT:
		case M_BOOLEAN:
		case M_STRING:
			returnval =  1;
			break;
		case M_DICTIONARY:	
			returnval =  data->data.dict->tablesize;
			break;
		case M_LIST:
			returnval = data->data.list->size;
			break;
		default:
			fprintf(stderr, "Can't identify type -- Type => %d\n", getDataType());
			returnval = -1;
			break;
	}
  
	return returnval;
}

/*
Datum Datum::getKey(int n)  const {

	if(getDataType() != M_DICTIONARY) {
		fprintf(stderr, "Can't get keys in something other than M_DICTIONARY -- Type => %d\n", getDataType());
	 Datum undef;
		return undef;
	}
  
	if(data->type != SCARAB_DICT){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Mismatched internal data type while setting a dictionary element; should be %d, is %d",
				 data->type, SCARAB_DICT);
	 Datum undef;
		return undef;
	}
  
	lockDatum();
	ScarabDatum ** sd = data->data.dict->keys;

	int n_keys = data->data.dict->tablesize;
  
	if(n < 0 || n >= n_keys){
		fprintf(stderr, "Key index (%d) is larger than number of keys (%d)\n", n, n_keys);
	 Datum undef;
		unlockDatum();
		return undef;
	}
  
	ScarabDatum *key = sd[n];
	
 Datum returnval(key);
	unlockDatum();

	return returnval; 
}
 */

std::vector<Datum> Datum::getKeys() const {
	std::vector<Datum> keys;
	
	if(getDataType() != M_DICTIONARY) {
		merror(M_SYSTEM_MESSAGE_DOMAIN, "attempting to get keys in something that's not a dictionary");
		return keys;
	}
	
	lockDatum();
	ScarabDatum ** sd = data->data.dict->keys;
	
    // DDC: 12/09: I think this should be size, not tablesize
    // CJS: 9/12: No, tablesize is correct.  size indicates the number of non-NULL keys in the table; you
    // have to interate through all tablesize slots to find them.
	int max_keys = data->data.dict->tablesize;
    
	for (int i = 0; i < max_keys; ++i) {
        if (sd[i]) {
            keys.push_back(sd[i]);
        }
	}
	
	unlockDatum();
	
	return keys;
}

//mData Datum::removeElement(const char * key) {
//	if(getDataType() != M_DICTIONARY) {
//		fprintf(stderr, "mData::removeElement(const char *)\n"
//				"Can't remove element using  keys in something other than M_DICTIONARY -- Type => %d\n",
//				getDataType());
//	 Datum undef;
//		return undef;
//	}
//  
//	ScarabDatum * scarab_key = scarab_new_string(key);
//
//	ScarabDatum *removedVal = scarab_dict_remove(data, scarab_key);
//
//	scarab_free_datum(scarab_key);  
// Datum newData(removedVal);
//	scarab_free_datum(removedVal);
//  
//	return newData;
//}


void Datum::addElement(const Datum &value) {
  if(getDataType() != M_LIST) {
    fprintf(stderr, "mData::addElement(Datum value)\n"
	    "Can't add element in order in something other than M_LIST -- Type => %d\n",getDataType());
    return;
  }
  
  if(data->type != SCARAB_LIST){
	mwarning(M_SYSTEM_MESSAGE_DOMAIN,
		"Mismatched internal data type while setting a list element; should be %d, is %d",
		SCARAB_LIST, data->type);
		return;
  }
	
	int index = 0;
  // find the index of the last true element
	for(int i=getMaxElements()-1; i>=0; --i) {
		if(getElement(i).getDataType() != M_UNDEFINED) {
			index = i+1;
			break;
		}
	}
  
  setElement(index, value);
}
		
void Datum::setElement(int index, const Datum &value) {
 
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif
  
  // TODO: we could do something clever for M_DICTIONARY here
  if(getDataType() != M_LIST) {
    fprintf(stderr, "mData::setElement(int index, Datum value)\n"
	    "Can't set element in order in something other than M_LIST -- Type => %d\n",getDataType());
    #if INTERNALLY_LOCKED_MDATA
		unlock();
	#endif
	return;
  }
  
  if(data->type != SCARAB_LIST){
	mwarning(M_SYSTEM_MESSAGE_DOMAIN,
		"Mismatched internal data type while setting a list element; should be %d, is %d",
			SCARAB_LIST, data->type);
		#if INTERNALLY_LOCKED_MDATA
			unlock();
		#endif
		return;
  }

  int nelements = getMaxElements();
  
  // expand list if needed?
  if(nelements <= index) {
	  ScarabDatum *newList = (ScarabDatum *)scarab_list_new(index+1);
	  for(int i=0; i<data->data.list->size; ++i) {
		  if(scarab_list_get(data, i))
			  scarab_list_put(newList, i, scarab_list_get(data, i));
	  }
	  
	  scarab_free_datum(data);
	  data = newList;
	  
//    mwarning(M_SYSTEM_MESSAGE_DOMAIN,
//	     "Attempt to set an element of an Datum M_LIST beyond its edge");
//    // TODO expand list
//    #if INTERNALLY_LOCKED_MDATA
//	  unlock();
//	#endif
//	return;
  }
  
  
  ScarabDatum *item_to_remove = scarab_list_put(data, index,
						value.getScarabDatum()); // now doesn't want to be a copy
												 // reference automatically increments
						//value.getScarabDatumCopy());  // should be copy
  if(item_to_remove != NULL){
    scarab_free_datum(item_to_remove);
  }
  
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
  
}

Datum Datum::getElement(int index)  const{
  
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif
  
  //TODO: we could do something clever for M_DICTIONARY here
	if(getDataType() != M_LIST) {
		merror(M_SYSTEM_MESSAGE_DOMAIN, "Cannot get element by index: value is not a list");
    
	 Datum undefined;
		#if INTERNALLY_LOCKED_MDATA
			unlock();
		#endif
		return undefined;
	}
	
	
	
	if(data->type != SCARAB_LIST){
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			"Mismatched internal data type: attempting to access as list, but has type: %d",
			data->type);
	 Datum undefined;
		#if INTERNALLY_LOCKED_MDATA
			unlock();
		#endif
		return undefined;
	}
    
    if (index < 0 || index >= getMaxElements()) {
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Requested list index (%d) is out of bounds", index);
        
        Datum undefined;
#if INTERNALLY_LOCKED_MDATA
        unlock();
#endif
        return undefined;
    }

	if(data == NULL) {
		merror(M_SYSTEM_MESSAGE_DOMAIN,
			"Attempted to access a list with a NULL internal datum");
		//fprintf(stderr, "Requested index (%d) is larger than number of elements (%d)\n", index, getNElements());
    
	 Datum undefined;
		#if INTERNALLY_LOCKED_MDATA
			unlock();
		#endif
		return undefined;
	}

	ScarabDatum *datum = scarab_list_get(data, index);
	
	
 Datum newdata(datum);
	#if INTERNALLY_LOCKED_MDATA
		unlock();
	#endif
	return newdata;
}

vector<Datum> Datum::getElements() const {
	vector<Datum> elements;
	
	if(getDataType() != M_LIST) {
		merror(M_SYSTEM_MESSAGE_DOMAIN, "mData::getElements()\n"
			   "Can't getElements in something other than M_LIST -- Type => %d\n",getDataType());
	} else {
		for(int i=0; i<getNElements(); ++i) {
			elements.push_back(getElement(i));
		}
	}
	return elements;
}


// Private methods
void Datum::createDictionary(int is) {
	int initialsize = is;

    /* Scarab ensures that the hash table has at least one slot.  There's no reason to fuss about this.
	if(initialsize < 1) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Attempt to create an M_DICTIONARY with size less than 1...using size of 1");
		initialsize = 1;
	}
     */
	
	data = (ScarabDatum *)scarab_dict_new( initialsize,
										   &scarab_dict_times2);
	
	datatype = M_DICTIONARY;
}

void Datum::createList(int ls) {
	int size = ls;
	
    /* Scarab will handle an empty list correctly.  There's no reason to forbid it.
	if(size < 1) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Attempt to create an M_LIST with size less than 1...using size of 1");
		size = 1;
	}
     */
	
	data = (ScarabDatum *)scarab_list_new( size);
	/*for(int i = 0; i < size; i++){	// the list comes this way
		scarab_list_put(data, i, NULL);
	}*/
	
	datatype = M_LIST;
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
        case M_BOOLEAN:
            buf << (d.getBool() ? "true" : "false");
            break;
        case M_FLOAT:
            buf << d.getFloat();
            break;
        case M_STRING:
            buf << d.getString();
            break;
        case M_DICTIONARY: {
            buf << "{";
            const std::vector<Datum> keys = d.getKeys();
            for (int i = 0; i < keys.size(); i++) {
                if (i > 0) {
                    buf << ", ";
                }
                
                const Datum &key = keys[i];
                if (key.isString()) {
                    buf << key.getStringQuoted();
                } else {
                    buf << key;
                }
                
                buf << ": ";
                
                const Datum value = d.getElement(key);
                if (value.isString()) {
                    buf << value.getStringQuoted();
                } else {
                    buf << value;
                }
            }
            buf << "}";
            break;
        }
        case M_LIST: {
            buf << "[";
            const int numElements = d.getNElements();
            for (int i = 0; i < numElements; i++) {
                if (i > 0) {
                    buf << ", ";
                }
                const Datum item = d.getElement(i);
                if (item.isString()) {
                    buf << item.getStringQuoted();
                } else {
                    buf << item;
                }
            }
            buf << "]";
            break;
        }
        default:
            break;
    }
    
	return buf;
}


END_NAMESPACE_MW


























