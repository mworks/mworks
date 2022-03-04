#import "MWEyeWindowController.h"
#import "MWPlotView.h"

#import "MWorksCocoa/MWClientServerBase.h"
#import "MWorksCocoa/MWCocoaEvent.h"
#import "MWorksCore/GenericData.h"
#import "MWorksCore/VariableProperties.h"
#import "MWorksCore/StandardVariables.h"
#import "MWorksCore/StimulusDisplay.h"


#define	EYE_WINDOW_CALLBACK_KEY @"MWEyeWindowController callback key"
NSString * MWEyeWindowVariableUpdateNotification = @"MWEyeWindowVariableUpdateNotification";


@implementation MWEyeWindowController


- (void)awakeFromNib {
	[[NSNotificationCenter defaultCenter] addObserver:self 
											 selector:@selector(updateEyeVariableNames)
												 name:MWEyeWindowVariableUpdateNotification 
											   object:nil];
	
	[plotView setTimeOfTail:[options.timeOfTail floatValue]];
}


@synthesize delegate=delegate;

- (void)setDelegate:(id)new_delegate {
	if(![new_delegate respondsToSelector:@selector(unregisterCallbacksWithKey:)] ||
	   ![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:
												   selector:
												   callbackKey:
                                                   onMainThread:)] ||
	   ![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:
												   selector:
												   callbackKey:
												   forVariableCode:
                                                   onMainThread:)] ||
	   ![new_delegate respondsToSelector:@selector(codeForTag:)]) {
		[NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWEyeWindowController"];		
	}
	
	delegate = new_delegate;
	[delegate registerEventCallbackWithReceiver:self 
                                       selector:@selector(codecReceived:)
                                    callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
								forVariableCode:RESERVED_CODEC_CODE
                                   onMainThread:YES];
}

- (IBAction)acceptWidth:(id)sender {
	[plotView setWidth:[scaleSlider floatValue]];
}

- (IBAction)reset:(id)sender {
	[plotView reset];
    [timePlotView reset];
}



- (void)updateEyeVariableNames {
	[plotView setTimeOfTail:[options.timeOfTail floatValue]];
	[self cacheCodes];
}



/*******************************************************************
 *                Callback Methods
 *******************************************************************/
- (void)codecReceived:(MWCocoaEvent *)event {
	[self cacheCodes];	
}

- (void)serviceHEvent:(MWCocoaEvent *)event {
	[plotView addEyeHEvent:event];		
}

- (void)serviceMainScreenInfoEvent:(MWCocoaEvent *)event {
    mw::Datum *mainScreenInfo = [event data];
    
    GLdouble left, right, bottom, top;
    mw::StimulusDisplay::getDisplayBounds(*mainScreenInfo, left, right, bottom, top);
    
    NSRect bounds = NSMakeRect(left, bottom, right-left, top-bottom);
    [plotView setDisplayBounds:bounds];
}

- (void)serviceVEvent:(MWCocoaEvent *)event {
	[plotView addEyeVEvent:event];		
}

- (void)serviceStmEvent:(MWCocoaEvent *)event {
	mw::Datum *stm_announce = [event data];
	
	if (stm_announce->isUndefined()) {					//stimulus announce should NEVER be NULL
		mwarning(M_NETWORK_MESSAGE_DOMAIN, "Received NULL for stimulus announce event.");
	} else {
		if(stm_announce->isList()) {
			[plotView acceptStmAnnounce:stm_announce Time:[event time]];
		}
	}
}

- (void)serviceCalEvent:(MWCocoaEvent *)event {
	mw::Datum *cal_announce = [event data];
	
	if (cal_announce->isUndefined()) {					//calibrator announce should NEVER be NULL
		mwarning(M_NETWORK_MESSAGE_DOMAIN, "Received NULL for calibrator announce event.");
	} else {
		if(cal_announce->isDictionary()) {
			[plotView acceptCalAnnounce:cal_announce];
		}
	}
}

- (void)serviceStateEvent:(MWCocoaEvent *)event {
	[plotView addEyeStateEvent:event];
}


- (void)serviceAuxHEvent:(MWCocoaEvent *)event {
	[plotView addAuxHEvent:event];
}


