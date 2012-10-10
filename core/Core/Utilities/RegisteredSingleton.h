/*
 *  RegsiteredSingleton.h
 *  MWorksCore
 *
 *  Created by bkennedy on 10/31/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef REGISTER_SINGLETON_H_
#define REGISTER_SINGLETON_H_

// DDC: 9/09:  Normally, I'd say using macros for this kind of thing is Wrong with a capital "W".
//      However, the alternative (and original version) uses inherited, templated static members
//      which results in crazy syntax and unstable behavior across gcc versions.
//      Basically, I think the issue of inherited+templated static membership is a sufficiently
//      dark corner of C++ as to be worth avoiding

#include "boost/shared_ptr.hpp"
#include "Exceptions.h"
#include "Component.h"

#define REGISTERED_SINGLETON_CODE_INJECTION(TYPE)     SINGLETON_INSTANCE_STATIC_MEMBER(TYPE)  \
                                                      SINGLETON_INSTANCE_METHOD(TYPE) \
                                                      SINGLETON_REGISTERINSTANCE_METHOD(TYPE) \
                                                      SINGLETON_DESTROY_METHOD(TYPE)

#define SINGLETON_INSTANCE_STATIC_DECLARATION(TYPE)   boost::shared_ptr<TYPE> TYPE::singleton_instance;
#define SINGLETON_INSTANCE_STATIC_MEMBER(TYPE)   static boost::shared_ptr<TYPE> singleton_instance;
#define SINGLETON_INSTANCE_METHOD(TYPE)          static  \
    boost::shared_ptr<TYPE> instance(const bool should_throw=true){\
      if(should_throw && singleton_instance == NULL){ \
        throw SimpleException("trying to access singleton instance before it is registered"); \
      } \
      return singleton_instance; \
    }


#define SINGLETON_REGISTERINSTANCE_METHOD(TYPE)  	static \
    void registerInstance(const boost::shared_ptr<mw::Component> &an_instance) { \
      if(an_instance == NULL) { \
        throw SimpleException("attempting to register NULL instance for a singleton");	 \
      }		\
      shared_ptr<TYPE> as_type = boost::dynamic_pointer_cast<TYPE, mw::Component>(an_instance); \
      if(as_type == NULL) { \
        throw SimpleException("attempting to register singleton instance for wrong type of object");	\
      } \
      registerInstance(as_type);  \
    }\
    static void registerInstance(const boost::shared_ptr<TYPE> &an_instance){\
      if(an_instance == NULL) { \
        throw SimpleException("attempting to register NULL instance for a singleton");	 \
      }		\
      if(singleton_instance != NULL) { \
        throw SimpleException("attempting to register singleton instance when there's already an instance"); \
      } \
      singleton_instance = an_instance; \
    }

#define  SINGLETON_DESTROY_METHOD(TYPE)   static \
    void destroy() {  \
      singleton_instance = boost::shared_ptr<TYPE>(); \
    } 




//#include "boost/shared_ptr.hpp"
//#include "Exceptions.h"
//#include "Component.h"
//namespace mw {
//template <class T> class RegisteredSingleton {
//protected:
//	static boost::shared_ptr<T> singleton_instance;
//public:
//	static boost::shared_ptr<T> instance(const bool should_throw=true) {		
//		if(singleton_instance == NULL && should_throw) {
//			throw SimpleException("trying to access singleton instance before it is registered");
//		}
//		
//		return singleton_instance;
//	}
	
//	static void registerInstance(const boost::shared_ptr<mw::Component> &an_instance) {
//		
//		if(an_instance == NULL) {
//			throw SimpleException("attempting to register NULL instance for a singleton")	;	
//		}		
//		
//    // DDC: 9/09, relaxing for now
//		//if(singleton_instance != NULL) {
//		//	throw SimpleException("attempting to register singleton instance when there's already an instance");			
//		//}
//
//		singleton_instance = boost::dynamic_pointer_cast<T, mw::Component>(an_instance);
//
//		if(singleton_instance == NULL) {
//			throw SimpleException("attempting to register singleton instance for wrong type of object");			
//		}
//	}
//
//	static void destroy() {		
//    if(singleton_instance == NULL) {
//			throw SimpleException("trying to destroy singleton when there isn't one registered");
//		}
//		
//		singleton_instance = boost::shared_ptr<T>();
//	}
//};
//
//template <class T> boost::shared_ptr<T> RegisteredSingleton<T> ::singleton_instance;
//	
//}

#endif
