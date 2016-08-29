/* MWVariablesWindowController 
 *Copyright 2007 MIT. All rights reserved.
 */

#import <MWorksCocoa/MWClientProtocol.h>


@interface MWVariablesWindowController : NSWindowController <NSOutlineViewDataSource,
                                                             NSOutlineViewDelegate,
                                                             NSTextFieldDelegate,
                                                             MWClientPluginWorkspaceState>

@property(nonatomic, weak) IBOutlet NSOutlineView *outlineView;
@property(nonatomic, weak) IBOutlet NSSearchField *searchField;
@property(nonatomic, weak) IBOutlet id<MWClientProtocol> client;

- (IBAction)doubleClickOnOutlineView:(id)sender;
- (IBAction)searchTextChanged:(id)sender;

@end
