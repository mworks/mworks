#import "MWorksCocoa/MWCocoaEvent.h"
#import "MWorksCocoa/MWClientServerBase.h"
#import "MWorksCocoa/NSString+MWorksCocoaAdditions.h"
#import "MWorksCore/GenericData.h"
#import "MWorksCore/VariableProperties.h"
#import "MWMATLABWindowController.h"
#import "MWVarEntry.h"

#include <MWorksCore/ScarabServices.h>


@interface MWMATLABWindowController(Private)
- (void)codecArrived:(MWCocoaEvent *)codec;
- (void)updateClientCallbacks:(NSArray *)names_of_variables_to_register;
- (void)MATLABExecutionLoop:(id)obj;
- (void)serviceEvent:(MWCocoaEvent *)event;
- (void)reset;
- (void)updateCurrentSelectedVariables:(id)arg;
@end

#define MATLAB_WINDOW_CALLBACK_KEY @"MWorksMATLABWindow callback key"

#define MATLAB_EXECUTABLE_PATH @"MATLAB client window - MATLAB executable path"
#define MATLAB_M_FILE @"MATLAB client window - MATLAB .m file"
#define SYNC_EVENT_NAME @"MATLAB client window - sync event name"
#define SELECTED_VAR_NAMES @"MATLAB client window - selected variables"
#define COLLECTING_DATA @"MATLAB client window - running"
#define PROCESSING_DATA @"MATLAB client window - processing"
#define MW_SCROLL_TO_BOTTOM @"MATLAB client window - scroll to bottom on output"

@implementation MWMATLABWindowController


-  (id) init {
	self = [super init];
	if (self != nil) {
		matlabLock = [[NSRecursiveLock alloc] init];
		eventList = [[NSMutableArray alloc] init];
		executionList = [[NSMutableArray alloc] init];
		default_selected_variables = [[NSArray alloc] init];
		collectingEvents = NO;
		savedCodec = new Datum();
	}
	return self;
}



- (void)dealloc {
	delete savedCodec;
	
}

- (NSString *)matlabFileName {return [matlab_file_name lastPathComponent]; }
- (void)setMatlabFileName:(NSString *)new_matlab_file {
	matlab_file_name = [new_matlab_file copy];	
	[mi setMatlabFile:matlab_file_name];
	
}

- (void) setLogTextContent:(NSString *)new_content {
	logTextContent = [new_content copy];
	
	// set it in the UI
    [[logTextView textStorage] setAttributedString:[[NSAttributedString alloc] initWithString:logTextContent
                                                                                   attributes:@{ NSForegroundColorAttributeName: [NSColor textColor] }]];
	
};	
@synthesize logTextContent;


@synthesize delegate;

- (void)setDelegate:(id)new_delegate {
	if(![new_delegate respondsToSelector:@selector(unregisterCallbacksWithKey:)] ||
	   ![new_delegate respondsToSelector:@selector(variableNames)] ||
	   ![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:
												   selector:
												   callbackKey:
                                                   onMainThread:)] ||
	   ![new_delegate respondsToSelector:@selector(registerEventCallbackWithReceiver:
												   selector:
												   callbackKey:
												   forVariableCode:
                                                   onMainThread:)] ||
	   ![new_delegate respondsToSelector:@selector(codeForTag:)] ||
	   ![new_delegate respondsToSelector:@selector(updateVariableWithTag:
												   withData:)]) {
		[NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWMATLABWindowController"];		
	}
	
	delegate = new_delegate;
	[delegate registerEventCallbackWithReceiver:self 
                                       selector:@selector(codecArrived:)
                                    callbackKey:[MATLAB_WINDOW_CALLBACK_KEY UTF8String]
								forVariableCode:RESERVED_CODEC_CODE
                                   onMainThread:YES];	
}

- (void)awakeFromNib {
    if (!initializeScarab()) {
        NSLog(@"Scarab initialization failed");
    }
    
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	
	[self setProcessing:[defaults boolForKey:PROCESSING_DATA]];
	[self setRunning:[defaults boolForKey:COLLECTING_DATA]];
	
	NSString *matlabFile = [defaults objectForKey:MATLAB_M_FILE];
	if(matlabFile != nil) {
		[self setMatlabFileName:matlabFile];
	}
	
	
	if([defaults stringForKey:SYNC_EVENT_NAME]) {
		self.syncEventName = [defaults stringForKey:SYNC_EVENT_NAME];
	} else {
		self.syncEventName = @"";
	}
	
	[NSThread detachNewThreadSelector:@selector(MATLABExecutionLoop:) 
							 toTarget:self
						   withObject:nil];

	[self setLogTextContent:@"Matlab output:\n"];	
	if ([defaults boolForKey:MW_SCROLL_TO_BOTTOM]) {
		scrollToBottom = [defaults boolForKey:MW_SCROLL_TO_BOTTOM];
	} else {
		scrollToBottom = TRUE;
	}
	if (scrollToBottom == TRUE) {
		[scrollToBottomButton setState:NSControlStateValueOn];
	} else {		
		[scrollToBottomButton setState:NSControlStateValueOff];
	}
		
}

