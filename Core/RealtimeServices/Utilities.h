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

#include "Event.h"
#include <string>
#include "Exceptions.h"
namespace mw {
//#define MONKEYWORKS_DEBUG_MODE	0 // make this a compiler flag... 
//read the freaking history that is why it is there

/**
 * Generate message event and puts the event into the global event stream.
 * Some events have text prepended to them.  ex mwarning will prepend
 * the string "WARNING: ".
 */
void mprintf(const char *format, ...);
void mprintf(MessageDomain dom, const char *format, ...);
void parserwarning(const char *format, ...);
void mwarning(MessageDomain dom, const char *format, ...);
void parsererror(const char *format, ...);
void merror(MessageDomain dom, const char *format, ...);
void mfatal_error(MessageDomain dom, const char *format, ...);
void mnetwork(const char * format, ...);
// send generic type information.  this message will not attach any 
// characters to the message, but you can pass it a standard type.
void mgeneric_printf(int type, const char *format, ...);

/**
 * Adding the debug message so that you can debug your code using
 * printfs without generating events.  To print debug messages you
 * must have MONKEYWORKS_DEBUG_MODE defined. 
 * Easiest thing is to make it a build 
 * flag in other c++ flags with the -D option.
 */
void mdebug(const char* format, ...);

static inline MonkeyWorksTime MIN_MONKEY_WORKS_TIME() {
	MonkeyWorksTime min = 1;
	for(unsigned int i=0; i<(sizeof(MonkeyWorksTime)*8)-1; ++i) {
		min = min << 1;
	}
	return min;
}

static inline MonkeyWorksTime MAX_MONKEY_WORKS_TIME() {
	MonkeyWorksTime max = 0;
	MonkeyWorksTime min = MIN_MONKEY_WORKS_TIME();
	max = ~max;
	max = max ^ min;
	return max;
}

extern MessageOrigin GlobalMessageOrigin;


//void tick();  // start stop watch (like matlab function)
//long tock();  // report how much time has elapsed
	
}
#endif

