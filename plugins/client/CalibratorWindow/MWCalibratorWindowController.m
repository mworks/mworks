#import "MWorksCore/StandardVariables.h"
#import "MWorksCocoa/MWCocoaEvent.h"
#import "MWorksCocoa/MWClientServerBase.h"
#import "MWorksCore/GenericData.h"
#import "MWCalibratorWindowController.h"
#import "MWCalibratorRecord.h"

#define DEFAULTS_PRESERVE_HIDDEN_PARAMS_ON_UPDATE_KEY @"Eye Calibrator Window - preserve hidden params on update"

#define CALIBRATOR_WINDOW_CALLBACK_KEY @"MWCalibratorWindowController callback key"


@interface MWCalibratorWindowController(PrivateMethods)
- (void)serviceEvent:(MWCocoaEvent *)event;
- (BOOL)insertCalibratorUnlessAlreadyExists:(NSString *)calibratorName maxHParams:(int)maxHParams maxVParams:(int)maxVParams;
- (BOOL)calibratorAlreadyExists:(NSString *)calibratorName;
- (NSString *)getDisplayedCalibratorName;
- (void)updateParameterDisplay;
- (int)getCalibratorIndexByName:(NSString *)calibratorName;
- (void)sendCalibrationParams:(int)calibratorIndex;
- (void)cacheCodes;
@end

@implementation MWCalibratorWindowController

- (void)awakeFromNib {
	calibratorRecords = [[NSMutableArray alloc] init];
	
	crLock = [[NSLock alloc] init];
	
	displayedCalibratorIndex = -1;
	calibratorAnnounceCode = [[NSNumber alloc] initWithInt:-1];
    
    self.preserveHiddenParamsOnUpdate = [[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_PRESERVE_HIDDEN_PARAMS_ON_UPDATE_KEY];
	
	[self setSelectedCalibratorName:DEFAULT_CALIBRATOR_DISPLAY];
	[self setVOffset:DEFAULT_V_OFFSET];
	[self setVGain:DEFAULT_V_GAIN];
	[self setHOffset:DEFAULT_H_OFFSET];
	[self setHGain:DEFAULT_H_GAIN];
}


@synthesize delegate;

- (void)setDelegate:(id)new_delegate {
	if(![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:
												   selector:
												   callbackKey:
                                                   onMainThread:
                                                   )] ||
	   ![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:
												   selector:
												   callbackKey:
												   forVariableCode:
                                                   onMainThread:)] ||
	   ![new_delegate respondsToSelector:@selector(codeForTag:)] ||
	   ![new_delegate respondsToSelector:@selector(unregisterCallbacksWithKey:)] ||
	   ![new_delegate respondsToSelector:@selector(setValue: forKey:)]) {
		[NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWCalibratorWindowController"];		
	}
	
	delegate = new_delegate;
	[delegate registerEventCallbackWithReceiver:self 
                                       selector:@selector(serviceEvent:)
                                    callbackKey:[CALIBRATOR_WINDOW_CALLBACK_KEY UTF8String]
                                   onMainThread:YES];

}

- (void)serviceEvent:(MWCocoaEvent *)event {	
	int code = [event code];

	if(code == 0) {
		[self cacheCodes];
	}

	if(code == [calibratorAnnounceCode intValue]){
		if([calibratorAnnounceCode intValue] != -1) {		
			
			Datum cal_announce(*[event data]);
			
			if(cal_announce.isDictionary()) {
				if(cal_announce.hasKey(CALIBRATOR_NAME) &&
				   cal_announce.hasKey(CALIBRATOR_ACTION) &&
				   cal_announce.hasKey(CALIBRATOR_PARAMS_H) &&
				   cal_announce.hasKey(CALIBRATOR_PARAMS_V) ) {
					// get the name of the calibrator that is being passed in the event
					if(cal_announce.getElement(CALIBRATOR_ACTION) == CALIBRATOR_ACTION_UPDATE_PARAMS) {
						Datum calibratorNameElement(cal_announce.getElement(CALIBRATOR_NAME));
						
						NSString *calibratorName = [NSString stringWithCString:calibratorNameElement.getString().c_str()
																	  encoding:NSASCIIStringEncoding];
						
						Datum hParams(cal_announce.getElement(CALIBRATOR_PARAMS_H));
						Datum vParams(cal_announce.getElement(CALIBRATOR_PARAMS_V));
						
						if(hParams.getNElements() >= MIN_NUM_CALIBRATION_PARAMS &&
						   vParams.getNElements() >= MIN_NUM_CALIBRATION_PARAMS) {
							
							
							[crLock lock];
							[self insertCalibratorUnlessAlreadyExists:calibratorName maxHParams:hParams.getNElements() maxVParams:vParams.getNElements()];
							int calibratorIndex = [self getCalibratorIndexByName:calibratorName];
							
							MWCalibratorRecord *cr = [calibratorRecords objectAtIndex:calibratorIndex];
							
							for(int i=0; i<hParams.getNElements(); ++i) {
								Datum hParam(hParams[i]);
								if(hParam.isFloat())	
									[cr setHParameterIndex:i value:hParam.getFloat()];
							}
							
							for(int i=0; i<vParams.getNElements(); ++i) {
								Datum vParam(vParams[i]);
								if(vParam.isFloat())	
									[cr setVParameterIndex:i value:vParam.getFloat()];
							}
							
							[crLock unlock];
						}
						// Thread safety
						[self performSelectorOnMainThread:
							@selector(updateParameterDisplay)
											   withObject: nil
											waitUntilDone: NO];						
					}
				}
			}
		}
	} 
}