// *******************************************************************
// *                 Interface builder methods
// *******************************************************************
- (IBAction)chooseMATLABFile:(id)sender {
	[matlabLock lock];
	
	NSOpenPanel * op = [NSOpenPanel openPanel];
	[op setTitle:@"Select MATLAB .m file to open"];
	[op setCanChooseDirectories:NO];
	[op setCanChooseFiles:YES];
	[op setAllowsMultipleSelection:NO];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    [op setAllowedFileTypes:[NSArray arrayWithObject:@"m"]];
#pragma clang diagnostic pop
	
	int bp = [op runModal];
	
	if(bp == NSModalResponseOK) {
		
		NSArray * fn = [op URLs];
		NSEnumerator * fileEnum = [fn objectEnumerator];
		NSURL * filename;
		NSString *matlabFile;
		
		while((filename = [fileEnum nextObject])) {
			matlabFile = [filename path];
		}
		
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		[defaults setObject:matlabFile forKey:MATLAB_M_FILE];
		
		[self setMatlabFileName:matlabFile];
	}		
	
	[matlabLock unlock];
	[self reset];
}

- (IBAction)primeMATLABEngine:(id)sender {
	[matlabLock lock];
	[mi startMATLABEngine];
	[matlabLock unlock];
}

- (IBAction)resetAction:(id)sender {
	[self reset];
}


/**********************************
 * Accessors
 **********************************/
- (NSString *)syncEventName { return sync_event_name; }
- (void)setSyncEventName:(NSString *)new_sync_event_name {
	@synchronized(vl) {
		sync_event_name = [new_sync_event_name copy];
		[vl setSyncEventName:sync_event_name];
		[[NSUserDefaults standardUserDefaults] setObject:sync_event_name forKey:SYNC_EVENT_NAME];
		
		[self updateVariableFilter];
		[vl reloadData];
	}
	
	[self reset];
}

- (int)processing { return processing; }
- (void)setProcessing:(int)new_processing {
	[[NSUserDefaults standardUserDefaults] setBool:new_processing forKey:PROCESSING_DATA];
	processing = new_processing;
}

- (int)running { return running; }
- (void)setRunning:(int)new_running {
	[[NSUserDefaults standardUserDefaults] setBool:new_running forKey:COLLECTING_DATA];
	running = new_running;
}

@synthesize numberToProcessString = number_to_process_string;




/*******************************************************************
 *                 MWDataEventListenerProtocol Methods
 *******************************************************************/
- (void)serviceEvent:(MWCocoaEvent *)event {
	[matlabLock lock];
	if(running == NSControlStateValueOn) {
		if([event code] == [[delegate codeForTag:sync_event_name] intValue]) {
			Datum *syncData = [event data];
			if(syncData->getInteger() > 0 && !collectingEvents) {
				collectingEvents = YES;
				[eventList removeAllObjects];
			} else if(syncData->getInteger() == 0) {				
				if(collectingEvents) {
					collectingEvents = NO;
					[eventList addObject:event];
					[executionList addObject:[NSArray arrayWithArray:eventList]];
					[eventList removeAllObjects];
				}
			}
		}
		
		if(collectingEvents) {
			[eventList addObject:event];
		}
	}
	[matlabLock unlock];
}

#define MATLAB_DEBUG_OUTPUT_MAX_LENGTH 10000

- (IBAction)changeScrollToBottom:(id)sender {
	if ([sender state] == NSControlStateValueOn) {
		[self doScrollToBottom];
		scrollToBottom = TRUE; 
	} else {
		scrollToBottom = FALSE;
	}
	[[NSUserDefaults standardUserDefaults] setBool:scrollToBottom forKey:MW_SCROLL_TO_BOTTOM];
	
	//if ([sender state] == NSControlStateValueOff) {
	// do nothing
}


// *******************************************************************
// *                           Private Methods
// *******************************************************************



