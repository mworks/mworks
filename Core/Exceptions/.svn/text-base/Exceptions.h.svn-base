/*
 *  Exceptions.h
 *  MonkeyWorksCore
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
namespace mw {
using namespace std;

class SimpleException : public std::exception {

	protected:
	
		string message;
		string subject;
		MessageDomain domain;
		
	public:

		SimpleException(string _message) : message(_message), subject(""){ 
			domain = M_GENERIC_MESSAGE_DOMAIN;
		};
		
		SimpleException(string _message, string _subject) : message(_message),
															 subject(_subject){ 
			domain = M_GENERIC_MESSAGE_DOMAIN;
		};
		
		SimpleException(MessageDomain _domain, string _message, string _subject = ""):
															message(_message),
															subject(_subject){
			domain = _domain;
		}
		
		SimpleException(const SimpleException& e){
			domain = e.domain;
			message = e.message;
			subject = e.subject;
		}
		
		virtual ~SimpleException() throw() {}
		
		string getMessage(){ return message; }
		string getSubject(){ return subject; }
		MessageDomain getDomain(){ return domain; }
		
		virtual const char* what() const throw(){
			if(subject.empty()){
				return message.c_str();
			} else {
				string returnstring = message + ": ";
				returnstring = returnstring + subject;
				return returnstring.c_str();
			}
		}

};

class NonFatalSimpleException : public SimpleException{ };


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
	
	InvalidXMLException(string _refid, string _message, string _subject = "") :
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
}

#endif

