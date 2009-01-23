//
//  MWVariableOutlineView.h
//  MonkeyWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/30/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWVariableOutlineView : NSOutlineView {

}


- (void) textDidEndEditing: (NSNotification *) notification;
- (void) tableViewSelectionDidChange: (NSNotification *) notification;

@end