/////////////////////////////////////////////////
// Accessors
/////////////////////////////////////////////////


- (void)setPreserveHiddenParamsOnUpdate:(BOOL)value {
    if (value != _preserveHiddenParamsOnUpdate) {
        [self willChangeValueForKey:@"preserveHiddenParamsOnUpdate"];
        _preserveHiddenParamsOnUpdate = value;
        [self didChangeValueForKey:@"preserveHiddenParamsOnUpdate"];
        [[NSUserDefaults standardUserDefaults] setBool:_preserveHiddenParamsOnUpdate
                                                forKey:DEFAULTS_PRESERVE_HIDDEN_PARAMS_ON_UPDATE_KEY];
    }
}


@synthesize vOffset = v_offset;
@synthesize vGain = v_gain;
@synthesize hOffset = h_offset;
@synthesize hGain = h_gain;
@synthesize selectedCalibratorName = selected_calibrator_name;


/*******************************************************************
*                           Private Methods
*******************************************************************/
- (int)getCalibratorIndexByName:(NSString *)calibratorName {
	for(int i=0; i<[calibratorRecords count]; ++i) {
		MWCalibratorRecord *cr = [calibratorRecords objectAtIndex:i];
		NSString *existCalibName = [cr getCalibratorName];
		if([existCalibName isEqualToString:calibratorName])
			return i;
	}
	return -1;
}

- (void)updateParameterDisplay {
	if(displayedCalibratorIndex > -1 && [calibratorRecords count] > 0) {
		[crLock lock];
		MWCalibratorRecord *cr = [calibratorRecords objectAtIndex:displayedCalibratorIndex];
		
		[self setSelectedCalibratorName:[cr getCalibratorName]];
		[self setVOffset:[cr getVParameter:V_OFFSET_INDEX]];
		[self setVGain:[cr getVParameter:V_GAIN_INDEX]];
		[self setHOffset:[cr getHParameter:H_OFFSET_INDEX]];
		[self setHGain:[cr getHParameter:H_GAIN_INDEX]];

		[crLock unlock];
	}
}

static void endEditingInWindow(NSWindow *window) {
    // Finish editing in all fields
    if (![window makeFirstResponder:window]) {
        [window endEditingFor:nil];
    }
}

- (IBAction)resetCalibratorParams:(id)sender {
    endEditingInWindow(self.window);
    
	if(![self.selectedCalibratorName isEqualToString:DEFAULT_CALIBRATOR_DISPLAY])
	{
		if(delegate != nil) {
			NSNumber *calibratorRequestCode = [delegate codeForTag:[NSString stringWithCString:REQUEST_CALIBRATOR_TAGNAME 
																					  encoding:NSASCIIStringEncoding]];
			
			[crLock lock];
			
			[self insertCalibratorUnlessAlreadyExists:self.selectedCalibratorName maxHParams:6 maxVParams:6]; // need to find a better way to manage the max elements
			
			int calibratorIndex = [self getCalibratorIndexByName:self.selectedCalibratorName];
			
			if(calibratorIndex > -1 && [calibratorRequestCode intValue] != -1) {
				Datum calibratorResetRequest(M_DICTIONARY, 2);
				
				Datum name([self.selectedCalibratorName cStringUsingEncoding:NSASCIIStringEncoding]);
				calibratorResetRequest.addElement(R_CALIBRATOR_NAME,name);
				
				Datum action(R_CALIBRATOR_ACTION_SET_PARAMETERS_TO_DEFAULTS);
				calibratorResetRequest.addElement(R_CALIBRATOR_ACTION,action);
				
				[delegate updateVariableWithCode:[calibratorRequestCode intValue] withData:&calibratorResetRequest];
			}
			[crLock unlock];
		}
	}
}

