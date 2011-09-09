//
//  MWBreadcrumbArrayController.h
//  NewEditor
//
//  Created by David Cox on 3/25/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MWExperimentTreeController.h"


@interface MWBreadcrumbController : NSObject {
	IBOutlet MWExperimentTreeController *experiment_tree_controller;
}

- (IBAction) gotoNode:(NSXMLElement *)node;

@end
