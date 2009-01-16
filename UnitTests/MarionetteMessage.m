//
//  MarionetteMessage.m
//  MonkeyWorksCore
//
//  Created by bkennedy on 8/24/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MarionetteMessage.h"


@implementation MarionetteMessage

- (id)initWithXMLNode:(NSXMLElement *)element {
	self = [super init];
	if(self != nil) {
		
		NSXMLNode *type_attribute = [element attributeForName:@"type"];
		if(type_attribute == nil) {
			NSLog(@"xml_node: %@ didn't have type attribute for message", [element XMLString]);
			NSLog(@"   defaulting to \"whole_mesage\"");
			type=M_WHOLE_MESSAGE;			
		} else {
			if([[type_attribute stringValue] isEqualToString:@"contains"]) {
				type = M_CONTAINS;
			} else if ([[type_attribute stringValue] isEqualToString:@"starts_with"]) {
				type = M_STARTS_WITH;
			} else if ([[type_attribute stringValue] isEqualToString:@"ends_with"]) {
				type = M_ENDS_WITH;
			} else if ([[type_attribute stringValue] isEqualToString:@"whole_message"]) {
				type = M_WHOLE_MESSAGE;
			} else {
				NSLog(@"xml_node: %@ didn't have valid type attribute for message", [element XMLString]);
				NSLog(@"   defaulting to \"whole_mesage\"");				
				type=M_WHOLE_MESSAGE;			
			}
		}
		
		message = [[element stringValue] copy];
	}
	return self;
}

+ (id)messageWithXMLNode:(NSXMLElement*)element {
	return [[[self alloc] initWithXMLNode:element] autorelease];
}

- (void) dealloc {
	[message release];
	[super dealloc];
}

- (NSString *)message { return message; }
- (MessageParseType)type {return type; }

- (BOOL)matches:(NSString *)string_to_match {
	switch(type) {
		case M_CONTAINS:
		{
			NSRange error_range = [string_to_match rangeOfString:message];
			return error_range.location != NSNotFound;
			break;
		}
		case M_STARTS_WITH:
		{
			NSRange error_range = [string_to_match rangeOfString:message];
			return error_range.location == 0;
			break;
		}
		case M_ENDS_WITH:
		{
			NSRange error_range = [string_to_match rangeOfString:message];
			return error_range.location == ([string_to_match length] - error_range.length);
			break;
		}
		case M_WHOLE_MESSAGE:
			return [string_to_match isEqualToString:message];
			break;
		default:
			return NO;
			break;
	}
}

@end
