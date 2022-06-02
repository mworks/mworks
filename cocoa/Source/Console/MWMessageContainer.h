/**
 * MWMessageContainer.h
 *
 * Description: The default data object for the default message view.
 *
 * History:
 * Paul Jankunas on 4/1/05 - Created.
 * Paul Jankunas on 7/20/05 - Overhaul of container to make it more 
 *  centralized and in control of messages.  It now supports the ability
 *  to create NSAttributedStrings of itself for the new message console.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#ifndef MW_MESSAGE_CONTAINER_H_
#define MW_MESSAGE_CONTAINER_H_

#import <Cocoa/Cocoa.h>

#include <MWorksCore/Event.h>
using namespace mw;

// Message type constants.
enum {
    MW_ERROR_MESSAGE       = M_ERROR_MESSAGE,
    MW_WARNING_MESSAGE     = M_WARNING_MESSAGE,
    MW_GENERIC_MESSAGE     = M_GENERIC_MESSAGE,
    MW_FATAL_ERROR_MESSAGE = M_FATAL_ERROR_MESSAGE,
    MW_MAX_MESSAGE_TYPE    = M_MAX_MESSAGE_TYPE
};

@interface MWMessageContainer : NSObject {
    NSString * message;
    NSDate * timeValue;
    NSNumber * altTimeValue;
    int msgType;
	int msgDomain;
    NSColor * messageTimeColor;
    NSColor * messageColor;
	NSMutableParagraphStyle *paragraphStyle;
@private
    NSColor * LocalColor;
	NSColor * RemoteColor;
    NSColor * DefaultColor;
    NSColor * WarningColor;
    NSColor * ErrorColor;
}

/**
 * Initializes a MWMessageContainer object with the given message, time and
 * message type.  Time is stored as an NSNumber and represents the time
 * taken from a core event.
 */
- (id)initMessageContainerWithMessage:(NSString *)msg 
							   atTime:(NSNumber *)tme 
							   ofType:(int)type
					   onLocalConsole:(BOOL)localConsole;

+ (NSMutableParagraphStyle *)sharedMessageParagraphStyle;															  
															  
/**
 * Returns an attributed string that is a concatenation of the time
 * and message components of this container.  The string also has the color
 * sets to the messageColor value.
 */
- (NSAttributedString *)messageForConsole;

/**
 * Setting the color of a message to a standard color.
 */
- (void)setMessageTimeColor:(int)colorType;
- (void)setMessageColor:(BOOL)isLocal;

/**
 * Returns the message as a plain string.
 */
- (NSString *)message;

/**
 * Returns the message color
 */
- (NSColor *)messageTimeColor;
- (NSColor *)messageColor;

/**
 * Returns the time as an NSDate object
 */
- (NSDate *)dateTimeValue;

/**
 * Returns the message time as an NSNumber
 */
- (NSNumber *)numberTimeValue;

/**
 * Returns the message type
 */
- (int)type;

@end

#endif
