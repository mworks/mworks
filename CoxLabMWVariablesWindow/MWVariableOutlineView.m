//
//  MWVariableOutlineView.m
//  MonkeyWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/30/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWVariableOutlineView.h"


@implementation MWVariableOutlineView


// make return and tab only end editing, and not cause other cells to edit

- (void) textDidEndEditing: (NSNotification *) notification
{
    NSDictionary *userInfo = [notification userInfo];
	
    int textMovement = [[userInfo valueForKey:@"NSTextMovement"] intValue];
	
    if (textMovement == NSReturnTextMovement
        || textMovement == NSTabTextMovement
        || textMovement == NSBacktabTextMovement) {
		
        NSMutableDictionary *newInfo;
        newInfo = [NSMutableDictionary dictionaryWithDictionary: userInfo];
		
        [newInfo setObject: [NSNumber numberWithInt: NSIllegalTextMovement]
					forKey: @"NSTextMovement"];
		
        notification =
            [NSNotification notificationWithName: [notification name]
										  object: [notification object]
										userInfo: newInfo];
		
    }
	
    [super textDidEndEditing: notification];
    [[self window] makeFirstResponder:self];
	
} // textDidEndEditing


//
//- (void) tableViewSelectionDidChange: (NSNotification *) notification
//{
//    int row;
//    row = [self selectedRow];
//	
//    if (row == -1) {
////        do stuff for the no-rows-selected case
//    } else {
////        do stuff for the selected row
//    }
//			
//} // tableViewSelectionDidChange









@end
