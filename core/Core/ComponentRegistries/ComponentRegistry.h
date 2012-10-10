#ifndef COMPONENT_REGISTRY_H
#define COMPONENT_REGISTRY_H

/*
 *  ComponentRegistry.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 27 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 *
 *  The "mw::Component Registry" is basically just a list of service registries, organized
 *  by type.  So, there might be a Clock service registry, a state system service registry
 *  etc. etc. contained under the component registry, each of which describes and gives
 *  access to the registered components of each type (see ServiceRegistry.h)
 *
 *  Typically, one would go to the component registry to gain access to plugin-defined
 *  components, and the ill-named "service" registries just work under the hood.  The
 *  goal here is to give systematic, programmatic access to everything that gets added to 
 *  the system via plugins, all in one place.
 *
 *  The most important method gets the "Factory" object.  A "factory" object is just a
 *  a simple object that will furnish instances of the object in question, cast as their
 *  core-defined base class.  So, "myHighlySpecializedClock" plugin will provide an 
 *  ClockFactory object that produces instances of "myHighlySpecializedClock", cast as
 *  Clock's. The "factory object" formalism allows the core to instantiate user-defined
 *  classes that inherit from the core's base classes, without having to know anything 
 *  about the inherited class.
 */

#include "Stimulus.h"
#include "Clock.h"
#include "StateSystem.h"
#include "Utilities.h"
#include "ComponentFactory.h"
#include "GenericVariable.h"
#include "boost/shared_ptr.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/filesystem/path.hpp"
#include <map>
#include <boost/regex.hpp>
#include <boost/unordered_map.hpp>


namespace mw {
	class ComponentRegistry {
		
	protected:
		
		static shared_ptr<ComponentRegistry> shared_component_registry;
		
		// a map of object factories, indexed by type name (string)
		std::map< std::string, shared_ptr<ComponentFactory> > factories;
		
		// a map object instances, indexed by tag name (string)
        boost::unordered_map< std::string, shared_ptr<mw::Component> > instances;
		std::map< std::string, shared_ptr<StimulusNode> > stimulus_nodes;
		std::map< long, std::string > tagnames_by_id;
		
        // these regexes are cached at the object / member level to avoid
        // memory inefficiency at runtime
        boost::regex u1, strip_it;
            
        boost::unordered_map< std::string, shared_ptr<Variable> > variable_cache;
        boost::unordered_map< std::pair<std::string, GenericDataType>, shared_ptr<Datum> > data_cache;
    
	public:
		
		ComponentRegistry();
		
		static shared_ptr<ComponentRegistry> getSharedRegistry(){
			if(shared_component_registry == NULL){
				shared_ptr<ComponentRegistry> shared(new ComponentRegistry());
				shared_component_registry = shared;
			}
			
			return shared_component_registry;
		}
    
        static void detachSharedRegistryPtr(){
          shared_component_registry = shared_ptr<ComponentRegistry>();
        }
		
		void resetInstances(){
			instances.clear();
			stimulus_nodes.clear();
            variable_cache.clear();
            data_cache.clear();
            tagnames_by_id.clear();
		}
		
		// Factory-oriented methods
        
		void registerFactory(const std::string &type_name, shared_ptr<ComponentFactory> factory);

		void registerFactory(const std::string &type_name, ComponentFactory *factory) {
            registerFactory(type_name, shared_ptr<ComponentFactory>(factory));
        }
        
        template< template<typename> class FactoryTemplate, typename ComponentType >
        void registerFactory() {
            shared_ptr< FactoryTemplate<ComponentType> > factory(new FactoryTemplate<ComponentType>());
            registerFactory(factory->getComponentInfo().getSignature(), factory);
        }
        
        void registerFactoryAlias(const std::string &type_name, const std::string &alias_name) {
            registerFactory(alias_name, getFactory(type_name));
        }
        
        template<typename ComponentType>
        void registerFactoryAlias(const std::string &alias_name) {
            ComponentInfo info;
            ComponentType::describeComponent(info);
            registerFactoryAlias(info.getSignature(), alias_name);
        }
		
