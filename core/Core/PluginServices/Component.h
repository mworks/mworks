/*
 *  Component.h
 *  MWorksCore
 *
 *  Created by David Cox on 11/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef	M_COMPONENT_H_
#define M_COMPONENT_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>
#include <sstream>
#include <map>
#include "Utilities.h"
#include "Map.h"

using boost::shared_ptr;


namespace mw {
	
	class ComponentRegistry;  // forward decl
    class ComponentInfo;
    class ParameterValue;
	
	class Component : public boost::enable_shared_from_this<Component>{
		
	private:
		
		std::string tag;
		std::string reference_id;
        std::string object_signature;
		
		// this is a compact, unique identifier for the purpose
		// of event-sending.  It needs to be small because it might
		// be sent over the network many times per second
		long compact_id;
		
		static long _id_count;
        
    protected:
        template <typename T>
        shared_ptr<T> clone() {
            shared_ptr<T> new_component(new T);
            
            new_component->setTag(getTag());
            new_component->setReferenceID(getReferenceID());
            new_component->setObjectSignature(getObjectSignature());
            
            return new_component;
        }
		
	public:
        static const std::string TAG;
        
        static void describeComponent(ComponentInfo &info);
        
        explicit Component(const Map<ParameterValue> &parameters);
        explicit Component(const std::string &_tag = "", const std::string &_sig = "");
		
		virtual ~Component(){ }
		
		Component( const mw::Component& copy ){ 
			tag = copy.tag;
			reference_id = copy.reference_id;
		}
		
		virtual bool isAmbiguous() const { return false; }
		
        template <class T>
        shared_ptr<T> component_shared_from_this(){
            shared_ptr<mw::Component> shared = shared_from_this();
			shared_ptr<T> casted = boost::dynamic_pointer_cast<T, mw::Component>(shared);
            return casted;
        }
		
		void setTag(const std::string &_tag) { tag = _tag; }
		const std::string& getTag() const { return tag; }
		
		void setReferenceID(const std::string &ref_id) { reference_id = ref_id; }
		const std::string& getReferenceID() const { return reference_id; }
		
        void setObjectSignature(const std::string &_sig) { object_signature = _sig; }
        const std::string& getObjectSignature() const { return object_signature; }
        
		long getCompactID() const { return compact_id; }
		
		
		virtual shared_ptr<mw::Component> createInstanceObject(){
			return shared_from_this();
		}
		
		virtual void addChild(std::map<std::string, std::string> parameters,
							  ComponentRegistry *reg,
							  shared_ptr<mw::Component> child){
			// TODO: throw "can't add child" exception
			throw  SimpleException("Invalid attempt to use base-class addChild component method. This is likely a bug.");
		}
		
		virtual void finalize(std::map<std::string, std::string> parameters,
							  ComponentRegistry *reg){
			return;
		}
		
        virtual std::string getStringRepresentation() const {
            return getTag();
        }
        
	};
	
	
	class AmbiguousComponentReference : public mw::Component{
		
    protected:
		
		vector< shared_ptr<mw::Component> > ambiguous_components; // the list of components that all
		// share the same tag name
		
		
    public:
		
		virtual bool isAmbiguous() const {  return true; }
		
		void addAmbiguousComponent(shared_ptr<mw::Component> component_to_add){
			ambiguous_components.push_back(component_to_add);
		}
		
		const vector< shared_ptr<mw::Component> >& getAmbiguousComponents() const {
			return ambiguous_components;
		}
        
        virtual std::string getStringRepresentation() const;
		
	};
}
#endif