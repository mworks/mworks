//
//  MWBreadcrumbArrayController.m
//  NewEditor
//
//  Created by David Cox on 3/25/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MWBreadcrumbController.h"


@implementation MWBreadcrumbController


- (IBAction) gotoNode:(NSXMLElement *)node {
	NSLog(@"goto node called");
	[experiment_tree_controller setSelectionWithNode:node];
}


@end
