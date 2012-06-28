#import "MWVariablesWindowController.h"

#import "MWorksCocoa/MWClientServerBase.h"
#import "MWorksCocoa/MWCocoaEvent.h"
#import "MWorksCore/GenericData.h"
#import "MWorksCore/VariableProperties.h"

#define VARIABLES_WINDOW_CALLBACK_KEY "MWorksVariableWindowCallbackKey"

@interface MWVariablesWindowController(PrivateMethods)
- (void)cacheCodes;
- (void)populateDataSource;
- (void)serviceEvent:(MWCocoaEvent *)event;
- (void)causeDataReload:(NSTimer *)timer;
- (void)causeDataReload2:(id)arg;
@end

@implementation MWVariablesWindowController


- (void)awakeFromNib {

	variables = Nil;
	if([delegate respondsToSelector:@selector(variables)]){
		variables = [delegate variables];
	}

	[NSTimer scheduledTimerWithTimeInterval:0.75
									 target:self 
								   selector:@selector(causeDataReload:)
								   userInfo:nil 
									repeats:YES];
									
	[delegate registerEventCallbackWithReceiver:self 
                                       selector:@selector(serviceEvent:)
                                    callbackKey:VARIABLES_WINDOW_CALLBACK_KEY
                                forVariableCode:RESERVED_CODEC_CODE
                                   onMainThread:YES];
}

//- (void)setDelegate:(id)new_delegate {
//	if(![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:
//												   andSelector:
//												   andKey:)] ||
//	   ![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:
//												   andSelector:
//												   andKey:
//												   forVariableCode:)] ||
//	   ![new_delegate respondsToSelector:@selector(codeForTag:)] ||
//	   ![new_delegate respondsToSelector:@selector(unregisterCallbacksWithKey:)] ||
//	   ![new_delegate respondsToSelector:@selector(varGroups)] ||
//	   ![new_delegate respondsToSelector:@selector(valueStringForTag:)] ||
//	   ![new_delegate respondsToSelector:@selector(isDictionary:)] ||
//	   ![new_delegate respondsToSelector:@selector(isList:)] ||
//	   ![new_delegate respondsToSelector:@selector(set: to:)]) {
//		[NSException raise:NSInternalInconsistencyException
//					format:@"Delegate doesn't respond to required methods for MWVariablesWindowController"];		
//	}
//	
//	delegate = new_delegate;
//	[delegate registerEventCallbackWithReceiver:self 
//									andSelector:@selector(serviceEvent:)
//										 andKey:VARIABLES_WINDOW_CALLBACK_KEY
//								forVariableCode:RESERVED_CODEC_CODE];
//}


- (void)serviceEvent:(MWCocoaEvent *)event {
	if([event code] == RESERVED_CODEC_CODE) { // new codec arrived
		[self cacheCodes];
		[self populateDataSource];
	}
	
}

/*******************************************************************
*                MWWindowController Methods
*******************************************************************/
- (NSString *)mWorksFrameAutosaveName {
    return @"MWorksVariablesWindow";
}

/*******************************************************************
*                           Private Methods
*******************************************************************/
- (void)cacheCodes {
	if(delegate != nil) {
		[delegate unregisterCallbacksWithKey:VARIABLES_WINDOW_CALLBACK_KEY];
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceEvent:)
                                        callbackKey:VARIABLES_WINDOW_CALLBACK_KEY
                                    forVariableCode:RESERVED_CODEC_CODE
                                       onMainThread:YES];
		
	}
}	

- (void)populateDataSource {
	if(delegate != nil) {
		[ds setRootGroups:[delegate varGroups]];
		
		[varView performSelectorOnMainThread:@selector(reloadData)
								  withObject:nil
							   waitUntilDone:NO];	
	}
}

- (void)causeDataReload:(NSTimer *)timer {
	[self performSelectorOnMainThread:@selector(causeDataReload2:)
						   withObject:nil 
						waitUntilDone:NO];
}

- (void)causeDataReload2:(id)arg {
		[varView setNeedsDisplayInRect:[varView rectOfColumn:1]];	
}


/*******************************************************************
*                           Delegate Methods
*******************************************************************/
- (NSString *)getValueString:(NSString *)tag {
	if(variables != nil) {
		return [variables valueForKey:tag];
	} else {
		return @"";
	}
}

- (void)set:(NSString *)tag toValue:(mw::Datum *)val {
	if(variables != nil) {	
        NSObject *value;
        if (val->isString()) {
            value = [NSString stringWithCString:val->getString() encoding:NSASCIIStringEncoding];
        } else {
            value = [NSNumber numberWithDouble:val->getFloat()];
        }
        [variables setValue:value forKey:tag];
	}
}

- (BOOL)isTagDictionary:(NSString *)tag {
	//if(delegate != nil) {
//		return [[delegate isDictionary:tag] boolValue];
//	} else {
		return NO;
	//}
}

- (BOOL)isTagList:(NSString *)tag {
//	if(delegate != nil) {
//		return [[delegate isList:tag] boolValue];
//	} else {
		return NO;
//	}
}


@end