- (IBAction)updateCalibratorParams:(id)sender {
    endEditingInWindow(self.window);
    
	if(![self.selectedCalibratorName isEqualToString:DEFAULT_CALIBRATOR_DISPLAY])
	{
		[crLock lock];
		
		[self insertCalibratorUnlessAlreadyExists:self.selectedCalibratorName maxHParams:6 maxVParams:6]; // need to find a better way to manage the max elements
		
		
		int calibratorIndex = [self getCalibratorIndexByName:self.selectedCalibratorName];
		
		
		
		if(calibratorIndex > -1) {
			displayedCalibratorIndex = calibratorIndex;
			
			MWCalibratorRecord *cr = [calibratorRecords objectAtIndex:calibratorIndex];
			
			for(int i=0; i<[cr getNHParameters]; ++i) {
				double value = 0;
				
				switch(i) {
					case H_OFFSET_INDEX:
						value = [self hOffset];
						break;
					case H_GAIN_INDEX:
						value = [self hGain];
						break;
					default:
                        if (self.preserveHiddenParamsOnUpdate) {
                            value = [cr getHParameter:i];
                        } else {
                            value = 0.0;
                        }
						break;
				}
				
				[cr setHParameterIndex:i value:value];
			}
			
			for(int i=0; i<[cr getNVParameters]; ++i) {
				double value = 0;
				
				switch(i) {
					case V_OFFSET_INDEX:
						value = [self vOffset];
						break;
					case V_GAIN_INDEX:
						value = [self vGain];
						break;
					default:
                        if (self.preserveHiddenParamsOnUpdate) {
                            value = [cr getVParameter:i];
                        } else {
                            value = 0.0;
                        }
						break;
				}
				
				[cr setVParameterIndex:i value:value];
			}
			
			[self sendCalibrationParams:calibratorIndex];
		}
		[crLock unlock];
	}
}




- (void)sendCalibrationParams:(int)calibratorIndex {
	if(delegate != nil) {
		NSNumber *calibratorRequestCode = [delegate codeForTag:[NSString stringWithCString:REQUEST_CALIBRATOR_TAGNAME 
																				  encoding:NSASCIIStringEncoding]];
		
		MWCalibratorRecord *cr = [calibratorRecords objectAtIndex:calibratorIndex];
		
		// Create the dictionary object to use
		
		if([calibratorRequestCode intValue] != -1) {
			Datum calibratorRequest(M_DICTIONARY, 4);
			Datum name([[cr getCalibratorName] cStringUsingEncoding:NSASCIIStringEncoding]);
			calibratorRequest.addElement(R_CALIBRATOR_NAME,name);
			
			Datum action(R_CALIBRATOR_ACTION_SET_PARAMETERS);
			calibratorRequest.addElement(R_CALIBRATOR_ACTION,action);
			
			Datum hParams(M_LIST, [cr getNHParameters]);
			
			for(int i=0; i<[cr getNHParameters]; ++i) {
				Datum param(M_FLOAT, [cr getHParameter:i]);
				hParams.setElement(i,param);
			}
			
			calibratorRequest.addElement(R_CALIBRATOR_PARAMS_H,hParams);
			
			Datum vParams(M_LIST, [cr getNVParameters]);
			
			for(int i=0; i<[cr getNVParameters]; ++i) {
				Datum param(M_FLOAT, [cr getVParameter:i]);
				vParams.setElement(i,param);
			}
			
			calibratorRequest.addElement(R_CALIBRATOR_PARAMS_V,vParams);
			
			[delegate updateVariableWithCode:[calibratorRequestCode intValue] withData:&calibratorRequest];
		}
	}
}

- (NSString *)getDisplayedCalibratorName {
	return [[calibratorRecords objectAtIndex:displayedCalibratorIndex] getCalibratorName];
}

- (BOOL)calibratorAlreadyExists:(NSString *)calibratorName {
	for(int i=0; i<[calibratorRecords count]; ++i) {
		MWCalibratorRecord *cr = [calibratorRecords objectAtIndex:i];
		NSString *existCalibName = [cr getCalibratorName];
		if([existCalibName isEqualToString:calibratorName])
			return YES;
	}
	return NO;
}

- (BOOL)insertCalibratorUnlessAlreadyExists:(NSString *)calibratorName maxHParams:(int)maxHParams maxVParams:(int)maxVParams {
	if(![self calibratorAlreadyExists:calibratorName]) {
		MWCalibratorRecord *cr = [[MWCalibratorRecord alloc] initWithCalibratorName:calibratorName maxHParams:maxHParams maxVParams:maxVParams];
		[calibratorRecords addObject:cr];
		if(displayedCalibratorIndex == -1)
			displayedCalibratorIndex = 0;
		return true;
	} else {
		return false;
	}
}

- (void)cacheCodes {
	if(delegate != nil) {
		calibratorAnnounceCode = [delegate codeForTag:[NSString stringWithCString:ANNOUNCE_CALIBRATOR_TAGNAME
																		 encoding:NSASCIIStringEncoding]];
		
		[delegate unregisterCallbacksWithKey:[CALIBRATOR_WINDOW_CALLBACK_KEY UTF8String]];
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceEvent:)
                                        callbackKey:[CALIBRATOR_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:RESERVED_CODEC_CODE
                                       onMainThread:YES];
		
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceEvent:)
                                        callbackKey:[CALIBRATOR_WINDOW_CALLBACK_KEY UTF8String]
                                    forVariableCode:[calibratorAnnounceCode intValue]
                                       onMainThread:YES];
	}
}

@end