- (void) doScrollToBottomMain { 
	
	[logTextView scrollRangeToVisible:NSMakeRange([[logTextView textStorage] length], 0) ];
	
}


- (void) appendLogTextMain:(NSString *)logText {
    NSTextStorage *textStorage = logTextView.textStorage;
    [textStorage appendAttributedString:[[NSAttributedString alloc] initWithString:logText
                                                                        attributes:@{ NSForegroundColorAttributeName: [NSColor textColor] }]];
    
    NSMutableString* tStr = textStorage.mutableString;
	if ([tStr characterAtIndex:[tStr length]-1] != '\n') {
		// append newline
		[tStr appendString:@"\n"];
	}
    
	// Trim the beginning of the string if it is too long
	if ([tStr length] > MATLAB_DEBUG_OUTPUT_MAX_LENGTH) {
		NSRange deleteRange = NSMakeRange(0, ([tStr length] - MATLAB_DEBUG_OUTPUT_MAX_LENGTH));
			[tStr deleteCharactersInRange:deleteRange];
		}
			  
		[self->logTextView setNeedsDisplay:YES];
		if (self->scrollToBottom) {
			[self doScrollToBottom];
		}
}			  
	
- (void)codecArrived:(MWCocoaEvent *)event {
	[matlabLock lock];
    
    Datum *new_codec = [event data];
    
    if (*new_codec == *savedCodec) {
        // Codec is unchanged.  No need to reset anything.
        [matlabLock unlock];
        return;
    }
	
	// need to force it by capturing the pointer so it makes it a dictionary
	delete savedCodec;
	savedCodec = new Datum(*new_codec);
	
	NSArray *current_selected_variables = [NSArray array];
	
	[self performSelectorOnMainThread:@selector(updateDefaultSelectedVariables:)
						   withObject:nil 
						waitUntilDone:YES];
	
	@synchronized(vl) {
		
		[vl clear];
		
		// determine max code number of codec
		int max_key = -1;
        for (auto &item : new_codec->getDict()) {
            max_key = std::max(max_key, int(item.first));
        }
		
		for(int i=0;i<=max_key;++i) {
			Datum codec_entry(new_codec->getElement(Datum(M_INTEGER, i)));
			
			if(!codec_entry.isUndefined()) {
				NSString *variableName = [NSString stringWithCString:codec_entry.getElement("tagname").getString().c_str()
															encoding:NSASCIIStringEncoding];
				MWVarEntry *var = [[MWVarEntry alloc] initWithName:variableName];
				
				// select previous variables
				if(default_selected_variables != nil) {
					NSEnumerator *enumerator = [default_selected_variables objectEnumerator];
					NSString *selectedVariableName;
					
					while(selectedVariableName = [enumerator nextObject]) {
						if([selectedVariableName isEqualToString:variableName]) {
							[var setSelected:YES];
						}
					}
				}
				[vl addVariable:var];		
			}
		}
		
		[vl setSyncEventName:[self syncEventName]];
		current_selected_variables = [vl currentSelectedVariables];
		[vl reloadData];
	}
	[matlabLock unlock];
	
	[self updateClientCallbacks:current_selected_variables];
	
	[self reset];
}

// must be run on main thread
- (void)updateDefaultSelectedVariables:(id)arg {
	default_selected_variables = [[NSArray alloc] initWithArray:[[NSUserDefaults standardUserDefaults] objectForKey:SELECTED_VAR_NAMES]];
}




- (void)updateClientCallbacks:(NSArray *)codes_to_register {
	[delegate unregisterCallbacksWithKey:[MATLAB_WINDOW_CALLBACK_KEY UTF8String]];
	
	// register for codecs
	[delegate registerEventCallbackWithReceiver:self 
                                       selector:@selector(codecArrived:)
                                    callbackKey:[MATLAB_WINDOW_CALLBACK_KEY UTF8String]
								forVariableCode:RESERVED_CODEC_CODE
                                   onMainThread:YES];		
	
	NSString *name;	
	NSEnumerator *enumerator = [codes_to_register objectEnumerator];
	
	while(name = [enumerator nextObject]) {
		NSNumber *event_code = [delegate codeForTag:name];
		[delegate registerEventCallbackWithReceiver:self 
                                           selector:@selector(serviceEvent:)
                                        callbackKey:[MATLAB_WINDOW_CALLBACK_KEY UTF8String]
									forVariableCode:[event_code intValue]
                                       onMainThread:YES];	
	}
}

