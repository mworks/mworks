//
//  MWErrorViewController.h
//  NewEditor
//
//  Created by David Cox on 11/13/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MWXMLDocument.h"
#import "MWExperimentTreeController.h"


@interface MWErrorArrayController : NSArrayController {

	IBOutlet NSTableView *error_table_view;
	
	IBOutlet NSPanel *errorPanel;
	
	IBOutlet MWExperimentTreeController *experiment_tree_controller;
	
	IBOutlet MWXMLDocument *doc;

}
- (IBAction)hideErrorPanel:(id)sender;
- (IBAction)showErrorPanel:(id)sender;
- (IBAction)toggleErrorPanel:(id)sender;

- (IBAction)triggerGoToError:(id)sender;

- (NSIndexPath *)findIndexPathForNodeID:(NSString *)node_id inTreeNode:(NSTreeNode *)tree_node;
@end
