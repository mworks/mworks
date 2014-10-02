/* MWVariablesWindowController 
 *Copyright 2007 MIT. All rights reserved.
 */

#import "MWorksCocoa/MWClientProtocol.h"
#import "MWorksCocoa/MWCodec.h"
#import "MWorksCore/GenericData.h"
#import "MWVariablesDataSource.h"


@interface MWVariablesWindowController : NSWindowController <MWClientPluginWorkspaceState> {
	IBOutlet MWVariablesDataSource *ds;
	IBOutlet NSOutlineView *varView;
	IBOutlet id<MWClientProtocol> delegate;
	MWCodec *variables;
}

- (NSString *)getValueString:(NSString *)tag;
- (void)set:(NSString *)tag toValue:(mw::Datum *)val;

@end
