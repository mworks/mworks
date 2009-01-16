//
//  MWGenericInspectorController.m
//  NewEditor
//
//  Created by David Cox on 11/4/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWGenericInspectorView.h"


@implementation MWGenericInspectorView


// NSTableView delegate notification
- (void)textDidEndEditing:(NSNotification *)aNotification{
	//[parent_tree_controller reloadData:self];
	[super textDidEndEditing:aNotification];
}

@end
