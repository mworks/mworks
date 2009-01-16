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

- (IBAction)delete:(id)sender{

	[[self delegate] remove:sender];
}


/*- (IBAction)undo:(id)sender{

	[[self delegate] undo:sender];
}


- (IBAction)redo:(id)sender{

	[[self delegate] redo:sender];
}*/


- (IBAction)copy:(id)sender{

	[[self delegate] copy:sender];
}

- (IBAction)cut:(id)sender{

	[[self delegate] cut:sender];
}

- (IBAction)paste:(id)sender{

	[[self delegate] paste:sender];
}

- (BOOL)validateMenuItem:(NSMenuItem *)item {

	return [[self delegate] validateMenuItem:item];
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
