//
//  MWVariableOutlineView.m
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/30/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWVariableOutlineView.h"


@implementation MWVariableOutlineView


- (void)textDidEndEditing:(NSNotification *)notification
{
    NSInteger editedColumn = [self editedColumn];
    NSInteger lastColumn = [[self tableColumns] count] - 1;
    int textMovement = [[[notification userInfo] valueForKey:@"NSTextMovement"] intValue];
    NSInteger nextRow = [self editedRow] + 1;
	
    // Need to do this *after* we get the info about the edited cell
    [super textDidEndEditing:notification];
    
    //
    // If the user finished editing a cell in the last column by pressing TAB, start editing the first editable
    // cell in the next row
    //
    if (editedColumn == lastColumn &&
        textMovement == NSTabTextMovement &&
        nextRow < [self numberOfRows])
    {
        NSUInteger firstEditableColumn = [[self tableColumns] indexOfObjectPassingTest:^(id obj, NSUInteger idx, BOOL *stop) {
            return [(NSTableColumn *)obj isEditable];
        }];
        [self selectRowIndexes:[NSIndexSet indexSetWithIndex:nextRow] byExtendingSelection:NO];
        [self editColumn:firstEditableColumn row:nextRow withEvent:nil select:YES];
    }
}


@end
