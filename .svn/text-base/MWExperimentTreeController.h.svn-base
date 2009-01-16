//
//  MWExperimentTreeController.h
//  NewEditor
//
//  Created by David Cox on 11/1/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MWXMLDocument.h"

@class MWExperimentTreeController;
@class MWInspectorViewController;

#import "MWInspectorViewController.h"
#import "NSTreeController_Extensions.h"


@interface MWExperimentTreeController : NSTreeController {
	
	// The parent document
	IBOutlet MWXMLDocument *document;
	
	// The outline view representing the experiment tree
	IBOutlet NSOutlineView *experiment_tree;
	IBOutlet NSMenu *context_menu;
	
	IBOutlet NSPanel *miniInspector;
	
	IBOutlet MWInspectorViewController *inspector_view_controller;
	
	// An array for holding internally dragged-n-dropped nodes
	NSArray *dragged_nodes;

}

- (IBAction) reloadData:(id)_id;
- (void)takeFocus;

- (IBAction)updateInspectorView:(id)sender;
- (BOOL)pasteDataFromPasteboard:(NSPasteboard *)pboard toDestinationIndexPath:(NSIndexPath *)destination;


- (void)setSelectionWithNode:(id)node;
- (NSIndexPath *)findIndexPathForNode:(NSXMLNode *)node inTreeNode:(NSTreeNode *)tree_node;

- (void)launchMiniInspector: (NSPoint)loc;


- (void) pasteInElements:(NSArray *)elements_to_paste atDestination:destination;
- (NSArray *)convertStringToXMLElements:(NSString *)raw_string;
- (NSArray *)smartConvertFilenamesToXMLElements:(NSArray *)files;
- (BOOL)validatePasteOfNodeWithKey:(NSString *)node_key 
                    library:(NSXMLDocument *)lib 
                    intoTarget:(NSXMLElement *)target_element 
                    atIndex:(int)index;
@end
