#import <Cocoa/Cocoa.h>

@interface ImageAndTextCell : NSTextFieldCell {
@private
    NSImage *image;
	NSString *alternateString;
	NSXMLElement *represented;
	
	NSTreeNode *node;
	
	id delegate;
}

- (void)setAccessoryImage:(NSImage *)anImage;
- (NSImage *)accessoryImage;

- (void)drawWithFrame:(NSRect)cellFrame inView:(NSView *)controlView;
- (NSSize)cellSize;

- (void)setAlternateString:(NSString *)alternate;
- (NSString *)alternateString;


- (IBAction)cut:(id)sender;
- (IBAction)copy:(id)sender;
- (IBAction)paste:(id)sender;

- (void)setDelegate:(id)del;
- (id)delegate;

- (void)setNode:(NSTreeNode *)node;
- (NSTreeNode *)node;

- (void)setRepresented:(NSXMLElement *)rep;

@end
