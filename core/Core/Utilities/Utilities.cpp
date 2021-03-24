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
#include "StateSystem.h"
#include <string>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include <boost/scope_exit.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>


BEGIN_NAMESPACE_MW


	MessageOrigin GlobalMessageOrigin = M_UNKNOWN_MESSAGE_ORIGIN;


	static void _makeString(const std::string &format, va_list ap,
                            MessageType type,
                            MessageDomain domain = M_GENERIC_MESSAGE_DOMAIN)
    {
        // Allocate the mutex on the heap to ensure that it's never destructed.  This prevents potential crashes
        // at application shutdown, caused by a thread trying to acquire the mutex after it's been destructed.
        static boost::mutex * const globalMessageVariableMutex = new boost::mutex;
        
        char *buffer = nullptr;
        BOOST_SCOPE_EXIT(&buffer) {
            free(buffer);
        } BOOST_SCOPE_EXIT_END
        
        if (-1 == vasprintf(&buffer, format.c_str(), ap)) {
            return;
        }
        
        // If the message is a warning or an error, append line number information (if available)
        if (type >= M_WARNING_MESSAGE) {
            boost::shared_ptr<StateSystem> stateSystem = StateSystem::instance(false);
            boost::shared_ptr<State> currentState;
            if (stateSystem && (currentState = stateSystem->getCurrentState().lock())) {
                auto &currentLocation = currentState->getLocation();
                if (!currentLocation.empty()) {
                    char *oldBuffer = buffer;
                    buffer = nullptr;
                    BOOST_SCOPE_EXIT(&oldBuffer) {
                        free(oldBuffer);
                    } BOOST_SCOPE_EXIT_END
                    
                    if (-1 == asprintf(&buffer, "%s [%s]", oldBuffer, currentLocation.c_str())) {
                        return;
                    }
                }
            }
        }
		
        // Make a copy of GlobalMessageVariable to ensure that it isn't replaced while we're using it.  This can
        // happen when an experiment is being unloaded, and various threads are shutting down and trying to issue
        // messages, while another thread is re-initializing the standard variables.
		if (auto messageVar = GlobalMessageVariable) {
            Datum messageDatum(M_DICTIONARY, 4);
            messageDatum.addElement(M_MESSAGE_DOMAIN, Datum(M_INTEGER, domain));
            messageDatum.addElement(M_MESSAGE, Datum(buffer));
            messageDatum.addElement(M_MESSAGE_TYPE, Datum(M_INTEGER, type));
            messageDatum.addElement(M_MESSAGE_ORIGIN, Datum(M_INTEGER, GlobalMessageOrigin));
            
            // Use a mutex to ensure that the set and reset happen atomically
            boost::lock_guard<boost::mutex> lock(*globalMessageVariableMutex);
            messageVar->setValue(messageDatum);
            messageVar->setSilentValue(0L);
		}
        
        // If the message is an error, and #stopOnError is set to true, stop the experiment
        if (type >= M_ERROR_MESSAGE) {
            if (auto soe = stopOnError) {
                if (soe->getValue().getBool()) {
                    if (auto stateSystem = StateSystem::instance(false)) {
                        stateSystem->stop();
                    }
                }
            }
        }
        
        // For debugging:  If the environment variable MWORKS_WRITE_MESSAGES_TO_STDERR is set,
        // write the message to standard error
        static const bool echo_to_stderr = []() {
            return (nullptr != getenv("MWORKS_WRITE_MESSAGES_TO_STDERR"));
        }();
        if (echo_to_stderr) {
            fprintf(stderr, "%s\n", buffer);
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
		
		_makeString(pre, ap, M_WARNING_MESSAGE, dom);
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

    /*
    void merror(MessageDomain dom, std::string format, ...) {
		va_list ap;
		va_start(ap, format);
		std::string pre("ERROR: " + format);
		_makeString(pre, ap, M_ERROR_MESSAGE, dom);
		va_end(ap);
	}
     */
    
    
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
#define MSG_BUFFER_SIZE 2048
		va_list ap;
		va_start(ap, format);
		char buffer[MSG_BUFFER_SIZE];// = { '\0' };
		vsnprintf(buffer, MSG_BUFFER_SIZE-1, format, ap);
		va_end(ap);
		fprintf(stderr, "MDEBUG: %s\n", buffer); fflush(stderr);
#endif
	}


END_NAMESPACE_MW
