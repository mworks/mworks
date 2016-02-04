//
//  MarionetteMessage.m
//  MWorksCore
//
//  Created by bkennedy on 8/24/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MarionetteMessage.h"

#ifdef __x86_64__
#  define CURRENT_ARCH @"x86_64"
#else
#  define CURRENT_ARCH @"i386"
#endif


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

		NSXMLNode *arch_attribute = [element attributeForName:@"arch"];
        if (nil != arch_attribute) {
            arch = [[arch_attribute stringValue] copy];
        }
        
        numMatches = 0;
        
		NSXMLNode *max_matches_attribute = [element attributeForName:@"max_matches"];
        if (nil != max_matches_attribute) {
            maxNumMatches = [[max_matches_attribute stringValue] intValue];
        } else {
            maxNumMatches = -1;
        }
	}
	return self;
}

+ (id)messageWithXMLNode:(NSXMLElement*)element {
	return [[self alloc] initWithXMLNode:element];
}


- (NSString *)message { return message; }
- (MessageParseType)type {return type; }

- (BOOL)matches:(NSString *)string_to_match {
    BOOL result = NO;

	switch(type) {
		case M_CONTAINS:
		{
			NSRange error_range = [string_to_match rangeOfString:message];
			result = (error_range.location != NSNotFound);
			break;
		}
		case M_STARTS_WITH:
		{
			NSRange error_range = [string_to_match rangeOfString:message];
			result = (error_range.location == 0);
			break;
		}
		case M_ENDS_WITH:
		{
			NSRange error_range = [string_to_match rangeOfString:message];
			result = (error_range.location == ([string_to_match length] - error_range.length));
			break;
		}
		case M_WHOLE_MESSAGE:
			result = [string_to_match isEqualToString:message];
			break;
		default:
			break;
	}
    
    if (result) {
        numMatches++;
        if ((maxNumMatches >= 0) && (numMatches > maxNumMatches)) {
            result = NO;
        }
    }
    
    return result;
}

- (BOOL)archMatchesCurrentArch {
    return ((nil == arch) || [arch isEqualToString:CURRENT_ARCH]);
}

@end
