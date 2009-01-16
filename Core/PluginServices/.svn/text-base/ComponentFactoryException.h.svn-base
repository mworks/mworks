/*
 *  ComponentFactoryException.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 12/3/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#ifndef COMPONENT_FACTORY_EXCEPTION_H_
#define COMPONENT_FACTORY_EXCEPTION_H_
namespace mw {
class ComponentFactoryException : public std::exception {
protected:
	std::string _what;
	std::string _referenceID;
public:
	ComponentFactoryException(const std::string &refID) {
		_referenceID = refID;
	}
	
	virtual ~ComponentFactoryException() throw() {}
	virtual const char* what() const throw() = 0;	
	
	virtual const char* referenceID() const throw() {
		return _referenceID.c_str();
	}
};

class MissingAttributeException : public ComponentFactoryException {
public:
	MissingAttributeException(const std::string &referenceID,
							   const std::string &errorMessage) : ComponentFactoryException(referenceID) {
		_what = errorMessage;
	}
	
	virtual ~MissingAttributeException() throw() {}
	
	virtual const char* what() const throw() {
		return _what.c_str();
	}
};

class InvalidReferenceException : public ComponentFactoryException {
public:
	InvalidReferenceException(const std::string &referenceID,
							   const std::string &name,
							   const std::string &value) : ComponentFactoryException(referenceID) {
		// WTF? This is totally not how this is used
		//_what = "invalid reference for \"" + value + "\" of type \"" + name + "\"";
		_what = "invalid value \"" + value + "\" for attribute \"" + name + "\"";
	}
	
	virtual ~InvalidReferenceException() throw() {}
	
	virtual const char* what() const throw() {
		return _what.c_str();
	}
};

class InvalidAttributeException : public ComponentFactoryException {
public:
	InvalidAttributeException(const std::string &referenceID,
							   const std::string &name,
							   const std::string &value) : ComponentFactoryException(referenceID) {
		_what = "invalid value (" + value + ") for attribute \"" + name + "\"";
	}
	
	virtual ~InvalidAttributeException() throw() {}
	
	virtual const char* what() const throw() {
		return _what.c_str();
	}
};

class MissingReferenceException : public ComponentFactoryException {
protected:
public:
	MissingReferenceException(const std::string &referenceID,
							   const std::string &name,
							   const std::string &value) : ComponentFactoryException(referenceID) {
		_what = "can't find a reference for \"" + value + "\" for attribute \"" + name + "\"";
	}
	
	virtual ~MissingReferenceException() throw() {}
	
	virtual const char* what() const throw() {
		return _what.c_str();
	}
};
}

#endif
