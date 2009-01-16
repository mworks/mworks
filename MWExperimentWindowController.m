//
//  MWExperimentWindowController.m
//  NewEditor
//
//  Created by David Cox on 3/25/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MWExperimentWindowController.h"


@implementation MWExperimentWindowController

@synthesize inspectorPanelVisible;
@synthesize libraryPanelVisible;
@synthesize experimentGraphPanelVisible;

- (id)init {
	inspector_panel_temporarily_suppressed = NO;
	library_panel_temporarily_suppressed = NO;
	experiment_graph_panel_temporarily_suppressed = NO;
	
	experimentGraphPanelVisible = NO;
	
	return self;
}

- (void)windowDidResignMain:(NSNotification *)notification {
	if([inspector_panel isVisible]){
		inspector_panel_temporarily_suppressed = YES;
	}
	[inspector_panel orderOut:self];
	
	if([library_panel isVisible]){
		library_panel_temporarily_suppressed = YES;
	}
	[library_panel orderOut:self];
	
	if([experiment_graph_panel isVisible]){
		experiment_graph_panel_temporarily_suppressed = YES;
	}
	[experiment_graph_panel orderOut:self];
	
}

- (void)windowDidBecomeMain:(NSNotification *)notification {
	if(inspector_panel_temporarily_suppressed){
		[inspector_panel orderFront:self];
		inspector_panel_temporarily_suppressed = NO;
	}
	
	if(library_panel_temporarily_suppressed){
		[library_panel orderFront:self];
		library_panel_temporarily_suppressed = NO;
	}
	
	if(experiment_graph_panel_temporarily_suppressed){
		[experiment_graph_panel orderFront:self];
		experiment_graph_panel_temporarily_suppressed = NO;
	}
}

- (IBAction)toggleInspectorVisible:(id)sender{
	if([inspector_panel isVisible]){
		[inspector_panel orderOut:self];
	} else {
		[inspector_panel orderFront:self];
	}
}
- (IBAction)toggleLibraryVisible:(id)sender{
	if([library_panel isVisible]){
		[library_panel orderOut:self];
	} else {
		[library_panel orderFront:self];
	}
}
- (IBAction)toggleExperimentGraphPanelVisible:(id)sender{
	if([experiment_graph_panel isVisible]){
		[experiment_graph_panel orderOut:self];
	} else {
		[experiment_graph_panel orderFront:self];
	}
}


- (IBAction)newElement:(id)sender{
    [new_panel makeKeyAndOrderFront:sender];
}


- (NSUndoManager *)windowWillReturnUndoManager:(NSWindow *)sender{ 
    return [[self document] undoManager];
}

@end
