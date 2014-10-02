#import "MWVariablesWindowController.h"

#import "MWorksCocoa/MWClientServerBase.h"
#import "MWorksCocoa/MWCocoaEvent.h"
#import "MWorksCore/GenericData.h"
#import "MWorksCore/VariableProperties.h"

#define VARIABLES_WINDOW_CALLBACK_KEY "MWorksVariableWindowCallbackKey"

@interface MWVariablesWindowController(PrivateMethods)
- (void)cacheCodes;
- (void)populateDataSource:(MWCocoaEvent *)event;
- (void)causeDataReload:(NSTimer *)timer;
@end

@implementation MWVariablesWindowController


- (void)awakeFromNib {

    variables = [delegate variables];

	NSTimer *reloadTimer = [NSTimer timerWithTimeInterval:0.75
                                                   target:self
                                                 selector:@selector(causeDataReload:)
                                                 userInfo:nil
                                                  repeats:YES];
    [[NSRunLoop mainRunLoop] addTimer:reloadTimer forMode:NSDefaultRunLoopMode];
									
    [self cacheCodes];
}


/*******************************************************************
*                           Private Methods
*******************************************************************/
- (void)cacheCodes {
	if(delegate != nil) {
		[delegate unregisterCallbacksWithKey:VARIABLES_WINDOW_CALLBACK_KEY];
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(populateDataSource:)
                                        callbackKey:VARIABLES_WINDOW_CALLBACK_KEY
                                    forVariableCode:RESERVED_CODEC_CODE
                                       onMainThread:YES];
		
	}
}	

- (void)populateDataSource:(MWCocoaEvent *)event {
	if(delegate != nil) {
		[ds setRootGroups:[delegate varGroups] forOutlineView:varView];
	}
}

- (void)causeDataReload:(NSTimer *)timer {
    [varView setNeedsDisplayInRect:[varView rectOfColumn:1]];
}


/*******************************************************************
*                           Delegate Methods
*******************************************************************/
- (NSString *)getValueString:(NSString *)tag {
	if(variables != nil) {
        mw::Datum value = [variables valueForVariable:tag];
		return [NSString stringWithUTF8String:(value.toString(true).c_str())];
	} else {
		return @"";
	}
}

- (void)set:(NSString *)tag toValue:(mw::Datum *)val {
	if(variables != nil) {	
        [variables setValue:(*val) forVariable:tag];
	}
}


- (NSDictionary *)workspaceState {
    NSMutableDictionary *workspaceState = [NSMutableDictionary dictionary];
    
    NSArray *expandedGroups = [ds expandedItems];
    if ([expandedGroups count] > 0) {
        [workspaceState setObject:expandedGroups forKey:@"expandedGroups"];
    }
    
    return workspaceState;
}


- (void)setWorkspaceState:(NSDictionary *)workspaceState {
    NSArray *expandedGroups = [workspaceState objectForKey:@"expandedGroups"];
    if (expandedGroups && [expandedGroups isKindOfClass:[NSArray class]]) {
        [ds setExpandedItems:expandedGroups forOutlineView:varView];
    }
}


@end


























