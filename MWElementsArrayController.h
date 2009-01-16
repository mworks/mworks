#import <Cocoa/Cocoa.h>
#import "MWLibraryTreeController.h"
#import "MWExperimentTreeController.h"

@interface MWElementsArrayController : NSArrayController {

    IBOutlet NSTableView *elementsTableView;
    IBOutlet MWLibraryTreeController *libraryController;
    IBOutlet MWExperimentTreeController *experimentController;
    IBOutlet NSWindow *window;
    IBOutlet NSWindow *propertiesPanel;
}

-(BOOL)control:(NSControl*)control textView:(NSTextView*)textView doCommandBySelector:(SEL)commandSelector;

@end
