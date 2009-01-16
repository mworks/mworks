//
//  MWStringTruncateValueTransformer.m
//  New Client
//
//  Created by David Cox on 3/12/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MWStringTruncateValueTransformer.h"


@implementation MWStringTruncateValueTransformer

+ (Class)transformedValueClass { return [NSString class]; }
+ (BOOL)allowsReverseTransformation { return NO; }
- (id)transformedValue:(id)value {	
	
	NSString *input = value;
	NSString *trunc = @"";
	
	if([input length] > TRUNC_MAX_CHARS){
		NSRange begin_chars, end_chars;
		
		begin_chars.location = 0; 
		begin_chars.length = TRUNC_BEGIN_CHARS;
		end_chars.location = [input length] - TRUNC_END_CHARS;
		end_chars.length = TRUNC_END_CHARS;
		trunc = [value substringWithRange:begin_chars];
		trunc = [trunc stringByAppendingString:@"..."];
		trunc = [trunc stringByAppendingString:[value substringWithRange:end_chars]];
	} else {
		trunc = input;
	}
	
	return trunc;
}

@end