- (void)MATLABExecutionLoop:(id)obj {
	@autoreleasepool {
		do { 
			@autoreleasepool {
				[matlabLock lock];
				self.numberToProcessString = [NSString stringWithFormat:@"Process %lu events", (unsigned long)[executionList count]];
				[matlabLock unlock];
				
				
				if(processing == NSControlStateValueOn) {
					NSArray *eventsToExecute = nil;
					[matlabLock lock];
					Datum codec(*savedCodec);
					if([executionList count] > 0 && codec.isDictionary()) {
						eventsToExecute = [executionList objectAtIndex:0];
						[executionList removeObjectAtIndex:0];
					}
					[matlabLock unlock];
					
					if(eventsToExecute != nil && codec.isDictionary()) {
						mxArray *dataStruct = [mi createDataStruct:eventsToExecute
														 withCodec:&codec];
						[mi runMatlabFile:dataStruct];
						mxDestroyArray(dataStruct);
					}
				}
				usleep(5000); // sleep to surrender the processor
			}
		} while (1);
	}
}

// this must be locked before being called
- (void)reset {
	[matlabLock lock];
	collectingEvents = NO;
	//	[delegate set:[[delegate codeForTag:[self syncEventName]] intValue] to:&sync_zero];
	//	[delegate setValue:[NSNumber numberWithInt:0] forKey:[@"variables." stringByAppendingString:self.syncEventName]];
	
	
	NSArray *variable_names = [delegate variableNames];
	NSEnumerator *var_name_enumerator = [variable_names objectEnumerator];
	NSString *var_name = nil;
	
	while(var_name = [var_name_enumerator nextObject]) {
		if([var_name isEqualToString:self.syncEventName]) {
			Datum data(0L);
			[delegate updateVariableWithTag:self.syncEventName 
								   withData:&data];			
		}
	}
	
	[mi resetRetval];
	[executionList removeAllObjects];	
	[matlabLock unlock];
}


///////////////////////////////////////////////////////////////////////
// Delegate functions
///////////////////////////////////////////////////////////////////////


// this only gets called from the main thread
- (void)updateVariableFilter {
	[matlabLock lock];
	
	NSArray *selected_variables;
	
	@synchronized(vl) {
        selected_variables = [vl currentSelectedVariables];
	}
	
	if([selected_variables count] > 1) {
		// don't update the defaults until there is some variables to update
		[[NSUserDefaults standardUserDefaults] setObject:selected_variables forKey:SELECTED_VAR_NAMES];		
	}
	
	
	[matlabLock unlock];
	[self updateClientCallbacks:selected_variables];	
}

- (void) appendLogText:(NSString *)logText {
	
	[self performSelectorOnMainThread: @selector(appendLogTextMain:)
						   withObject: logText
						waitUntilDone: NO];
}

-(void) doScrollToBottom {
	
	[self performSelectorOnMainThread:@selector(doScrollToBottomMain)
						   withObject:nil 
						waitUntilDone:YES];
}


- (NSDictionary *)workspaceState {
    NSMutableDictionary *workspaceState = [NSMutableDictionary dictionary];
    
    if (running == NSControlStateValueOn) {
        if (matlab_file_name) {
            [workspaceState setObject:matlab_file_name forKey:@"filePath"];
        }
        if (sync_event_name) {
            [workspaceState setObject:sync_event_name forKey:@"syncEventName"];
        }
        @synchronized(vl) {
            [workspaceState setObject:[vl currentSelectedVariables] forKey:@"selectedVariables"];
        }
    }
    
    return workspaceState;
}


- (void)setWorkspaceState:(NSDictionary *)workspaceState {
    NSString *newFilePath = [workspaceState objectForKey:@"filePath"];
    if (newFilePath && [newFilePath isKindOfClass:[NSString class]]) {
        newFilePath = [newFilePath mwk_absolutePath];
        [[NSUserDefaults standardUserDefaults] setObject:newFilePath forKey:MATLAB_M_FILE];
        [self setMatlabFileName:newFilePath];
    }
    
    NSString *newSyncEventName = [workspaceState objectForKey:@"syncEventName"];
    if (newSyncEventName && [newSyncEventName isKindOfClass:[NSString class]]) {
        [self setSyncEventName:newSyncEventName];
    }
    
    NSArray *newSelectedVariables = [workspaceState objectForKey:@"selectedVariables"];
    if (newSelectedVariables &&
        [newSelectedVariables isKindOfClass:[NSArray class]] &&
        [newSelectedVariables count] > 0)
    {
        [[NSUserDefaults standardUserDefaults] setObject:newSelectedVariables forKey:SELECTED_VAR_NAMES];
    }
}


@end



























