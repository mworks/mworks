/**
 * GenericData.h
 *
 * You need to be careful when copying dictionaries and lists.  The under-
 * lying objects are NOT copied, so modifying a dictionary if you are not the
 * only one using use scarabDeepCopy instead to create a new Data object.
 *
 *
 * Copyright (c) 2005 MIT. All rights reserved.
 */
#ifndef _GENERIC_DATA_H
#define _GENERIC_DATA_H

#include "MonkeyWorksTypes.h"
#include "ScarabServices.h"
#include "Lockable.h"
#include <string>
#include <vector>
#include "AnyScalar.h"

#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/basic_binary_oprimitive.hpp>
#include <boost/archive/basic_binary_iprimitive.hpp>
#include "Serialization.h"
namespace mw {
	
#define M_NO_DATA   0      // JJD added Aug 16, 2005
	
#define INTERNALLY_LOCKED_MDATA 0
	
	enum GenericDataType{M_UNDEFINED=0,M_INTEGER, M_FLOAT, M_BOOLEAN, M_STRING, 
	M_LIST, M_DICTIONARY, M_MAX_DATA_TYPE};
	
	
#if INTERNALLY_LOCKED_MDATA
	class Data : public Lockable{
#else
		class Data{
#endif	
			
		protected:   
			GenericDataType datatype;
			ScarabDatum *data;
			//mLockable *scarab_datum_lock;
			
			
			void initScarabDatum();
			
			void createDictionary(const int intialsize);
			void createList(const int size);
			
			void lockDatum() const;
			void unlockDatum() const;
			
		public:
			/**
			 * Constructors:  All constructors create a scarab object that can
			 * be sent over the network.  
			 */
			Data();
			Data(GenericDataType type, const int arg);
			Data(GenericDataType type, const double arg);
			Data(const long value);
			Data(const MonkeyWorksTime value);
			Data(const double value);
			Data(const float value);
			Data(const bool value);
			Data(const char * string);   
			Data(const std::string &string); 
			Data(const stx::AnyScalar);
			
			/**
			 * Overrides the default copy constructor to deeply copy
			 * the scarab datum.
			 */
			Data(const Data& that);
			
			/**
			 * Copies the scarabdatum, deeply, used for receiving data events
			 * on the network.
			 */
			Data(ScarabDatum * datum);
			
			/**
			 * Default Destructor.
			 */
			virtual ~Data();
			
			/**
			 * Returns the data type of the Data object.
			 */
			GenericDataType getDataType() const;
			void setDataType(const GenericDataType _type);
			
			/**
			 * Returns the scarab data package that stores this object
			 */
			ScarabDatum * getScarabDatum()  const;
			ScarabDatum * getScarabDatumCopy()  const;
			
			/**
			 * If datatype is one of the numeric types then it will return
			 * a representation for it, otherwise it will return 0.
			 */
			long getBool() const;
			double getFloat() const;
			long getInteger() const;
			
			/**
			 * Returns a string if Data is of type M_STRING, otherwise returns 0
			 * lenght of string is null terminated length or -1 otherwise
			 */
			const char *getString() const;
			int getStringLength() const;
			
			// debug function to print the datum.
			virtual void printToSTDERR();
			
			// Removes old data and creates a new datum with the new value.
			virtual void setBool(bool value);
			virtual void setFloat(double value);
			virtual void setInteger(long long value);
			virtual void setString(const char * string, int size);
			virtual void setString(std::string string);
			
			bool isInteger()  const;
			bool isFloat() const;
			bool isBool() const;
			bool isString() const;
			bool isDictionary() const;
			bool isList() const;
			bool isUndefined() const;
			bool isNumber() const;
			
			// this will overwrite the default asssignment operator.
			Data& operator=(const Data& that);
			
			virtual operator long() const;		
			virtual operator int() const;
			virtual operator short() const;
			virtual operator double() const;
			virtual operator float() const;
			virtual operator bool() const;
			virtual operator long long() const;
			virtual operator std::string() const;
			virtual operator stx::AnyScalar() const;
			
			//virtual void operator=(MonkeyWorksTime newdata);
			virtual void operator=(long newdata);
			virtual void operator=(int newdata);
			virtual void operator=(short newdata);
			virtual void operator=(bool newdata);
			virtual void operator=(double newdata);
			virtual void operator=(float newdata);
			virtual void operator=(const char * newdata);
			virtual void operator=(std::string newdata);
			virtual void operator=(stx::AnyScalar newdata);
			
			//void inc();
			// prefix operators.
			virtual void operator++();
			virtual void operator--();
			
			virtual bool operator==(long newdata) const;
			virtual bool operator==(double newdata) const;
			virtual bool operator==(bool newdata) const;
			virtual bool operator==(const char * newdata) const;
			virtual bool operator!=(const char * newdata) const;
			
			
			virtual Data operator+(const Data&) const;
			virtual Data operator-(const Data&) const;
			virtual Data operator*(const Data&) const;
			virtual Data operator/(const Data&) const;
			virtual Data operator%(const Data&) const;
			virtual Data operator==(const Data&) const;
			virtual Data operator!=(const Data&) const;
			virtual Data operator>(const Data&) const;
			virtual Data operator>=(const Data&) const;
			virtual Data operator<(const Data&) const;
			virtual Data operator<=(const Data&) const;
			
			
			
			int getNElements() const;
			int getMaxElements() const;
			
			bool hasKey(const Data &key) const;
			Data getKey(const int n)  const;
			std::vector<Data> getKeys() const;
			
			
			virtual void addElement(const Data &key, const Data &value);
			virtual void addElement(const Data &value);
			virtual void setElement(const int index, const Data &value);
			std::vector<Data> getElements() const;
			
			Data getElement(const Data &key) const;
			Data getElement(const int i) const;
			
			//  virtual Data removeElement(const char * key);
			
			// TODO: MUST IMPLEMENT THESE
			// virtual Data removeElement(const Data key);
			// TODO
			//  void removeElement(int index);
			
			
			virtual Data operator [](const int i);
			
			virtual std::string toString() const;
			
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
				std::string sval;
				
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
						sval = getString();
						ar << sval;
					case M_LIST:
					case M_DICTIONARY:
					default:
						// TODO: serialize lists and dicts
						break;
						
				} 
			}
			
			template<class Archive>
			void load(Archive &ar, const unsigned int version){
				
				long long ival;
				double fval;
				bool bval;
				std::string sval;
				
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
						ar >> sval;
						setString(sval);
					case M_LIST:
					case M_DICTIONARY:
					default:
						// TODO: serialize lists and dicts
						break;
						
				} 
			}
		};
		
		
		class ImmutableData : public Data {
			
		public:
			
			ImmutableData(const Data& d){
				setDataType(d.getDataType());
				data = d.getScarabDatumCopy();
			}
			
			// Overload empty methods for all mutators
			virtual void setBool(bool value) {  }
			virtual void setFloat(double value) { }
			virtual void setInteger(long long value) { }
			virtual void setString(const char * string, int size) { }
			virtual void setString(std::string string, int size) { }
			
			
			virtual void addElement(const char * key,Data value) { }
			virtual void addElement(Data value) { }
			virtual void setElement(int index, Data value) { }	
			
			Data removeElement(const char * key) { return Data(); }
			
		};
		
	}
#endif
	
