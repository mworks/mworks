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


class ComponentFactoryException : public SimpleException {

public:
	virtual ~ComponentFactoryException() throw() { }

	ComponentFactoryException(const std::string &refID, const std::string &message) :
        SimpleException(M_PARSER_MESSAGE_DOMAIN, message)
    {
        if (refID.empty()) {
            _referenceID = refID;
        } else {
            _referenceID = "<unknown>";
        }
	}
	
	const std::string& referenceID() const {
		return _referenceID;
	}

private:
	std::string _referenceID;
    
};


class MissingAttributeException : public ComponentFactoryException {
public:
	MissingAttributeException(const std::string &referenceID, const std::string &errorMessage) :
        ComponentFactoryException(referenceID, errorMessage)
    { }
};


class UnknownAttributeException : public ComponentFactoryException {
public:
    UnknownAttributeException(const std::string &referenceID, const std::string &attributeName) :
        ComponentFactoryException(referenceID, "Unknown attribute: \"" + attributeName + "\"")
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
	InvalidAttributeException(const std::string &referenceID, const std::string &name, const std::string &value) :
        ComponentFactoryException(referenceID, "invalid value (" + value + ") for attribute \"" + name + "\"")
    { }
};


class MissingReferenceException : public ComponentFactoryException {
public:
	MissingReferenceException(const std::string &referenceID, const std::string &name, const std::string &value) :
        ComponentFactoryException(referenceID,
                                  "can't find a reference for \"" + value + "\" for attribute \"" + name + "\"")
    { }
};


END_NAMESPACE_MW


#endif






















