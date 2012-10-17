//
//  MWXMLNodeAdditions.m
//  NewEditor
//
//  Created by David Cox on 11/11/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWXMLNodeAdditions.h"


@implementation NSXMLNode (MWXMLNodeAdditions)

- (void)willChangeValueForKey:(NSString *)key{
	if (key && [self parent]) {
		[super willChangeValueForKey:key];
		[[self parent] oneStepWillChangeValueForKey:[self name]];
		[[self parent] oneStepWillChangeValueForKey:@"prettyXMLString"];
		[[self parent] oneStepWillChangeValueForKey:@"object"];
	}
}


- (void)didChangeValueForKey:(NSString *)key{
	if (key && [self parent]) {
		[super didChangeValueForKey:key];
		[[self parent] oneStepDidChangeValueForKey:[self name]];
		[[self parent] oneStepDidChangeValueForKey:@"prettyXMLString"];
		[[self parent] oneStepDidChangeValueForKey:@"object"];
	}
}

- (void)oneStepWillChangeValueForKey:(NSString *)key{
	if (key && [self parent]) {
		[super willChangeValueForKey:key];
		[super willChangeValueForKey:[self name]];
		[super willChangeValueForKey:@"prettyXMLString"];
		[super willChangeValueForKey:@"object"];
	}
}


- (void)oneStepDidChangeValueForKey:(NSString *)key{
	if (key && [self parent]) {
		[super didChangeValueForKey:key];
		[super didChangeValueForKey:[self name]];
		[super didChangeValueForKey:@"prettyXMLString"];
		[super didChangeValueForKey:@"object"];
	}
}


@end
