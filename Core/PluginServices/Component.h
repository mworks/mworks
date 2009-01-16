/*
 *  Component.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 11/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef	M_COMPONENT_H_
#define M_COMPONENT_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <iostream>
#include <map>
#include "Utilities.h"
namespace mw {
	using namespace boost;
	
	class mwComponentRegistry;  // forward decl
	
	class Component {
		
	protected:
		
		std::string tag;
		std::string reference_id;
		
		// this is a compact, unique identifier for the purpose
		// of event-sending.  It needs to be small because it might
		// be sent over the network many times per second
		long compact_id;
		
		weak_ptr<mw::Component> self_ptr;
		
		static long _id_count;
		
	public:
		
		Component(){ 
			compact_id = mw::Component::_id_count++;
		}
		Component(std::string _tag){  
			tag = _tag;  
			compact_id = mw::Component::_id_count++;
		}
		
		virtual ~Component(){ }
		
		Component( const mw::Component& copy ){ 
			tag = copy.tag;
			reference_id = copy.reference_id;
		}
		
		virtual bool isAmbiguous(){ return false; }
		
		virtual void setSelfPtr(weak_ptr<mw::Component> ptr){ self_ptr = ptr; }
		
		template <class T>
		weak_ptr<T> getSelfPtr(){
			shared_ptr<mw::Component> shared(self_ptr);
			shared_ptr<T> casted = dynamic_pointer_cast<T, mw::Component>(shared);
			weak_ptr<T> weakened(casted);
			return weakened;
		}
		
		virtual void setTag(std::string _tag){ tag = _tag; }
		virtual std::string getTag(){	return tag;  }
		
		void setReferenceID(std::string ref_id){  reference_id = ref_id; }
		std::string getReferenceID(){  return reference_id;  }
		
		long getCompactID(){  return compact_id;  }
		
		
		virtual shared_ptr<mw::Component> createInstanceObject(){
			mprintf("Using base createInstanceObject()");
			shared_ptr<mw::Component> self_ptr(getSelfPtr<mw::Component>());
			return self_ptr;
		}
		
		virtual void addChild(std::map<std::string, std::string> parameters,
							  mwComponentRegistry *reg,
							  shared_ptr<mw::Component> child){
			// TODO: throw "can't add child" exception
			throw  SimpleException("Attempt to use base-class addChild component method");
		}
		
		virtual void finalize(std::map<std::string, std::string> parameters,
							  mwComponentRegistry *reg){
			return;
		}
		
	};
	
	
	class AmbiguousComponentReference : public mw::Component{
		
    protected:
		
		vector< shared_ptr<mw::Component> > ambiguous_components; // the list of components that all
		// share the same tag name
		
		
    public:
		
		virtual bool isAmbiguous(){  return true; }
		
		virtual void addAmbiguousComponent(shared_ptr<mw::Component> component_to_add){
			ambiguous_components.push_back(component_to_add);
		}
		
		virtual vector< shared_ptr<mw::Component> > getAmbiguousComponents(){
			return ambiguous_components;
		}
		
	};
}
#endif