- (void)serviceAuxVEvent:(MWCocoaEvent *)event {
	[plotView addAuxVEvent:event];
}


- (void)serviceAEvent:(MWCocoaEvent *)event {
    [timePlotView addAEvent:event];
}


- (void)serviceBEvent:(MWCocoaEvent *)event {
    [timePlotView addBEvent:event];
}


/*******************************************************************
*                           Private Methods
*******************************************************************/
- (void)cacheCodes {
    int mainScreenInfoCodecCode = -1;
	int hCodecCode = -1;
	int vCodecCode = -1;
	int stimDisplayUpdateCodecCode = -1;
	int calAnnounceCodecCode = -1;
	int eyeStateCodecCode = -1;
	int auxHCodecCode = -1;
	int auxVCodecCode = -1;
	int aCodecCode = -1;
	int bCodecCode = -1;
	
	if(delegate != nil) {
        mainScreenInfoCodecCode = [[delegate codeForTag:@MAIN_SCREEN_INFO_TAGNAME] intValue];
		hCodecCode = [[delegate codeForTag:options.h] intValue];
		vCodecCode = [[delegate codeForTag:options.v] intValue];
		stimDisplayUpdateCodecCode = [[delegate codeForTag:@STIMULUS_DISPLAY_UPDATE_TAGNAME] intValue];
		calAnnounceCodecCode = [[delegate codeForTag:@ANNOUNCE_CALIBRATOR_TAGNAME] intValue];
		eyeStateCodecCode = [[delegate codeForTag:options.eyeState] intValue];
		auxHCodecCode = [[delegate codeForTag:options.auxH] intValue];
		auxVCodecCode = [[delegate codeForTag:options.auxV] intValue];
		aCodecCode = [[delegate codeForTag:options.a] intValue];
		bCodecCode = [[delegate codeForTag:options.b] intValue];
		
		[delegate unregisterCallbacksWithKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]];
        
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(codecReceived:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:RESERVED_CODEC_CODE
                                       onMainThread:YES];
		
		[delegate registerEventCallbackWithReceiver:self
                                           selector:@selector(serviceMainScreenInfoEvent:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:mainScreenInfoCodecCode
                                       onMainThread:NO];
		
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceHEvent:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:hCodecCode
                                       onMainThread:NO];
		
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceVEvent:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:vCodecCode
                                       onMainThread:NO];
		
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceStmEvent:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:stimDisplayUpdateCodecCode
                                       onMainThread:NO];
		
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceCalEvent:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:calAnnounceCodecCode
                                       onMainThread:NO];
		
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceStateEvent:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:eyeStateCodecCode
                                       onMainThread:NO];
		
		[delegate registerEventCallbackWithReceiver:self
                                           selector:@selector(serviceAuxHEvent:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:auxHCodecCode
                                       onMainThread:NO];
		
		[delegate registerEventCallbackWithReceiver:self
                                           selector:@selector(serviceAuxVEvent:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:auxVCodecCode
                                       onMainThread:NO];
		
		[delegate registerEventCallbackWithReceiver:self
                                           selector:@selector(serviceAEvent:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:aCodecCode
                                       onMainThread:NO];
		
		[delegate registerEventCallbackWithReceiver:self
                                           selector:@selector(serviceBEvent:)
                                        callbackKey:[EYE_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:bCodecCode
                                       onMainThread:NO];
	}
	if(hCodecCode == -1 || 
	   vCodecCode == -1 || 
	   calAnnounceCodecCode == -1 || 
	   eyeStateCodecCode == -1 || 
	   stimDisplayUpdateCodecCode == -1 ||
       auxHCodecCode == -1 ||
       auxVCodecCode == -1 ||
       aCodecCode == -1 ||
       bCodecCode == -1)
    {
		NSString *warningMessage = @"Eye window can't find the following variables: ";
		if(hCodecCode == -1) {
			warningMessage = [warningMessage stringByAppendingString:options.h];
			warningMessage = [warningMessage stringByAppendingString:@", "];
		}
		if(vCodecCode == -1) {
			warningMessage = [warningMessage stringByAppendingString:options.v];
			warningMessage = [warningMessage stringByAppendingString:@", "];
		}
		if(stimDisplayUpdateCodecCode == -1) {
			warningMessage = [warningMessage stringByAppendingString:@STIMULUS_DISPLAY_UPDATE_TAGNAME];
			warningMessage = [warningMessage stringByAppendingString:@", "];
		}
		if(calAnnounceCodecCode == -1) {
			warningMessage = [warningMessage stringByAppendingString:@ANNOUNCE_CALIBRATOR_TAGNAME];
			warningMessage = [warningMessage stringByAppendingString:@", "];
		}
		if(eyeStateCodecCode == -1) {
			warningMessage = [warningMessage stringByAppendingString:options.eyeState];
			warningMessage = [warningMessage stringByAppendingString:@", "];
		}
		if(auxHCodecCode == -1) {
			warningMessage = [warningMessage stringByAppendingString:options.auxH];
			warningMessage = [warningMessage stringByAppendingString:@", "];
		}
		if(auxVCodecCode == -1) {
			warningMessage = [warningMessage stringByAppendingString:options.auxV];
			warningMessage = [warningMessage stringByAppendingString:@", "];
		}
		if(aCodecCode == -1) {
			warningMessage = [warningMessage stringByAppendingString:options.a];
			warningMessage = [warningMessage stringByAppendingString:@", "];
		}
		if(bCodecCode == -1) {
			warningMessage = [warningMessage stringByAppendingString:options.b];
			warningMessage = [warningMessage stringByAppendingString:@", "];
		}
		
		warningMessage = [warningMessage substringToIndex:([warningMessage length] - 2)];
		mwarning(M_NETWORK_MESSAGE_DOMAIN, "%s", [warningMessage cStringUsingEncoding:NSASCIIStringEncoding]);		
		
	}
}	


- (NSDictionary *)workspaceState {
    NSMutableDictionary *workspaceState = [NSMutableDictionary dictionary];
    
    [workspaceState setObject:options.timeOfTail forKey:@"tailSeconds"];
    [workspaceState setObject:options.h forKey:@"eyeX"];
    [workspaceState setObject:options.v forKey:@"eyeY"];
    [workspaceState setObject:options.eyeState forKey:@"saccade"];
    [workspaceState setObject:options.auxH forKey:@"auxX"];
    [workspaceState setObject:options.auxV forKey:@"auxY"];
    [workspaceState setObject:options.a forKey:@"digA"];
    [workspaceState setObject:options.b forKey:@"digB"];
    
    return workspaceState;
}


- (void)setWorkspaceState:(NSDictionary *)workspaceState {
    NSNumber *newTimeOfTail = [workspaceState objectForKey:@"tailSeconds"];
    if (newTimeOfTail && [newTimeOfTail isKindOfClass:[NSNumber class]]) {
        options.timeOfTail = newTimeOfTail;
    }
    
    NSString *newH = [workspaceState objectForKey:@"eyeX"];
    if (newH && [newH isKindOfClass:[NSString class]]) {
        options.h = newH;
    }
    
    NSString *newV = [workspaceState objectForKey:@"eyeY"];
    if (newV && [newV isKindOfClass:[NSString class]]) {
        options.v = newV;
    }
    
    NSString *newEyeState = [workspaceState objectForKey:@"saccade"];
    if (newEyeState && [newEyeState isKindOfClass:[NSString class]]) {
        options.eyeState = newEyeState;
    }
    
    NSString *newAuxH = [workspaceState objectForKey:@"auxX"];
    if (newAuxH && [newAuxH isKindOfClass:[NSString class]]) {
        options.auxH = newAuxH;
    }
    
    NSString *newAuxV = [workspaceState objectForKey:@"auxY"];
    if (newAuxV && [newAuxV isKindOfClass:[NSString class]]) {
        options.auxV = newAuxV;
    }
    
    NSString *newA = [workspaceState objectForKey:@"digA"];
    if (newA && [newA isKindOfClass:[NSString class]]) {
        options.a = newA;
    }
    
    NSString *newB = [workspaceState objectForKey:@"digB"];
    if (newB && [newB isKindOfClass:[NSString class]]) {
        options.b = newB;
    }
}


@end




























