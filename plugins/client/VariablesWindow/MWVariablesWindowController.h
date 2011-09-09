/* MWVariablesWindowController 
 *Copyright 2007 MIT. All rights reserved.
 */

#import "MWorksCocoa/MWWindowController.h"
#import "MWorksCore/GenericData.h"
#import "MWVariablesDataSource.h"


@interface MWVariablesWindowController : NSWindowController {
	IBOutlet MWVariablesDataSource *ds;
	IBOutlet NSOutlineView *varView;
	IBOutlet id delegate;
	id variables;
}


@end


@interface MWVariablesWindowController(DelegateMethods) 
- (NSString *)getValueString:(NSString *)tag;
- (void)set:(NSString *)tag toValue:(mw::Datum *)val;
- (BOOL)isTagDictionary:(NSString *)tag;
- (BOOL)isTagList:(NSString *)tag;
@end