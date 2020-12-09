/**
 * Utilities.h
 *
 * Description: Utility functions.
 *
 *
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 04/05/05 - Change message functions to create distinct
 *      message types.  
 * Paul Jankunas on 05/23/05 - Added message functions to place default
 *      text into different types of messages.
 * Paul Jankunas on 07/18/05 - Added mnetwork and mgeneric_printf functions
 *
 * Copyright 2005 MIT.  All rights reserved.
 */
#ifndef MONKEYWORKS_UTILITIES_H__
#define MONKEYWORKS_UTILITIES_H__

#define TOSTRING2(x) #x
#define TOSTRING(x) TOSTRING2(x)
#define FILELINE __FILE__ ":" TOSTRING(__LINE__)

#include <limits>
#include <string>

#include "Event.h"
#include "Exceptions.h"


BEGIN_NAMESPACE_MW


//#define MONKEYWORKS_DEBUG_MODE	0 // make this a compiler flag... 
//read the freaking history that is why it is there

/**
 * Generate message event and puts the event into the global event stream.
 * Some events have text prepended to them.  ex mwarning will prepend
 * the string "WARNING: ".
 */
void mprintf(const char *format, ...) __attribute__((__format__ (__printf__, 1, 2)));
void mprintf(MessageDomain dom, const char *format, ...) __attribute__((__format__ (__printf__, 2, 3)));
void parserwarning(const char *format, ...) __attribute__((__format__ (__printf__, 1, 2)));
void mwarning(MessageDomain dom, const char *format, ...) __attribute__((__format__ (__printf__, 2, 3)));
void parsererror(const char *format, ...) __attribute__((__format__ (__printf__, 1, 2)));

//void merror(MessageDomain dom, std::string, ...);
void merror(MessageDomain dom, const char *format, ...) __attribute__((__format__ (__printf__, 2, 3)));
void mfatal_error(MessageDomain dom, const char *format, ...) __attribute__((__format__ (__printf__, 2, 3)));
void mnetwork(const char * format, ...) __attribute__((__format__ (__printf__, 1, 2)));
// send generic type information.  this message will not attach any 
// characters to the message, but you can pass it a standard type.
void mgeneric_printf(int type, const char *format, ...) __attribute__((__format__ (__printf__, 2, 3)));

/**
 * Adding the debug message so that you can debug your code using
 * printfs without generating events.  To print debug messages you
 * must have MONKEYWORKS_DEBUG_MODE defined. 
 * Easiest thing is to make it a build 
 * flag in other c++ flags with the -D option.
 */
void mdebug(const char* format, ...) __attribute__((__format__ (__printf__, 1, 2)));

extern MessageOrigin GlobalMessageOrigin;


inline MWTime MIN_MWORKS_TIME() {
    return std::numeric_limits<MWTime>::min();
}


inline MWTime MAX_MWORKS_TIME() {
    return std::numeric_limits<MWTime>::max();
}
	

END_NAMESPACE_MW


#endif // !defined(MONKEYWORKS_UTILITIES_H__)


























