/*
 *  ComponentFactoryException.h
 *  MWorksCore
 *
 *  Created by bkennedy on 12/3/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#ifndef COMPONENT_FACTORY_EXCEPTION_H_
#define COMPONENT_FACTORY_EXCEPTION_H_

#include "MWorksMacros.h"
#include "Exceptions.h"


BEGIN_NAMESPACE_MW


class ComponentFactoryException : public FatalParserException {

public:
    ComponentFactoryException(const std::string &message) :
        FatalParserException(message)
    { }

	ComponentFactoryException(const std::string &refID, const std::string &message) :
        FatalParserException(message)
    { }
    
};


class MissingAttributeException : public ComponentFactoryException {

public:
	MissingAttributeException(const std::string &referenceID, const std::string &errorMessage) :
        ComponentFactoryException(referenceID, errorMessage)
    { }
    
    MissingAttributeException(const std::string &attributeName) :
        ComponentFactoryException("Object is missing required attribute \"" + attributeName + "\"")
    { }

};


class UnknownAttributeException : public ComponentFactoryException {
public:
    UnknownAttributeException(const std::string &attributeName) :
        ComponentFactoryException("Unknown attribute: \"" + attributeName + "\"")
    { }
};


class InvalidReferenceException : public ComponentFactoryException {
public:
	InvalidReferenceException(const std::string &referenceID, const std::string &name, const std::string &value) :
        ComponentFactoryException(referenceID, "invalid value \"" + value + "\" for attribute \"" + name + "\"")
    { }
};


class InvalidAttributeException : public ComponentFactoryException {
    
public:
	InvalidAttributeException(const std::string &name, const std::string &value) :
        ComponentFactoryException("invalid value (" + value + ") for attribute \"" + name + "\"")
    { }
    
    InvalidAttributeException(const std::string &referenceID, const std::string &name, const std::string &value) :
        InvalidAttributeException(name, value)
    { }
    
};


class MissingReferenceException : public ComponentFactoryException {
    
public:
	MissingReferenceException(const std::string &name, const std::string &value) :
        ComponentFactoryException("can't find a reference for \"" + value + "\" for attribute \"" + name + "\"")
    { }
    
    MissingReferenceException(const std::string &referenceID, const std::string &name, const std::string &value) :
        MissingReferenceException(name, value)
    { }
    
};


END_NAMESPACE_MW


#endif






















