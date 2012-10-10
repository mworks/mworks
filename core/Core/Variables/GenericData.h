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
#include "AnyScalar.h"

#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/version.hpp>
#include <boost/archive/binary_oarchive.hpp>
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
	
	enum GenericDataType{M_UNDEFINED=0,M_INTEGER, M_FLOAT, M_BOOLEAN, M_STRING, 
	M_LIST, M_DICTIONARY, M_MAX_DATA_TYPE};
	
	
#if INTERNALLY_LOCKED_MDATA
	class Datum : public Lockable{
#else
    class Datum{
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
            Datum();
            Datum(GenericDataType type, const int arg);
            Datum(GenericDataType type, const double arg);
            Datum(const long value);
            Datum(const MWTime value);
            Datum(const double value);
            Datum(const float value);
            Datum(const bool value);
            Datum(const char * string);   
            Datum(const std::string &string); 
            Datum(const stx::AnyScalar);
			
        
        Datum( map<int, string> dict){
                
                int size = dict.size();
                createDictionary(size);
                
                map<int,string>::iterator i;
                for(i = dict.begin(); i != dict.end(); ++i){
                    
                    pair<int, string> key_val = *i;
                    Datum key((long)key_val.first);
                    Datum val((string)key_val.second);
                    
                    this->addElement(key, val);
                }
            }
        

            Datum(map<string, int> dict){
                
                int size = dict.size();
                createDictionary(size);
                
                map<string,int>::iterator i;
                for(i = dict.begin(); i != dict.end(); ++i){
                    
                    pair<string, int> key_val = *i;
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
			virtual ~Datum();
			
			/**
			 * Returns the data type of the Datum object.
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
			 * Returns a string if Datum is of type M_STRING, otherwise returns 0
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
            Datum& operator=(const Datum& that);
			
			virtual operator long() const;		
			virtual operator int() const;
			virtual operator short() const;
			virtual operator double() const;
			virtual operator float() const;
			virtual operator bool() const;
			virtual operator long long() const;
			virtual operator std::string() const;
			virtual operator stx::AnyScalar() const;
			
			//virtual void operator=(MWTime newdata);
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
			
			
			virtual Datum operator+(const Datum&) const;
			virtual Datum operator-(const Datum&) const;
			virtual Datum operator*(const Datum&) const;
			virtual Datum operator/(const Datum&) const;
			virtual Datum operator%(const Datum&) const;
			virtual Datum operator==(const Datum&) const;
			virtual Datum operator!=(const Datum&) const;
			virtual Datum operator>(const Datum&) const;
			virtual Datum operator>=(const Datum&) const;
			virtual Datum operator<(const Datum&) const;
			virtual Datum operator<=(const Datum&) const;
			
			
			
			int getNElements() const;
			int getMaxElements() const;
			
			bool hasKey(const Datum &key) const;
			std::vector<Datum> getKeys() const;
			
			
			virtual void addElement(const Datum &key, const Datum &value);
			virtual void addElement(const Datum &value);
			virtual void setElement(const int index, const Datum &value);
			std::vector<Datum> getElements() const;
			
            Datum getElement(const Datum &key) const;
            Datum getElement(const int i) const;
			
			//  virtual Datum removeElement(const char * key);
			
			// TODO: MUST IMPLEMENT THESE
			// virtual Datum removeElement(const Datum key);
			// TODO
			//  void removeElement(int index);
			
			
			virtual Datum operator [](const int i);
			
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
						sval = getString();
                        string_length = getStringLength();
                        ar << string_length;
                        /*std::cerr << "serializing: ";
                        for(int i = 0; i < string_length; i++){
                            std::cerr << " " << sval[i] << " ";
                            ar << sval[i];
                        }
                        std::cerr << std::endl;*/
                        ar << sval;
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
				std::string sval;
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
                        /*sval = "";
                        std::cerr << "deserializing: ";
                        for(int i = 0; i < string_length; i++){
                            ar >> c;
                            sval += c;
                            std::cerr << " " << c << " ";
                        }
                         std::cerr << std::endl;*/
                        
                        ar >> sval;
                        setString(sval);
                        
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
		
		
		class ImmutableData : public Datum {
			
		public:
			
			ImmutableData(const Datum& d){
				setDataType(d.getDataType());
				data = d.getScarabDatumCopy();
			}
			
			// Overload empty methods for all mutators
			virtual void setBool(bool value) {  }
			virtual void setFloat(double value) { }
			virtual void setInteger(long long value) { }
			virtual void setString(const char * string, int size) { }
			virtual void setString(std::string string, int size) { }
			
			
			virtual void addElement(const char * key, Datum value) { }
			virtual void addElement(Datum value) { }
			virtual void setElement(int index, Datum value) { }	
			
		 Datum removeElement(const char * key) { return Datum(); }
			
		};


END_NAMESPACE_MW


#endif
