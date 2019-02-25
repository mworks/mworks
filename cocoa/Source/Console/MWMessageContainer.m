/**
* MWMessageContainer.m
 *
 * History:
 * Paul Jankunas on 4/1/05 - Created.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#include "boost/date_time/posix_time/posix_time.hpp"
#import "MWMessageContainer.h"

@implementation MWMessageContainer

- (void)dealloc {
    message = nil;
    timeValue = nil;
    altTimeValue = nil;
    messageTimeColor = nil;
    DefaultColor = nil;
    WarningColor = nil;
    ErrorColor = nil;
	LocalColor = nil;
	RemoteColor = nil;
}

- (id)initMessageContainerWithMessage:(NSString *)msg 
							   atTime:(NSNumber *)tme 
							   ofType:(int)type
					   onLocalConsole:(BOOL)_localConsole {
    self = [super init];
    if(self) {
        message = msg;
        altTimeValue = tme;
        msgType = type;
        timeValue = nil;
        DefaultColor = [NSColor colorWithCalibratedRed:0.0 green:0.5 
												   blue:0.0 alpha:1.0];
		
        WarningColor = [NSColor brownColor];
        ErrorColor = [NSColor redColor];
        
		LocalColor = [NSColor textColor];
        if (@available(macOS 10.13, *)) {
            RemoteColor = [NSColor colorNamed:@"consoleRemoteTextColor" bundle:[NSBundle bundleForClass:[self class]]];
        } else {
            RemoteColor = [NSColor colorWithCalibratedRed:0.2 green:0.2 blue:0.2 alpha:1.0];
        }

		//[self initMessageParagraphStyle];
		
		[self setMessageTimeColor:msgType];
		[self setMessageColor:_localConsole];
    }
    return self;
}

+ (NSMutableParagraphStyle *)sharedMessageParagraphStyle {
	static NSMutableParagraphStyle *sharedMessageParagraphStyle = nil;
	
	if(sharedMessageParagraphStyle == nil){
		sharedMessageParagraphStyle = [NSMutableParagraphStyle alloc];
		[sharedMessageParagraphStyle setHeadIndent:70.0];
	}
	
	return sharedMessageParagraphStyle;
}

- (NSAttributedString *)messageForConsole {
	
	NSDictionary *timeAttributes = [NSDictionary
										dictionaryWithObjectsAndKeys:
		messageTimeColor,
		NSForegroundColorAttributeName,
		
		[NSFont boldSystemFontOfSize:0],
		NSFontAttributeName,
		
		[MWMessageContainer sharedMessageParagraphStyle],
		NSParagraphStyleAttributeName,
		nil];
	
	NSDictionary *messageAttributes;
	
		messageAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
			messageColor,
			NSForegroundColorAttributeName,

			[MWMessageContainer sharedMessageParagraphStyle],
			NSParagraphStyleAttributeName,
			nil];
		
	
    namespace bpt = boost::posix_time;
    bpt::time_duration eventTime = bpt::seconds((long)([altTimeValue longLongValue] / 1000000LL));
	
	NSMutableAttributedString *consoleMsg =
    [[NSMutableAttributedString alloc]
     initWithString:[NSString stringWithUTF8String:(bpt::to_simple_string(eventTime).c_str())]
     attributes:timeAttributes];

	/*  NSString * consoleMsg = [NSString stringWithFormat:@"%@",
								[[NSDate dateWithTimeIntervalSince1970:
									[altTimeValue doubleValue]]
									descriptionWithCalendarFormat:@"%H:%M:%S"
														 timeZone:nil
														   locale:nil]	
		];


consoleMsg = [consoleMsg stringByAppendingString:[NSString 
                                        stringWithFormat:@" %@\n", message]];
*/
	NSAttributedString *appendstring = 
		[ [NSAttributedString alloc] 
							initWithString:
			[NSString stringWithFormat:@":  %@\n",message]
			
								attributes: messageAttributes];
	
  //[consoleMsg appendAttributedString:appendstring];
	[consoleMsg appendAttributedString:appendstring];
	
	/*return [[[NSAttributedString alloc] initWithString:consoleMsg 
attributes:[NSDictionary
                                        dictionaryWithObject:messageColor
													  forKey:NSForegroundColorAttributeName]]
autorelease];*/
	
	return consoleMsg;
}

- (void)setMessageTimeColor:(int)colorType {
    switch(colorType) {
        case MW_FATAL_ERROR_MESSAGE:
            [[clang::fallthrough]];
        case MW_ERROR_MESSAGE:
            messageTimeColor = ErrorColor;
            break;
        case MW_WARNING_MESSAGE:
            messageTimeColor = WarningColor;
            break;
        default:
            messageTimeColor = DefaultColor;
            break;
    }
}

- (void)setMessageColor:(BOOL)isLocal {
	if (isLocal) {
		messageColor = LocalColor;
	} else {
		messageColor = RemoteColor;		
	}
}

- (NSColor *)messageTimeColor {
    return messageTimeColor;
}

- (NSColor *)messageColor {
	return messageColor;
}

- (NSString *)message {
    return message;
}

- (NSDate *)dateTimeValue {
    if(timeValue == nil) {
        timeValue = [NSDate dateWithTimeIntervalSince1970:[altTimeValue
			doubleValue]];
    }
    return timeValue;
}

- (NSNumber *)numberTimeValue {
    return altTimeValue;
}

- (int)type {
    return msgType;
}

@end

