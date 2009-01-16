//
//  MWExperimentWindowController.h
//  NewEditor
//
//  Created by David Cox on 3/25/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWExperimentWindowController : NSWindowController {

	IBOutlet NSPanel *inspector_panel;
	IBOutlet NSPanel *library_panel;
    IBOutlet NSPanel *experiment_graph_panel;
	
    IBOutlet NSPanel *new_panel;
	
    IBOutlet NSDocument *document;
    
	BOOL inspector_panel_temporarily_suppressed;
	BOOL library_panel_temporarily_suppressed;
	BOOL experiment_graph_panel_temporarily_suppressed;
    
    BOOL inspectorPanelVisible;
    BOOL libraryPanelVisible;
	BOOL experimentGraphPanelVisible;
}

@property BOOL inspectorPanelVisible;
@property BOOL libraryPanelVisible;
@property BOOL experimentGraphPanelVisible;

- (IBAction)toggleInspectorVisible:(id)sender;
- (IBAction)toggleLibraryVisible:(id)sender;
- (IBAction)toggleExperimentGraphPanelVisible:(id)sender;

- (IBAction)newElement:(id)sender;

@end
