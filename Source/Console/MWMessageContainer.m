/**
* MWMessageContainer.m
 *
 * History:
 * Paul Jankunas on 4/1/05 - Created.
 *
 * Copyright 2005 MIT. All rights reserved.
 */

#import "MWMessageContainer.h"

@implementation MWMessageContainer

- (void)dealloc {
    [message release];
    message = nil;
    [timeValue release];
    timeValue = nil;
    [altTimeValue release];
    altTimeValue = nil;
    messageTimeColor = nil;
    [DefaultColor release];
    DefaultColor = nil;
    [WarningColor release];
    WarningColor = nil;
    [ErrorColor release];
    ErrorColor = nil;
    [FatalColor release];
    FatalColor = nil;
    [NetworkColor release];
    NetworkColor = nil;
	[LocalColor release];
	LocalColor = nil;
	[RemoteColor release];
	RemoteColor = nil;
    [super dealloc];
}

- (id)initMessageContainerWithMessage:(NSString *)msg 
							   atTime:(NSNumber *)tme 
							   ofType:(int)type
					   onLocalConsole:(BOOL)_localConsole {
    self = [super init];
    if(self) {
        message = [msg retain];
        altTimeValue = [tme retain];
        msgType = type;
        timeValue = nil;
        DefaultColor = [[NSColor colorWithCalibratedRed:0.0 green:0.5 
												   blue:0.0 alpha:1.0] retain];
		
        WarningColor = [[NSColor brownColor] retain];
        ErrorColor = [[NSColor redColor] retain];
        FatalColor = [[NSColor darkGrayColor] retain];
        // dark green
        NetworkColor = [[NSColor colorWithCalibratedRed:0.0 green:0.5 
												   blue:0.0 alpha:1.0] retain];
        
		LocalColor = [[NSColor colorWithCalibratedRed:0.0 green:0.0 
												 blue:0.0 alpha:1.0] retain];
		RemoteColor = [[NSColor colorWithCalibratedRed:0.2 green:0.2 
												 blue:0.2 alpha:1.0] retain];

		//[self initMessageParagraphStyle];
		
		[self setMessageTimeColor:msgType];
		[self setMessageColor:_localConsole];
    }
    return self;
}

+ (NSMutableParagraphStyle *)sharedMessageParagraphStyle {
	static NSMutableParagraphStyle *sharedMessageParagraphStyle = nil;
	
	if(sharedMessageParagraphStyle == nil){
		sharedMessageParagraphStyle = [[NSMutableParagraphStyle alloc] retain];
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
		
	
	
	NSMutableAttributedString * consoleMsg = 
		[ [NSMutableAttributedString alloc] 
						initWithString:
			[[NSDate dateWithTimeIntervalSince1970:
				[altTimeValue doubleValue]]
							 descriptionWithCalendarFormat:@"%H:%M:%S"
												  timeZone:nil
													locale:nil]	
			
							attributes:timeAttributes
			];
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
	
	[consoleMsg appendAttributedString:[appendstring autorelease]];
	[consoleMsg autorelease];
	
	/*return [[[NSAttributedString alloc] initWithString:consoleMsg 
attributes:[NSDictionary
                                        dictionaryWithObject:messageColor
													  forKey:NSForegroundColorAttributeName]]
autorelease];*/
	
	return consoleMsg;
}

- (void)setMessageTimeColor:(int)colorType {
    switch(colorType) {
        case MW_GENERIC_MESSAGE:
            messageTimeColor = DefaultColor;
            break;
        case MW_ERROR_MESSAGE:
            messageTimeColor = ErrorColor;
            break;
        case MW_WARNING_MESSAGE:
            messageTimeColor = WarningColor;
            break;
        case MW_FATAL_ERROR_MESSAGE:
            messageTimeColor = FatalColor;
            break;
        case MW_NETWORK_MESSAGE:
            messageTimeColor = NetworkColor;
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
        timeValue = [[NSDate dateWithTimeIntervalSince1970:[altTimeValue
			doubleValue]] retain];
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

