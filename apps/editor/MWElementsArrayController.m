#import "MWElementsArrayController.h"

@implementation MWElementsArrayController

-(void)awakeFromNib {

    [elementsTableView setRefusesFirstResponder:YES];
}

-(void)dismiss {
    [window orderOut:self];
}

-(BOOL)control:(NSControl*)control textView:(NSTextView*)textView doCommandBySelector:(SEL)commandSelector {
    NSLog(@"delegate call");
    BOOL result = NO;	
    if (commandSelector == @selector(insertNewline:)) {
		// enter pressed
        NSLog(@"enter");
        // write the node to the pasteboard
        NSXMLElement *element = [[self selectedObjects] objectAtIndex:0];
        [libraryController writeElement:element toPasteboard:[NSPasteboard generalPasteboard]];
        [experimentController paste:self];
        [self dismiss];
        //[propertiesPanel makeKeyAndOrderFront:self];
		result = YES;
    }

	else if(commandSelector == @selector(moveUp:)) {
		// up arrow pressed
        NSLog(@"up");
        [self selectPrevious:self];
		result = YES;
	}
	else if(commandSelector == @selector(moveDown:)) {
		// down arrow pressed
        NSLog(@"down");
		[self selectNext:self];
        result = YES;
	}
    return result;
}

@end
