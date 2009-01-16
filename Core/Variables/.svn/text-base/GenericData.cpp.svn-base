/**
 * GenericData.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 01/28/05 - Added constructor to create Data objects from
 *                      ScarabDatums used in network transfers. Fixed spacing
 *
 * Paul Jankunas on 4/06/05 - Changing the way scarab objects are generated.
 *
 * Copyright (c) 2005 MIT. All rights reserved.
 */

// TEMP
#include "EmbeddedPerlInterpreter.h"
// ENDTEMP


#include "GenericData.h"
#include "Utilities.h"
#include "ScarabServices.h"
#include <string>
#include <sstream>
using namespace mw;

using namespace std;

#include "ExpressionParser.h"

void Data::initScarabDatum(){
  // TODO, this could cause a shitload of memory leaks
  // when setting a new Data it null's the ScarabDatum pointer and then writes a new one
  //  it never deletes the old one
  //  DDC: not really... doesn't get called except in constructors
  //       we should phase it out nonetheless
  data = NULL;
}

Data::Data() {
  initScarabDatum();
  //setInteger(-1);
  setDataType(M_UNDEFINED);
}


Data::Data(GenericDataType type, const int arg) {
  
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

Data::Data(GenericDataType type, const double arg) {
  
  
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

Data::Data(const double newdata) {
  initScarabDatum();
  setDataType(M_FLOAT);
  setFloat(newdata);
}


Data::Data(const float newdata) {
  initScarabDatum();
  setDataType(M_FLOAT);
  setFloat(newdata);
}


Data::Data(const long newdata){
  initScarabDatum();
  setDataType(M_INTEGER);
  setInteger(newdata);
}

Data::Data(const MonkeyWorksTime newdata){
  initScarabDatum();
  setDataType(M_INTEGER);
  setInteger(newdata);
}

Data::Data(const char * string) {
  initScarabDatum();
  datatype = M_STRING;
  setString(string, strlen(string)+1);
}

Data::Data(const std::string &string){
  initScarabDatum();
  datatype = M_STRING;
  setString(string.c_str(), string.size()+1);
}

Data::Data(bool newdata) {
  initScarabDatum();
  setDataType(M_BOOLEAN);
  setBool(newdata);
}

Data::Data(const Data& that) {
  
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  
  data = 0;

  if(that.getDataType() == M_UNDEFINED) {
	setDataType(M_UNDEFINED);      
  } else {
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

Data::Data(ScarabDatum * datum) {  
  
  data = 0;
  
  if(datum == NULL){
    //mwarning( M_SYSTEM_MESSAGE_DOMAIN,
	//      "Attempt to create an Data object from a NULL ScarabDatum");
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
  case SCARAB_FLOAT_OPAQUE:
	//       Dave: How did we end up with all of this bullshit anyways?
	//       Ben:  I don't know, so I just deleted it.
	//       <Ben skulks away and fixes his bugs>
	
	// TODO: part of the HAXXOR .. I don't like any of it and it will be 
	// immenently fixed.  Right after the nightly regressions get completed
    
	// DDC: meta-kludge to compensate for the platform-dependence of this
	//		most loathesome of hacks 
	#if __LITTLE_ENDIAN__
		double d = *((double *)(datum->data.opaque.data));
	#else
		char swap_bytes[sizeof(double)];
		char *double_bytes = (char *)(datum->data.opaque.data);
		for(unsigned int i = 0; i < sizeof(double); i++){
			swap_bytes[i] = double_bytes[sizeof(double) - i - 1];
		}
		
		double d = *((double *)swap_bytes);
	#endif
	
    setDataType(M_FLOAT);
    setFloat(d);
    return; // I don't like this return here, 
            // but it will go away when this is fixed

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

Data::Data(stx::AnyScalar newdata){
	initScarabDatum();
	
	operator=(newdata);	
}



Data::~Data() {
  scarab_free_datum(data);
  data = NULL;
}


void Data::lockDatum()  const{
	ScarabDatum *theDatum = getScarabDatum();
	scarab_lock_datum(theDatum);
}

void Data::unlockDatum()  const{
	ScarabDatum *theDatum = getScarabDatum();
	scarab_unlock_datum(theDatum);
}

GenericDataType Data::getDataType() const {
  return datatype;
}

void Data::setDataType(const GenericDataType type){
  datatype = type;
}

ScarabDatum * Data::getScarabDatum() const{ 
  return data;
}

ScarabDatum *Data::getScarabDatumCopy() const{
  ScarabDatum *new_datum = scarab_copy_datum(data);
  
  return new_datum;
}

long Data::getBool() const{
	
	if(data == NULL){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Attempt to access a broken Data object");
		return false;
	}

	bool result = false;
  
	switch (datatype) {
		case M_INTEGER:
		case M_BOOLEAN:
			result = (data->data.integer > 0);
			break;
		case M_FLOAT:
			result = (data->data.floatp > 0);
			break;
		default:
			break;
	}      
	
	return result;      
}

double Data::getFloat() const {
	
	#if INTERNALLY_LOCKED_MDATA
		lock();
	#endif;

	if(data == NULL){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Attempt to access a broken Data object");
		#if INTERNALLY_LOCKED_MDATA
			unlock();
		#endif
			return 0.0;
	}
  
	double result = 0;
  
	switch (datatype) {
		case M_INTEGER:
		case M_BOOLEAN:
			result = (double)(data->data.integer);
			break;
		case M_FLOAT:
			result = data->data.floatp;
			break;
		default:
			result = 0.0;
			break;
	}
  
	#if INTERNALLY_LOCKED_MDATA
		unlock();
	#endif
	return result;
}

long Data::getInteger() const{
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  
  if(data == NULL){
    mwarning(M_SYSTEM_MESSAGE_DOMAIN,
	     "Attempt to access a broken Data object");
    #if INTERNALLY_LOCKED_MDATA
		unlock();
	#endif
	return 0;
  }
  
  long result = 0;
  
  switch (datatype) {
  case M_INTEGER:
  case M_BOOLEAN:
    result = data->data.integer;
	break;
  case M_FLOAT:
    result = (long)(data->data.floatp);
	break;
  default:                
    result = 0;
	break;
  }
  
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
  return result;
}

const char * Data::getString() const{
  
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  
  const char *result;
  
  switch (datatype) {
  case M_STRING:
    result = (const char *)data->data.opaque.data;
    break;
  default:                
    result = NULL;
    break;
  }
  
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
  return result;
}

int Data::getStringLength()  const{
  
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  
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

void Data::setBool(bool newdata) {
  
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  
  datatype = M_BOOLEAN;
  if(data != NULL){
    scarab_free_datum(data);
  }
  data = scarab_new_integer( newdata);
  
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
}

void Data::setInteger(long long newdata) {
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  datatype = M_INTEGER;
  if(data != NULL){
    scarab_free_datum(data);
  }
  data = scarab_new_integer( newdata );
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
}

void Data::setFloat(double newdata) {
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  datatype = M_FLOAT;
  
  if(data != NULL){
    scarab_free_datum(data);
  }
  data = scarab_new_float( newdata);
  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
}


void Data::setString(const char * newdata, int size) {
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  datatype = M_STRING;

  if(data != NULL){
	scarab_free_datum(data); // TODO: why isn't this safe?
  }

  //int size = strlen(newdata) + 1;
  
  char *string = new char[size];
  memcpy(string, newdata, size);
  
  data = scarab_new_opaque(string, size);

  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
    
  delete[] string;
  
}


void Data::setString(std::string newdata) {
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  datatype = M_STRING;

  if(data != NULL){
	scarab_free_datum(data); // TODO: why isn't this safe?
  }

  data = scarab_new_opaque(newdata.c_str(), newdata.length()+1);


  #if INTERNALLY_LOCKED_MDATA
	unlock();
  #endif
  
}


bool Data::isInteger()  const{ return (datatype == M_INTEGER); }

bool Data::isBool()  const{ return (datatype == M_BOOLEAN); }
    
bool Data::isFloat()  const{ return (datatype == M_FLOAT); }
	
bool Data::isString()  const{ return (datatype == M_STRING); }

bool Data::isDictionary()  const{ return (datatype == M_DICTIONARY); }

bool Data::isList() const { return (datatype == M_LIST); }

bool Data::isUndefined() const { return (datatype == M_UNDEFINED || data == NULL); }

bool Data::isNumber() const {
	return (!isUndefined() && (isFloat() || isInteger() || isBool()));
}

Data& Data::operator=(const Data& that) {

  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  
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

void Data::operator=(long newdata) {
  setInteger(newdata);
}

void Data::operator=(int newdata) {
  setInteger(newdata);
}

void Data::operator=(short newdata) {
  setInteger(newdata);
}

void Data::operator=(bool newdata) {
  setBool(newdata);
}

void Data::operator=(double newdata) {
  setFloat(newdata);
}

void Data::operator=(float newdata) {
  setFloat(newdata);
}

void Data::operator=(const char * newdata) {
  setString(std::string(newdata));
}

void Data::operator=(std::string newdata){
  setString(newdata);
}

void Data::operator=(stx::AnyScalar newdata){
	stx::AnyScalar::attrtype_t type = newdata.getType();
	switch(type){
		case stx::AnyScalar::ATTRTYPE_BOOL:
			setBool(newdata.getBoolean());
			break;
		case stx::AnyScalar::ATTRTYPE_INTEGER:
		case stx::AnyScalar::ATTRTYPE_LONG:
		case stx::AnyScalar::ATTRTYPE_SHORT:
		case stx::AnyScalar::ATTRTYPE_WORD:
		case stx::AnyScalar::ATTRTYPE_DWORD:
		case stx::AnyScalar::ATTRTYPE_QWORD:
			setInteger(newdata.getLong());
			break;
		case stx::AnyScalar::ATTRTYPE_CHAR:
		case stx::AnyScalar::ATTRTYPE_BYTE:
			setInteger(newdata.getULong());
			break;
		case stx::AnyScalar::ATTRTYPE_FLOAT:
		case stx::AnyScalar::ATTRTYPE_DOUBLE:
			setFloat(newdata.getDouble());
			break;
		case stx::AnyScalar::ATTRTYPE_STRING:
			setString(newdata.getString());
			break;
		default:
			break;
	}
}


void Data::operator++() {
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

void Data::operator--() {
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

bool Data::operator==(long newdata)  const {
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


bool Data::operator==(const char * newdata)  const {
  switch(datatype) {
  case M_STRING:
    int eq = strncmp(newdata, getString(), getStringLength()); 
    return (eq == 0);
    break;
  default:
    mprintf("Operation == is undefined on this data type");
    return false; // to shut up the compiler
  } 
}

bool Data::operator!=(const char * newdata) const {
  switch(datatype) {
  case M_STRING:
    int eq = strncmp(newdata, getString(), getStringLength()); 
    return (eq != 0);
    break;
  default:
    mprintf("Operation != is undefined on this data type");
    return false; // to shut up the compiler
  } 
}

bool Data::operator==(double newdata) const {
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

bool Data::operator==(bool newdata) const {
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


Data::operator long() const{
  return getInteger();
}

Data::operator long long() const{
  return getInteger();
}

Data::operator int() const{
  return getInteger();
}

Data::operator short() const{
  return getInteger();
}

Data::operator double() const{
  return getFloat();
}

Data::operator float() const{
  return getFloat();
}

Data::operator bool() const{
  return getBool();
}

Data::operator std::string() const {
  std::string returnval = getString();
  return returnval;
}

Data::operator stx::AnyScalar() const {
	if(isInteger()){
		return stx::AnyScalar(getInteger());
	} else if(isFloat()){
		return stx::AnyScalar(getFloat());
	} else if(isString()){
		return stx::AnyScalar(getString());
	} else if(isBool()){
		return stx::AnyScalar(getBool());
	}
	
	return stx::AnyScalar();
}



Data Data::operator+(const Data& other)  const{
  Data returnval;
  if(isInteger() || isBool()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getInteger() + (long)other;
    } else if(other.isFloat()) {
      returnval = getInteger() + (double)other;
    }
  } else if(isFloat()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getFloat() + (long)other;
    } else if(other.isFloat()) {
      returnval = getFloat() + (double)other;
    }
  } else {
    returnval = 0;
  }
  return returnval;
}

Data Data::operator-(const Data& other)  const{
  Data returnval;
  if(isInteger() || isBool()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getInteger() - (long)other;
    } else if(other.isFloat()) {
      returnval = getInteger() - (double)other;
    }
  } else if(isFloat()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getFloat() - (long)other;
    } else if(other.isFloat()) { 
      returnval = getFloat() - (double)other;
    }
  } else {
    returnval = 0;
  }
  return returnval;
}

Data Data::operator*(const Data& other)  const{
  Data returnval;
  if(isInteger() || isBool()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getInteger() * (long)other;
    } else if(other.isFloat()) {
      returnval = getInteger() * (double)other;
    }
  } else if(isFloat()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getFloat() * (long)other;
    } else if(other.isFloat()) {
      returnval = getFloat() * (double)other;
    }
  } else {
    returnval = 0;
  }
  return returnval;
}

Data Data::operator/(const Data& other)  const{
  Data returnval;
  if(isInteger() || isBool()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getInteger() / (long)other;
    } else if(other.isFloat()) {
      returnval = getInteger() / (double)other;
    }
  } else if(isFloat()){
    if(other.isInteger() || other.isBool()) {
      returnval = getFloat() / (long)other;
    } else if(other.isFloat()) {
      returnval = getFloat() / (double)other;
    }
  } else {
    returnval = 0;
  }
  return returnval;
}

Data Data::operator%(const Data& other)  const{ 
  Data returnval;
  if(isInteger() || isBool()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getInteger() % (long)other;
    } else if(other.isFloat()) {
      returnval = getInteger() % (long)other;
    }
  } else if(isFloat()){
    if(other.isInteger() || other.isBool()) {
      returnval = getInteger() % (long)other;
    } else if(other.isFloat()) {
      returnval = getInteger() % (long)other;
    }
  } else {
    returnval = 0;
  }
  return returnval;
}

Data Data::operator==(const Data& other)  const{
	if(isInteger() || isBool()) {
		if(other.isInteger() || other.isBool()) {
			return Data(M_BOOLEAN, (getInteger() == (long)other));
		} else if(other.isFloat()) {
			return Data(M_BOOLEAN, (getInteger() == (double)other));
		} else {
			return Data(M_BOOLEAN, false);
		}
	} else if(isFloat()) {
		if(other.isInteger() || other.isBool()) {
			return Data(M_BOOLEAN, ( getFloat() == (long)other));
		} else if(other.isFloat()) {
			return Data(M_BOOLEAN, ( getFloat() == (double)other));
		} else {
			return Data(M_BOOLEAN, false);
		}
	} else if(isString()) {
		if(other.isString())
			return Data(M_BOOLEAN, (other == getString()));
		else
			return Data(M_BOOLEAN, false);
	} else if(isDictionary()) {
		if(other.isDictionary() && (getNElements() == other.getNElements())) {
			for(int i=0; i<getMaxElements(); i++) {
				Data key(getKey(i));
				
				if(!key.isUndefined()) {
					Data val1, val2;
					val1 = getElement(key);
					val2 = other.getElement(key);

					if(val1 != val2) {
						return Data(M_BOOLEAN, false);
					}
				}
			}
			return Data(M_BOOLEAN, true);
		} else {
			return Data(M_BOOLEAN, false);
		}
	} else if(isList()) {
		if(other.isList() && (getMaxElements() == other.getMaxElements())) {
			for(int i=0; i<getMaxElements(); i++) {
				Data val1, val2;
				val1 = getElement(i);
				val2 = other.getElement(i);

				if(val1 != val2) {
					return Data(M_BOOLEAN, false);
				}		
			}
			return Data(M_BOOLEAN, true);      
		} else {
			return Data(M_BOOLEAN, false);
		}
	} else if(isUndefined() && other.isUndefined()) {
		return Data(M_BOOLEAN, true);
	} else {
		return Data(M_BOOLEAN, false);
	}
}

Data Data::operator!=(const Data& d1)  const{
	if(isUndefined()) {
		if(d1.isUndefined())
			return Data(M_BOOLEAN, false);
		else 
			return Data(M_BOOLEAN, true);
	} else {
		Data d2(getScarabDatum()); // DDC: was leaking

		if(d1 == d2)
			return Data(M_BOOLEAN, false);
		else
			return Data(M_BOOLEAN, true);
	}
}

Data Data::operator>(const Data& other)  const{
  Data returnval;
  if(isInteger() || isBool()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getInteger() > (long)other;
    } else if(other.isFloat()) {
      returnval = getInteger() > (double)other;
    }
  } else if(isFloat()){
    if(other.isInteger() || other.isBool()) {
      returnval = getFloat() > (long)other;
    } else if(other.isFloat()) {
      returnval = getFloat() > (double)other;
    }
  } else {
    returnval = 0;
  }
  return returnval;
}

Data Data::operator>=(const Data& other)  const{
  Data returnval;
  if(isInteger() || isBool()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getInteger() >= (long)other;
    } else if(other.isFloat()) {
      returnval = getInteger() >= (double)other;
    }
  } else if(isFloat()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getFloat() >= (long)other;
    } else if(other.isFloat()) {
      returnval = getFloat() >= (double)other;
    }
  } else {
    returnval = 0;
  }
  return returnval;
}

Data Data::operator<(const Data& other)  const{
  Data returnval;
  if(isInteger() || isBool()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getInteger() < (long)other;
    } else if(other.isFloat()) {
      returnval = getInteger() < (double)other;
    }
  } else if(isFloat()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getFloat() < (long)other;
    } else if(other.isFloat()) {
      returnval = getFloat() < (double)other;
    }
  } else {
    returnval = 0;
  }
  return returnval;
}

