//
//  MWVarEntry.m
//  MWorksMATLABWindow
//
//  Created by Ben Kennedy on 9/22/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWVarEntry.h"


@implementation MWVarEntry

- (id) initWithName:(NSString *)name {
	self = [super init];
	if (self != nil) {
		variableName = [[NSString alloc] initWithString:name];
		selected = NO;
	}
	return self;
}


- (NSNumber *)selected {
	return [NSNumber numberWithBool:selected];
}

- (void)setSelected:(BOOL)_selected {
	selected = _selected;
}


- (NSString *)name {
	return variableName;
}

@end

