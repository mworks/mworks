#import "MWVariablesWindowController.h"

#import "MonkeyWorksCocoa/MWClientServerBase.h"
#import "MonkeyWorksCocoa/MWCocoaEvent.h"
#import "MonkeyWorksCore/GenericData.h"
#import "MonkeyWorksCore/VariableProperties.h"

#define VARIABLES_WINDOW_CALLBACK_KEY @"MonkeyWorksVariableWindow callback key"

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
									andSelector:@selector(serviceEvent:)
										 andKey:VARIABLES_WINDOW_CALLBACK_KEY
		forVariableCode:[NSNumber numberWithInt:RESERVED_CODEC_CODE]];
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
- (NSString *)monkeyWorksFrameAutosaveName {
    return @"MonkeyWorksVariablesWindow";
}

/*******************************************************************
*                           Private Methods
*******************************************************************/
- (void)cacheCodes {
	if(delegate != nil) {
		[delegate unregisterCallbacksWithKey:VARIABLES_WINDOW_CALLBACK_KEY];
		[delegate registerEventCallbackWithReceiver:self 
										andSelector:@selector(serviceEvent:)
											 andKey:VARIABLES_WINDOW_CALLBACK_KEY
									forVariableCode:[NSNumber numberWithInt:RESERVED_CODEC_CODE]];
		
	}
}	

- (void)populateDataSource {
	if(delegate != nil) {
		[ds addRootGroups:[[delegate varGroups] copyWithZone:Nil]];
		
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

- (void)set:(NSString *)tag toValue:(mw::Data *)val {
	if(variables != nil) {	
		string string_val = val->toString();
		NSString *the_string = [NSString stringWithCString:string_val.c_str() encoding:NSASCIIStringEncoding];
		[variables setValue:the_string forKey:tag];
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