		shared_ptr<ComponentFactory> getFactory(const std::string &type_name);
		
		
		// Instance-oriented methods
		
		// Use a registered factory to create a new object and register the
		// result in the instances map
		shared_ptr<mw::Component>	registerNewObject(std::string tag_name, 
													  std::string type_name, 
													  std::map<std::string, std::string> params);
		
		shared_ptr<mw::Component>	createNewObject(const std::string &type_name, 
													const std::map<std::string, std::string> &params);
		
		
		void registerObject(std::string tag_name, shared_ptr<mw::Component> component, bool force = false);
		void registerAltObject(const std::string &tag_name, shared_ptr<mw::Component> component);
		
		// not sure about this one
		void registerStimulusNode(const std::string &tag_name, shared_ptr<StimulusNode> stimNode);
		// or this one
		shared_ptr<StimulusNode> getStimulusNode(std::string tag_name){
			return stimulus_nodes[tag_name];
		}
		
		// Straight instance look-ups
		// These will throw a ComponentTypeException if the type does not match 
		// and a ComponentUndefinedException if the tag doesn't refer to a defined
		// object
		
		template <class T>
		shared_ptr<T>	getObject(std::string tag_name, std::string parent_scope){
			
			// Split the parent scope so that we can search from inside out
			vector<string> scope_components;
			split(scope_components, parent_scope, boost::algorithm::is_any_of("/"));
			
			shared_ptr<T> candidate;
			
			for(int i = scope_components.size(); i >= 0; i--){
				string qualified_tag = "";
				// Try progressively broader scopes until we find a valid entry
				for(int j=0; j < i; j++){
					qualified_tag += scope_components[j] + "/";
				}
				
				qualified_tag += tag_name;
				
				candidate = getObject<T>(qualified_tag);
				if(candidate != NULL){
					return candidate;
				}
			}
			
			return candidate;
		}
		
        
		template <class T>
		shared_ptr<T>	getObject(std::string tag_name){
			shared_ptr<mw::Component> obj = instances[tag_name];
			
			
			if(obj == NULL){
				// do something
				return shared_ptr<T>();
			}
            
            if(obj->isAmbiguous()){
                //string string_rep = obj->getStringRepresentation();
                shared_ptr<AmbiguousComponentReference> amb_ref = 
                            boost::dynamic_pointer_cast<AmbiguousComponentReference, Component>(obj);
                throw AmbiguousComponentReferenceException(amb_ref);
            }
            
			return boost::dynamic_pointer_cast<T, mw::Component>(obj);
		}
		
		// Instance lookups with some extra parsing smarts
		shared_ptr<Variable>	getVariable(std::string expression);
		shared_ptr<Variable>	getVariable(std::string expression, 
											std::string default_expression);
		shared_ptr<StimulusNode>	getStimulus(std::string expression);
		
		// Utility look-ups to centralize commonly used parsing
		bool getBoolean(std::string expression);
        Datum getValue(std::string expression, GenericDataType type = M_UNDEFINED);
        Datum getNumber(std::string expression, GenericDataType type = M_FLOAT);
		boost::filesystem::path getPath(std::string working, 
										std::string expression);
        
        Datum getComponentCodec(){
			
			int dict_size = tagnames_by_id.empty() ? 1 : tagnames_by_id.size();
			
            Datum codec(M_DICTIONARY, dict_size);
			std::map< long, std::string>::const_iterator it;
			
			for(it = tagnames_by_id.begin(); it != tagnames_by_id.end(); it++){
				codec.addElement((*it).first, (*it).second);
			}
			
			return codec;
		}
		
	
        // a debugging routine. Dump the names of every registered object
        void dumpToStdErr();
            	
	};
	
    
    // TODO: Once existing code has been updated to use this typedef,
    // it should be redefined as boost::shared_ptr<ComponentRegistry>
    typedef ComponentRegistry* ComponentRegistryPtr;
    
	
}

#endif