Data Data::operator<=(const Data& other)  const{
  Data returnval;
  if(isInteger() || isBool()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getInteger() <= (long)other;
    } else if(other.isFloat()) {
      returnval = getInteger() <= (double)other;
    }
  } else if(isFloat()) {
    if(other.isInteger() || other.isBool()) {
      returnval = getFloat() <= (long)other;
    } else if(other.isFloat()) {
      returnval = getFloat() <= (double)other;
    }
  } else {
    returnval = 0;
  }
  return returnval;
}

Data Data::operator[](int i) {
  if(getDataType() != M_LIST) {
    fprintf(stderr, "mData is not of type M_LIST -- Type => %d\n", getDataType());

	Data undefined;
    return undefined;
  }
	
  return getElement(i);
}

void Data::printToSTDERR() {
	fprintf(stderr, "Datatype => %d\n", datatype);
	switch (datatype) {
		case M_INTEGER:
			fprintf(stderr, "Value is => %ld\n", getInteger());
			break;
		case M_BOOLEAN:
			fprintf(stderr, "Value is => %ld\n", getBool());
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


void Data::addElement(const Data &key,  const Data &value) {  
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


bool Data::hasKey(const Data &key)  const{
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

Data Data::getElement(const Data &key)  const{

  
  if(getDataType() != M_DICTIONARY) {
    fprintf(stderr, "Can't search for elements with keys in something other than M_DICTIONARY -- Type => %d\n", getDataType());
    Data undefined;
    return undefined;
  }

  ScarabDatum *datum = scarab_dict_get(data, key.getScarabDatum());
  return Data(datum);
}

int Data::getNElements() const {
	int returnval = -1;
	
	switch(getDataType()) {
		case M_INTEGER:
		case M_FLOAT:
		case M_BOOLEAN:
		case M_STRING:
			returnval =  1;
			break;
		case M_DICTIONARY:	
			returnval = scarab_dict_number_of_elements(data);    
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

int Data::getMaxElements()  const{
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

Data Data::getKey(const int n)  const {

	if(getDataType() != M_DICTIONARY) {
		fprintf(stderr, "Can't get keys in something other than M_DICTIONARY -- Type => %d\n", getDataType());
		Data undef;
		return undef;
	}
  
	if(data->type != SCARAB_DICT){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Mismatched internal data type while setting a dictionary element; should be %d, is %d",
				 data->type, SCARAB_DICT);
		Data undef;
		return undef;
	}
  
	lockDatum();
	ScarabDatum ** sd = scarab_dict_keys(data);

	int n_keys = data->data.dict->tablesize;
  
	if(n < 0 || n >= n_keys){
		fprintf(stderr, "Key index (%d) is larger than number of keys (%d)\n", n, n_keys);
		Data undef;
		unlockDatum();
		return undef;
	}
  
	ScarabDatum *key = sd[n];
	
	Data returnval(key);
	unlockDatum();

	return returnval; 
}

std::vector<Data> Data::getKeys() const {
	std::vector<Data> keys;
	
	if(getDataType() != M_DICTIONARY) {
		merror(M_SYSTEM_MESSAGE_DOMAIN, "attempting to get keys in something that's not a dictionary");
		return keys;
	}
	
	lockDatum();
	ScarabDatum ** sd = scarab_dict_keys(data);
	
	int n_keys = data->data.dict->tablesize;
	
	for(int i = 0; i<n_keys; ++i) {
		keys.push_back(sd[i]);
	}
	
	unlockDatum();
	
	return keys;
}

//mData Data::removeElement(const char * key) {
//	if(getDataType() != M_DICTIONARY) {
//		fprintf(stderr, "mData::removeElement(const char *)\n"
//				"Can't remove element using  keys in something other than M_DICTIONARY -- Type => %d\n",
//				getDataType());
//		Data undef;
//		return undef;
//	}
//  
//	ScarabDatum * scarab_key = scarab_new_string(key);
//
//	ScarabDatum *removedVal = scarab_dict_remove(data, scarab_key);
//
//	scarab_free_datum(scarab_key);  
//	Data newData(removedVal);
//	scarab_free_datum(removedVal);
//  
//	return newData;
//}


void Data::addElement(const Data &value) {
  if(getDataType() != M_LIST) {
    fprintf(stderr, "mData::addElement(Data value)\n"
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
		
void Data::setElement(const int index, const Data &value) {
 
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif
  
  // TODO: we could do something clever for M_DICTIONARY here
  if(getDataType() != M_LIST) {
    fprintf(stderr, "mData::setElement(int index, Data value)\n"
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
//	     "Attempt to set an element of an Data M_LIST beyond its edge");
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

Data Data::getElement(const int index)  const{
  
  #if INTERNALLY_LOCKED_MDATA
	lock();
  #endif;
  
  //TODO: we could do something clever for M_DICTIONARY here
	if(getDataType() != M_LIST) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN, "mData::getElement(int index)\n"
			"Can't get element by index in something other than M_LIST -- Type => %d\n",getDataType());
		//fprintf(stderr, "mData::getElement(int index)\n"
		//	"Can't get element by index in something other than M_LIST -- Type => %d\n",getDataType());
    
		Data undefined;
		#if INTERNALLY_LOCKED_MDATA
			unlock();
		#endif
		return undefined;
	}
	
	
	
	if(data->type != SCARAB_LIST){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Mismatched internal data type: attempting to access as list, but has type: %d",
			data->type);
		Data undefined;
		#if INTERNALLY_LOCKED_MDATA
			unlock();
		#endif
		return undefined;
	}

	if(index >= getMaxElements()) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN, 
			"Requested index (%d) is larger than number of elements (%d)\n", 
			index, getNElements());
		//fprintf(stderr, "Requested index (%d) is larger than number of elements (%d)\n", index, getNElements());
    
		Data undefined;
		#if INTERNALLY_LOCKED_MDATA
			unlock();
		#endif
		return undefined;
	}

	if(data == NULL) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN, 
			"Attempted to access a list with a NULL internal datum");
		//fprintf(stderr, "Requested index (%d) is larger than number of elements (%d)\n", index, getNElements());
    
		Data undefined;
		#if INTERNALLY_LOCKED_MDATA
			unlock();
		#endif
		return undefined;
	}

	ScarabDatum *datum = scarab_list_get(data, index);
	
	
	Data newdata(datum);
	#if INTERNALLY_LOCKED_MDATA
		unlock();
	#endif
	return newdata;
}

vector<Data> Data::getElements() const {
	vector<Data> elements;
	
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
void Data::createDictionary(const int is) {
	int initialsize = is;
	if(initialsize < 1) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Attempt to create an M_DICTIONARY with size less than 1...using size of 1");
		initialsize = 1;
	}
	
	;
	data = (ScarabDatum *)scarab_dict_new( initialsize,
										   &scarab_dict_times2);
	
	datatype = M_DICTIONARY;
}

void Data::createList(const int ls) {
	int size = ls;
	
	if(size < 1) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
				 "Attempt to create an M_LIST with size less than 1...using size of 1");
		size = 1;
	}
	
	
	data = (ScarabDatum *)scarab_list_new( size);
	/*for(int i = 0; i < size; i++){	// the list comes this way
		scarab_list_put(data, i, NULL);
	}*/
	
	datatype = M_LIST;
}

std::string Data::toString() const {
	std::ostringstream buf;
	
	switch (datatype) {
		case M_INTEGER:
			buf << getInteger();
			break;
		case M_BOOLEAN:
			buf << getBool();
			break;
		case M_FLOAT:
			buf << getFloat();
			break;
		case M_STRING:
			buf << getString();
			break;    
		case M_DICTIONARY:
			buf << "DICT";
			break;
		case M_LIST:
			buf << "LIST";
			break;
		default:
			buf << "";
			break;
	}
	
	return buf.str();
}



