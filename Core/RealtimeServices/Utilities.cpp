/**
 * Utilities.cpp
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 04/05/05 - Change message functions to create distinct
 *      message types.  
 * Paul Jankunas on 05/23/05 - Added message functions to place default
 *      text into different types of messages.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#include "Utilities.h"
#include "EventBuffer.h"
#include "StandardVariables.h"
#include <string>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>


#include "Timer.h"
using namespace mw;


namespace mw {
	// system messages are going to be limited to 1024 characters.

#define MSG_BUFFER_SIZE 1024
	MessageOrigin GlobalMessageOrigin = M_UNKNOWN_MESSAGE_ORIGIN;
	
	bool egg_timer_initialized = false;
	Timer *egg_timer;
	
	static bool timer_has_expired;
	void *expireTimer(void *);
	
	void _makeString(const std::string &format, va_list ap, 
					 MessageType type, 
					 MessageDomain domain = M_GENERIC_MESSAGE_DOMAIN) {
		char buffer[MSG_BUFFER_SIZE];// = { '\0' };
		vsnprintf(buffer, MSG_BUFFER_SIZE-1, format.c_str(), ap);
		
		
		Data messageDatum=Data(M_DICTIONARY, 4);
		messageDatum.addElement(M_MESSAGE_DOMAIN, Data(M_INTEGER, domain));
		messageDatum.addElement(M_MESSAGE, Data(std::string(buffer)));
		messageDatum.addElement(M_MESSAGE_TYPE, Data(M_INTEGER, type));
		messageDatum.addElement(M_MESSAGE_ORIGIN, Data(M_INTEGER, GlobalMessageOrigin));
		if(GlobalMessageVariable != 0) {
			if(type == M_GENERIC_MESSAGE){
				GlobalMessageVariable->setValue(messageDatum);
			} else {
#ifndef	SILENCE_WARNINGS_MODE
				GlobalMessageVariable->setValue(messageDatum);
#endif
			}
		}	
	}
	
	
	// allow a simplified no-domain cal for an mprintf
	void mprintf(const char *format, ...) {
		va_list ap;
		va_start(ap, format);
		_makeString(format, ap, M_GENERIC_MESSAGE); 
		va_end(ap);
	}
	
	void mprintf(MessageDomain dom, const char *format, ...) {
		va_list ap;
		va_start(ap, format);
		_makeString(format, ap, M_GENERIC_MESSAGE, dom);       
		va_end(ap);
	}
	
	// because swig is stupid and requires it to be called this way
	void parserwarning(const char *format, ...) {
		
		va_list ap;
		va_start(ap, format);
		
		std::string pre("WARNING: " + std::string(format));
		
		_makeString(pre, ap, M_WARNING_MESSAGE, M_PARSER_MESSAGE_DOMAIN);
		va_end(ap);
	}
	
	void mwarning(MessageDomain dom, const char *format, ...) {
		
		va_list ap;
		va_start(ap, format);
		
		std::string pre("WARNING: " + std::string(format));
		
		_makeString(pre, ap, M_WARNING_MESSAGE);
		va_end(ap);
	}
	
	// for swig
	void parsererror(const char *format, ...) {
		va_list ap;
		va_start(ap, format);
		std::string pre("ERROR: " + std::string(format));
		_makeString(pre, ap, M_ERROR_MESSAGE, M_PARSER_MESSAGE_DOMAIN);
		va_end(ap);
	}
	
	void merror(MessageDomain dom, const char *format, ...) {
		va_list ap;
		va_start(ap, format);
		std::string pre("ERROR: " + std::string(format));
		_makeString(pre, ap, M_ERROR_MESSAGE, dom);
		va_end(ap);
	}
	
	
	void mfatal_error(MessageDomain dom, const char *format, ...) {
		va_list ap;
		va_start(ap, format);
		std::string pre("FATAL ERROR: " + std::string(format));
		_makeString(pre, ap, M_FATAL_ERROR_MESSAGE, dom);
		va_end(ap);
	}
	
	void mnetwork(const char * format, ...) {
		va_list ap;
		va_start(ap, format);
		std::string pre("NETWORK: " + std::string(format));
		_makeString(pre, ap, M_GENERIC_MESSAGE, M_NETWORK_MESSAGE_DOMAIN);
		va_end(ap);
	}
	
	void mgeneric_printf(int type, const char *format, ...) {
		va_list ap;
		va_start(ap, format);
		_makeString(format, ap, (MessageType)type);
		va_end(ap);
	}
	
	void mdebug(const char* format, ...) {
#if MONKEYWORKS_DEBUG_MODE
		va_list ap;
		va_start(ap, format);
		char buffer[MSG_BUFFER_SIZE];// = { '\0' };
		vsnprintf(buffer, MSG_BUFFER_SIZE-1, format, ap);
		va_end(ap);
		fprintf(stderr, "MDEBUG: %s\n", buffer); fflush(stderr);
#endif
	}
	
	void tick() {
		
	}
	
	long tock() {
		return 0L;
	}
	
	void startEggTimer(MonkeyWorksTime howlongus) {
		if(!egg_timer_initialized){
			egg_timer = new Timer();
			egg_timer_initialized = true;
		}
		
		//mprintf("setting egg timer for %lld us", howlongus);
		
		egg_timer->startUS(howlongus);
		
		//timer_has_expired  = false;
		//schedule(howlongms, 0, 1, expireTimer, NULL);
	}
	
	bool hasTimerExpired() {
		return egg_timer->hasExpired();
		//return timer_has_expired;
	}
	
	
	void *expireTimer(void *) {
		timer_has_expired = true;
		return NULL;
	}
	
}

