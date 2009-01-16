/*
 *  RegsiteredSingleton.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 10/31/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "boost/shared_ptr.hpp"
#include "Exceptions.h"
#include "Component.h"
namespace mw {
template <class T> class RegisteredSingleton {
protected:
	static boost::shared_ptr<T> singleton_instance;
public:
	static boost::shared_ptr<T> instance(const bool should_throw=true) {		
		if(singleton_instance == 0 && should_throw) {
			throw SimpleException("trying to access singleton instance before it is registered");
		}
		
		return singleton_instance;
	}
	
	static void registerInstance(const boost::shared_ptr<mw::Component> &an_instance) {
		
		if(an_instance == 0) {
			throw SimpleException("attempting to register NULL instance for a singleton")	;	
		}		
		
		if(singleton_instance != 0) {
			throw SimpleException("attempting to register singleton instance when there's already an instance");			
		}

		singleton_instance = dynamic_pointer_cast<T, mw::Component>(an_instance);

		if(singleton_instance == 0) {
			throw SimpleException("attempting to register singleton instance for wrong type of object");			
		}
	}
	
	static void destroy() {
		if(singleton_instance == 0) {
			throw SimpleException("trying to destroy singleton when there isn't one registered");
		}
		
		singleton_instance = boost::shared_ptr<T>();
	}
};

template <class T> boost::shared_ptr<T> RegisteredSingleton<T> ::singleton_instance;
	
}
