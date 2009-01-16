//
//  MWCustomInspectorSubviewController.m
//  NewEditor
//
//  Created by David Cox on 11/5/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWCustomInspectorSubviewController.h"


@implementation MWCustomInspectorSubviewController

- (id)initWithNibName:(NSString *)name{
	
	[self loadNib:name];
	
	return self;
}

- (BOOL)loadNib:(NSString *)name{

	// TODO: error check name
	if (![NSBundle loadNibNamed:name owner:self]){
        NSLog(@"Warning! Could not load nib file.\n");
		return NO;
    }
		
	// experiment_selection_controller should have been set from inside of the 
	if(experiment_selection_controller == Nil){
		return NO;
	}
	
	//[experiment_selection_controller bind:@"contentObject" 
//									 toObject:experiment_controller 
//									 withKeyPath:@"selection"
//									 options:nil];
	return YES;
}


- (NSView *)view{
	return custom_view;
}

@end
