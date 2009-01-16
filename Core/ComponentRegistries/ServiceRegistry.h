#ifndef SERVICE_REGISTRY_H
#define SERVICE_REGISTRY_H

/*
 *  ServiceRegistry.h
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 27 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 *  "Service" here basically means "user-defined object," with the idea that 
 *  the new object ("service") defines additional functionality.  Probably a bad/
 *  confusing name.  Maybe we can change it at some point.  Until then, get off my back.
 *
 *  A service registry is a listing of plugin-defined components of a particular type 
 *  (e.g. "mClock" or "mAction").  Some skeletal methods and objects for defining 
 *  dependencies (i.e. "myHighlySpecializedStateSystem" depends upon 
 *  "myHighlySpecializedClock"), but the implementation is absent/thin.
 *
 *  There are numerous fields in the service description that tell you the who/what/
 *  where of the plugin, and basic methods for asking for particular services.
 *  
 *  The most important method gets the "Factory" object.  A "factory" object is just a
 *  a simple object that will furnish instances of the object in question, cast as their
 *  core-defined base class.  So, "myHighlySpecializedClock" plugin will provide an 
 *  ClockFactory object that produces instances of "myHighlySpecializedClock", cast as
 *  Clock's. The "factory object" formalism allows the core to instantiate user-defined
 *  classes that inherit from the core's base classes, without having to know anything 
 *  about the inherited class.
 */

#include "ObjectFactory.h"
#include "ExpandableList.h"




enum DependencyLevel{ M_CRITICAL, M_WARNING, M_PREFERENCE };

class ServiceDependency {

    protected:
    
        ServiceType type;
        char *tagname;
        
        char *creatoremail, *identifierstring, *versiongreaterthan, *versionlessthan;
        
        DependencyLevel level;


    public:
    
        ServiceDependency(char *_tagname, ServiceType _type, DependencyLevel _level){
            tagname = _tagname;
            type = _type;
            level = _level;
        }
        
        void requireIdentifier(char *_identifierstring){
            identifierstring = _identifierstring;
        }
        
        void requireVersionGreaterThan(char *greaterthan){
            versiongreaterthan = greaterthan;
        }
        
        void requireVersionLessThan(char *lessthan){
            versionlessthan = lessthan;
        }
        
        void requireCreator(char *creator){
            creatoremail = creator;
        }
        
        bool hasVersionDependency(){
            return(versiongreaterthan || versionlessthan);
        }
        
        char *getVersionGreaterThan(){
            return versiongreaterthan;
        }
        
        char *getVersionLessThan(){
            return versionlessthan;
        }
        
        bool hasIdentifierDependency(){
            if(identifierstring){
                return true;
            } 
            
            return false;
        }
        
        char *getIdentifierString(){
            return identifierstring;
        }
        
        bool hasCreatorDependency(){
            if(creatoremail){
                return true;
            }
            
            return false;
        }
        
        char *getCreatorEmail(){
            return creatoremail;
        }
        
        DependencyLevel getLevel(){
            return level;
        }
};

class ServiceDescription {

    protected:

        // Basic info that is critical for functioning
        ServiceType type;  // type of component/service
        std::string tagname; // string containing tag name
        ObjectFactory *factory; // object which spits out a service object (this is the 
                                // base class
        
        // Dependencies
        ExpandableList<ServiceDependency> *dependencies;
        
        // Optional information
        
        // from plugin property list and file
        char *versionstring;
        char *identifierstring;
        char *frompluginpath;
        
        // convenience info
        char *creatoremail; // email address of creator


        // Structure built up to allow a tag to be multiply defined.  Resolution of which one to use
        // can be automatic or manual
        bool multiply_defined;
        ExpandableList<ServiceDescription> *multiple_definitions;
    
    public:
    
    
        ServiceDescription(std::string _tagname, 
							ObjectFactory *_factory, 
							ServiceType _type){
            tagname = _tagname;
            factory = _factory;
            type = _type;
            //critical = _critical;
            dependencies = new ExpandableList<ServiceDependency>();
        }
        
        
        std::string getTagName(){
            return tagname;
        }
        
        void setTagName(std::string newtag){
            //todo: free old tag?
            tagname = newtag;
        }
        
        ObjectFactory *getFactory(){
            return factory;
        }
        
        void setFactory(ObjectFactory *newfactory){
            factory = newfactory;
        }
        
        ServiceType getType(){
            return type;
        }
        
        void setType(ServiceType newtype){
            type = newtype;
        }
        
        void addDependency(shared_ptr<ServiceDependency> newdependency){
            dependencies->addElement(newdependency);
        }
        
        ExpandableList<ServiceDependency> *getDependencies(){
            return dependencies;
        }
        
        char *getVersionString(){
            return versionstring;
        }
        
        void setVersionString(char *newversion){
            versionstring = newversion;
        }

        
        char *getIdentifierString(){
            return identifierstring;
        }
        
        void setIdentifierString(char *newidentifier){
            identifierstring = newidentifier;
        }

        char *getPluginPath(){
            return frompluginpath;
        }
        
        void setPluginPath(char *newpath){
            frompluginpath = newpath;
        }


        char *getCreatorEmail(){
            return creatoremail;
        }


        void setCreatorEmail(char *newcreator){
            creatoremail = newcreator;
        }


        bool isMultiplyDefined(){
            return multiply_defined;
        }
        
        void multiplyDefine(shared_ptr<ServiceDescription> newdescription){
            if(multiply_defined){ // already more than one description
                multiple_definitions->addElement(newdescription);
            } else {
            
                //mServiceDescription *thisdescription = new ServiceDescription(this;

                multiple_definitions = new ExpandableList<ServiceDescription>();
                multiply_defined = true;
                multiple_definitions->addElement(shared_ptr<ServiceDescription>(this)); // TODO
                multiple_definitions->addElement(newdescription);
            }
        }
        
        ExpandableList<ServiceDescription> *getMultipleDefinitions(){
            return multiple_definitions;
        }
        
                
};



// Base class for service registries.  Registry objects allow for objects and services
// to be registered by name.  This allows plugins to register new functionality and for
// an experiment script, xml file, or compiled definition bundle to refer to this
// functionality by name.
//
// Contains an expandable dictionary of name/factory object pairs

class ServiceRegistry{

    ExpandableList<ServiceDescription> services;
    ExpandableList<ServiceDescription> unresolved;

    public:
    
        ServiceRegistry();
    
        void registerService(std::string tag, 
							 ObjectFactory *factory, 
							 ServiceType type);  
							       
        void registerService(shared_ptr<ServiceDescription> description);        
        
        void registerUnresolvedService(std::string tag);
        void registerUnresolved(std::string tag, ServiceType type);
    
        ObjectFactory *getFactory(std::string matchtag);
        
        
        bool hasService(std::string matchtag);
        bool hasUnresolvedService(std::string matchtag);
        bool hasService(std::string matchtag, 
						ExpandableList<ServiceDescription> *list);
        
        int getNServices();
        
        shared_ptr<ServiceDescription> getService(int i);
};



#endif

