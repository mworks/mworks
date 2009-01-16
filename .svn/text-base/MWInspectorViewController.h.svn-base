//
//  MWInspectorViewController.h
//  NewEditor
//
//  Created by David Cox on 11/5/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MWCustomInspectorView.h"
#import "MWExperimentTreeController.h"

#define	MWCustomInspectorKey	0
#define MWGenericInspectorKey	1
#define MWXMLInspectorKey		2

@interface MWInspectorViewController : NSObject {

    // Inspector panel
    IBOutlet NSPanel *inspector_panel;

	// Inspector views
	IBOutlet MWCustomInspectorView *inspector_view;	
	IBOutlet NSView *generic_view;
	IBOutlet NSView *xml_view;
	
	// The currently displayed view
	int current_view_index;
	
	
	IBOutlet MWExperimentTreeController *experiment_controller;
	
	// set from inside a loaded custom inspector nib
	IBOutlet NSView *custom_view;
	IBOutlet NSObjectController *selected_object_controller;
	
	// Filtering to remove display hint (i.e. "_hint") attributes
	NSPredicate *attributeFilter;
	
	IBOutlet NSArrayController *attributesController;

}

- (int) currentViewIndex;
- (void) setCurrentViewIndex:(int)ind;

- (IBAction)update:(id)sender;
- (IBAction)switchView:(id)sender;

- (BOOL)loadNib:(NSString *)name;

- (void)setAttributeFilter:(NSPredicate *)predicate;

- (IBAction)addAttribute:(id)sender;

@end
