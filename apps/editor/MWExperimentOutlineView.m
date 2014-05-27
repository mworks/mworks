//
//  MWExperimentOutlineView.m
//  NewEditor
//
//  Created by David Cox on 11/16/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWExperimentOutlineView.h"
#import "MWExperimentTreeController.h"

@implementation MWExperimentOutlineView

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)acceptsFirstMouse:(NSEvent *)theEvent {
    return YES;
}

- (BOOL)needsPanelToBecomeKey {
    return NO;
}

- (IBAction)delete:(id)sender{

	[(MWExperimentTreeController *)[self delegate] remove:sender];
}


/*- (IBAction)undo:(id)sender{

	[[self delegate] undo:sender];
}


- (IBAction)redo:(id)sender{

	[[self delegate] redo:sender];
}*/


- (IBAction)copy:(id)sender{

	[(MWExperimentTreeController *)[self delegate] copy:sender];
}

- (IBAction)cut:(id)sender{

	[(MWExperimentTreeController *)[self delegate] cut:sender];
}

- (IBAction)paste:(id)sender{

	[(MWExperimentTreeController *)[self delegate] paste:sender];
}

- (IBAction)expandAll:(id)sender{
    
	[self expandItem:nil expandChildren:YES];
}

- (BOOL)validateMenuItem:(NSMenuItem *)item {

	return [(MWExperimentTreeController *)[self delegate] validateMenuItem:item];
}


- (void)keyDown:(NSEvent *)theEvent {
	if([[theEvent characters] isEqualToString:@" "]){
		//NSLog(@"Booyah");
		[(MWExperimentTreeController *)[self delegate] launchMiniInspector:[NSEvent mouseLocation]];
	}
	
	[super keyDown:theEvent];
}


/*- (void)highlightSelectionInClipRect:(NSRect)clipRect
	{
	}*/

@end
