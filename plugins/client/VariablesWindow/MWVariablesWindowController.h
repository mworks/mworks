/* MWVariablesWindowController 
 *Copyright 2007 MIT. All rights reserved.
 */

#include <MWorksCore/GenericData.h>

#import <MWorksCocoa/MWClientProtocol.h>


@interface MWVariablesWindowController : NSWindowController <NSOutlineViewDataSource, NSOutlineViewDelegate, MWClientPluginWorkspaceState>

@property(nonatomic, weak) IBOutlet NSOutlineView *outlineView;
@property(nonatomic, weak) IBOutlet NSTableColumn *nameColumn;
@property(nonatomic, weak) IBOutlet NSTableColumn *valueColumn;
@property(nonatomic, weak) IBOutlet id<MWClientProtocol> client;

@end
