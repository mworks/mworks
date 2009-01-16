//
//  NSCustomInspectorView.m
//  NewEditor
//
//  Created by David Cox on 11/5/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWCustomInspectorView.h"


@implementation MWCustomInspectorView


- (void)setFrame:(NSRect)rect{
	[super setFrame:rect];
	[current_view setFrame:[self bounds]];
}

- (void)setInspectorView:(NSView *)view{
	current_view = view;
	[self  setSubviews:[NSArray arrayWithObjects:current_view,nil]];
	[self setAutoresizesSubviews:YES];
	[self setNeedsDisplay:YES];
	[current_view setFrame:[self bounds]];

}


@end
