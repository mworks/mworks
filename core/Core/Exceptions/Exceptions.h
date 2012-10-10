/*
 *  Exceptions.h
 *  MWorksCore
 *
 *  Created by David Cox on 1/27/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef M_EXCEPTIONS_H_
#define M_EXCEPTIONS_H_

#include <boost/format.hpp>
#include <string>
#include <exception>
#include "EventConstants.h"
#include <map>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/exception/all.hpp>
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <map>
#include <string>

using boost::error_info;
using boost::shared_ptr;
using boost::variant;
using std::string;



namespace mw {

// forward decls
class Stimulus;
class Variable;
class StimulusGroup;
class Component;

// fwd dec
class Component;
class AmbiguousComponentReference;

class SimpleException : public std::exception, public boost::exception {

	protected:
	
		string message;
		//string subject;
		MessageDomain domain;
		
	public:

		SimpleException(string _message) : message(_message){ 
			domain = M_GENERIC_MESSAGE_DOMAIN;
		};
		
		SimpleException(string _message, string _subject) : message(_message + ": " + _subject){ 
			domain = M_GENERIC_MESSAGE_DOMAIN;
		};

        SimpleException(MessageDomain _domain, string _message):
                                                            message(_message){
            domain = _domain;
        }
		
		SimpleException(MessageDomain _domain, string _message, string _subject):
															message(_message + ": " + _subject){
			domain = _domain;
		}
		
		//SimpleException(const SimpleException& e){
//			domain = e.domain;
//			message = e.message;
//		}
		
		virtual ~SimpleException() throw() {}
		
		virtual string getMessage(){ return message; }
		//string getSubject(){ return subject; }
		MessageDomain getDomain(){ return domain; }
    
        void setMessage(const std::string &newMessage) {
            message = newMessage;
        }
		
		virtual const char* what() const throw(){
            return message.c_str();
		}

};

class NonFatalParserException : public SimpleException{ 

public:
        
    NonFatalParserException(string _subject):SimpleException(M_PARSER_MESSAGE_DOMAIN, _subject){}
    NonFatalParserException(string _message, string _subject):SimpleException(M_PARSER_MESSAGE_DOMAIN, _message, _subject){}

};


class MalformedXMLException : public SimpleException{ 

	public:
	
	MalformedXMLException(string _message) 
							: SimpleException(M_PARSER_MESSAGE_DOMAIN, _message){ 
	};
};

class InvalidXMLException : public SimpleException{

	protected:
	
	string reference_id;

	
	public:
	
	InvalidXMLException(string _refid, string _message) :
        SimpleException(M_PARSER_MESSAGE_DOMAIN, _message),
        reference_id(_refid)
    { }
	
	InvalidXMLException(string _refid, string _message, string _subject) :
				SimpleException(M_PARSER_MESSAGE_DOMAIN, _message, _subject){
					
		reference_id = _refid;
	}
	
	InvalidXMLException(string _refid, const SimpleException& e) :
				SimpleException(e){
		reference_id = _refid;
	}
	
	virtual ~InvalidXMLException() throw() {}
	
	string getReferenceID(){  return reference_id;  }
};
    

class AmbiguousComponentReferenceException : public SimpleException {

protected:

    shared_ptr<AmbiguousComponentReference> component_reference;

public:
    AmbiguousComponentReferenceException(shared_ptr<AmbiguousComponentReference> ref);
    virtual ~AmbiguousComponentReferenceException() throw() {}


    shared_ptr<AmbiguousComponentReference> getAmbiguousComponentReference(){ return component_reference; }
    
};


class FatalParserException : public SimpleException {

public:
    FatalParserException(string _subject) : SimpleException(M_PARSER_MESSAGE_DOMAIN, _subject){ }
    
    FatalParserException(string _msg, string _subject) : SimpleException(M_PARSER_MESSAGE_DOMAIN, _msg, _subject){ }
    
    FatalParserException() : SimpleException(M_PARSER_MESSAGE_DOMAIN, "A fatal error occurred"){ }
    
    virtual ~FatalParserException() throw(){ }
    
    
};


class UnknownExpressionException : public SimpleException {
public:
    UnknownExpressionException(string _subject) : SimpleException(M_PARSER_MESSAGE_DOMAIN, "Invalid expression", _subject){ }
};

class UnknownVariableException : public SimpleException {
public:
    UnknownVariableException(string _subject) : SimpleException(M_PARSER_MESSAGE_DOMAIN, "Unknown variable", _subject){ }
};



class ComponentFactoryConflictException : public SimpleException{
  
    
public:
    
    ComponentFactoryConflictException(string _subject) : 
        SimpleException(M_PLUGIN_MESSAGE_DOMAIN, "A plugin has attempted to redefine the behavior of an existing component signature", _subject){
        }
    
    virtual ~ComponentFactoryConflictException() throw() {}
};
    
    
// Common error info payloads (can be added to exception objects
// above via operator<<

#define ERROR_INFO_TOKEN(TOKEN)         struct TOKEN ## _error_info_token
#define ERROR_INFO_STRING(TOKEN)        TOKEN ## _error_info_string
#define ERROR_INFO_OBJECT(TOKEN)        TOKEN ## _error_info

#define ERROR_INFO_DECLARATION(TOKEN, TYPE)                                         \
ERROR_INFO_TOKEN(TOKEN);                                                            \
typedef error_info< ERROR_INFO_TOKEN(TOKEN), TYPE >  ERROR_INFO_OBJECT(TOKEN);    


namespace error_info_types{
    typedef  variant<shared_ptr<Component>, string>     ComponentOrString;
    typedef  variant<shared_ptr<Stimulus>, string>      StimulusOrString;
    typedef  variant<shared_ptr<StimulusGroup>, string> StimulusGroupOrString;
    typedef  variant<shared_ptr<Variable>, string>      VariableOrString;
    typedef  variant< string >                          ErrorString;
    typedef  variant< std::map<string, string> , string>     ComponentAttributeMap;
    typedef  variant< int, double >                     ErrorNumber;
}

ERROR_INFO_DECLARATION( parent_component,       error_info_types::ComponentOrString );
ERROR_INFO_DECLARATION( child_component,        error_info_types::ComponentOrString );
ERROR_INFO_DECLARATION( component,              error_info_types::ComponentOrString );
ERROR_INFO_DECLARATION( object_type,            error_info_types::ErrorString);
ERROR_INFO_DECLARATION( parent_scope,           error_info_types::ErrorString);
ERROR_INFO_DECLARATION( ref_id,                 error_info_types::ErrorString);
ERROR_INFO_DECLARATION( stimulus,               error_info_types::StimulusOrString );
ERROR_INFO_DECLARATION( stimulus_group,         error_info_types::StimulusGroupOrString );
ERROR_INFO_DECLARATION( variable,               error_info_types::VariableOrString);
ERROR_INFO_DECLARATION( expression,             error_info_types::ErrorString);
ERROR_INFO_DECLARATION( subexpression,          error_info_types::ErrorString);
ERROR_INFO_DECLARATION( xml_code,               error_info_types::ErrorString);
ERROR_INFO_DECLARATION( component_attributes,   error_info_types::ComponentAttributeMap );
ERROR_INFO_DECLARATION( line_number,            error_info_types::ErrorNumber);
ERROR_INFO_DECLARATION( col_number,             error_info_types::ErrorNumber);
ERROR_INFO_DECLARATION( reason,         error_info_types::ErrorString);
ERROR_INFO_DECLARATION( additional_msg,         error_info_types::ErrorString);
ERROR_INFO_DECLARATION( parser_context,         error_info_types::ErrorString);


extern void display_extended_error_information(SimpleException& e);

// A "visitor" class for dealing with the above boost::variant objects
class to_string_visitor : public boost::static_visitor<string> {
public:

    string operator()(shared_ptr<mw::Component> operand) const;
    string operator()(shared_ptr<mw::Stimulus> operand) const;    
    string operator()(shared_ptr<mw::StimulusGroup> operand) const;
    string operator()(shared_ptr<mw::Variable> operand) const;
    string operator()(string operand) const {
        string return_string = operand;
        return return_string;
    }
};

}

#endif

