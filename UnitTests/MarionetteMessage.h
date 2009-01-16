//
//  MarionetteMessage.h
//  MonkeyWorksCore
//
//  Created by bkennedy on 8/24/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

enum MessageParseType {
	M_CONTAINS,
	M_STARTS_WITH, 
	M_ENDS_WITH,
	M_WHOLE_MESSAGE
};


@interface MarionetteMessage : NSObject {
	NSString *message;
	MessageParseType type;
}

- (id)initWithXMLNode:(NSXMLElement *)element;
+ (id)messageWithXMLNode:(NSXMLElement*)element;

- (NSString *)message;
- (MessageParseType)type;

- (BOOL)matches:(NSString *)string_to_match;

@end
