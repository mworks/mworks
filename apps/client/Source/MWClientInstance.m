//
//  MWClientInstance.m
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWClientInstance.h"

#import <MWorksCocoa/MWClientPlugin.h>
#import <MWorksCocoa/MWConsoleController.h>
#import <MWorksCocoa/NSString+MWorksCocoaAdditions.h>

#define ERROR_MESSAGE_CALLBACK_KEY	@"MWClientInstance::client_error_message_callback"
#define CLIENT_SYSTEM_EVENT_CALLBACK_KEY  @"MWClientInstance::system_event_callback"
#define EXPERIMENT_LOAD_PROGRESS_KEY @"MWClientInstance::experiment_load_progress_callback"
#define CLIENT_LOAD_MESSAGE_CALLBACK_KEY    @"MWClientInstance::load_message_callback"
#define CLIENT_CODEC_EVENT_CALLBACK_KEY @"MWClientInstance::codec_callback"

#define DEFAULTS_OPEN_PLUGIN_WINDOWS_KEY @"openPluginWindows"
#define DEFAULTS_GROUPED_PLUGIN_WINDOW_IS_OPEN_KEY @"groupedPluginWindowIsOpen"

#define MAX_NUM_RECENT_EXPERIMENTS 20


@implementation MWClientInstance


- (id)initWithAppController:(AppController *)_controller {
    // TODO: handle client creation failure!
    MWKClient *_client = [MWKClient client:NULL];

    self = [super initWithClient:_client];
    if (!self) {
        return nil;
    }
    
	appController = _controller;

	accumulatingErrors = NO;
	
    // Cosmetics
    headerColor = [appController uniqueColor];
    [headerBox setFillColor:headerColor];

	protocolNames = [[NSMutableArray alloc] init];
	errors = [[NSMutableArray alloc] init];
    serversideVariableSetNames = [[NSMutableArray alloc] init];
	
    NSArray *recentServers = [[NSUserDefaults standardUserDefaults] arrayForKey:@"recentServers"];
    if (recentServers && ([recentServers count] > 0)) {
        serverURL = [[recentServers objectAtIndex:0] copy];
    } else {
        serverURL = @"localhost";
    }
    
	serverPort = [NSNumber numberWithInteger:19989];
	
    variableSetName = Nil;
	self.variableSetName = Nil;
    self.variableSetLoaded = NO;
    
    // Register a callback to receive codec events
    [self.client registerCallbackWithKey:CLIENT_CODEC_EVENT_CALLBACK_KEY
                                 forCode:MWKReservedEventCodeCodec
                                callback:^(MWKEvent *event) {
                                    [self receiveCodec:event];
                                }];
    
    [self.client registerCallbackWithKey:CLIENT_SYSTEM_EVENT_CALLBACK_KEY
                                 forCode:MWKReservedEventCodeSystemEvent
                                callback:^(MWKEvent *event) {
                                    dispatch_async(dispatch_get_main_queue(), ^{
                                        [self handleSystemEvent:event];
                                    });
                                }];
									 
    [self setDataFileWillAutoOpen:NO];
	[self setDataFileOpen:false];
	[self setExperimentLoaded:false];
	[self setServerConnected:false];
	[self setLaunchIfNeeded:YES];
    
	// load plugins
	pluginWindows = [[NSMutableArray alloc] init];
	[self loadPlugins];
	
    [self.client startEventListener];
  
	#define CONNECTION_CHECK_INTERVAL	1.0
    connection_check_timer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)CONNECTION_CHECK_INTERVAL 
                                                              target:self 
                                                            selector:@selector(checkConnection) 
                                                            userInfo:Nil 
                                                             repeats:YES];
	
    
	return self;
}


- (void)shutDown {
	if([self serverConnected] || [self serverConnecting]){
		[self disconnect];
	}
    
    if (appController.shouldRestoreOpenPluginWindows) {
        [self saveOpenPluginWindows];
    }

    [self hideAllPlugins];
}

/*- (void)connectToURL:(NSString *)URL AtPort:(NSString *)port{
	NSLog([NSString stringWithFormat:@"Connecting beyotch (%@, %@)", URL, port]);
}*/



@synthesize	serverConnected;
@synthesize serverConnecting;
@synthesize serverURL;
@synthesize serverPort;
@synthesize serverName;
@synthesize launchIfNeeded;

// Experiment state
@synthesize experimentLoading;
@synthesize experimentLoadProgress;
@synthesize experimentLoadMessage;
@synthesize experimentLoaded;
@synthesize experimentRunning;
@synthesize experimentPaused;
@synthesize experimentName;
@synthesize clientsideExperimentPath;
@synthesize experimentPath;
@synthesize hasExperimentLoadErrors;
@synthesize errors;
@synthesize errorString;

// Variable set
@synthesize variableSetName;
@synthesize serversideVariableSetNames;
@synthesize variableSetLoaded;

// Data File state
@synthesize dataFileWillAutoOpen;
@synthesize dataFileOpen;
@synthesize dataFileName;
@synthesize dataFileOverwrite;

// Protocol state
@synthesize protocolNames;
@synthesize currentProtocolName;
@synthesize summaryString;

// Cosmetics
@synthesize headerColor;


- (void)setServerURL:(NSString *)newServerURL {
    if (![serverURL isEqualToString:newServerURL]) {
        serverURL = [newServerURL copy];
        [grouped_plugin_controller setWindowFrameAutosaveName:serverURL];
    }
}


- (void)checkConnection {
  
  @synchronized(self){
  
    BOOL actually_connected = self.client.connected;

	if(actually_connected != [self serverConnected]){
		[self setServerConnected:actually_connected];
		
		if(![self serverConnected]){
			[self disconnect];
		}
	} else if([self serverConnected]){
        NSString *putative_name = [self.client valueForTag:@"#serverName"].stringValue;
        if(putative_name != [self serverName] && putative_name != Nil){
            [self setServerName: putative_name];
        }
    }
	
  }
}

// Maintaining Overall GUI state
- (void)enforceDisconnectedState{
	[self setServerConnected:NO];
	[self setServerConnecting:NO];
	[self setExperimentLoaded:NO];
	[self setHasExperimentLoadErrors:NO];
	[self setExperimentLoading:NO];
	[self setExperimentRunning:NO];
	[self setExperimentPaused:NO];
	
	[self setExperimentName:@""];
	[self setExperimentPath:@""];
    
    if (appController.shouldAutoClosePluginWindows) {
        [self hideAllPlugins];
    }
	
	[self setServerName:Nil];
    [self setVariableSetName:Nil];
    
    [self setDataFileWillAutoOpen:NO];
    [self setDataFileOpen:NO];
    [self setDataFileName:Nil];
    [self setDataFileOverwrite:NO];
}


- (void)enforceConnectedState{
	[self setServerConnected:YES];
	[self setServerConnecting:NO];
	[self setExperimentLoaded:NO];
	[self setHasExperimentLoadErrors:NO];
	[self setExperimentLoading:NO];
	[self setExperimentRunning:NO];
	[self setExperimentPaused:NO];
	[self setExperimentPath:Nil];
	[self setExperimentName:Nil];
    [self setVariableSetName:Nil];
}

- (void)enforceLoadedState{
	[self setServerConnected:YES];
	[self setServerConnecting:NO];
	[self setExperimentLoaded:YES];
	[self setHasExperimentLoadErrors:NO];
	[self setExperimentLoading:NO];
	[self setExperimentRunning:NO];
	[self setExperimentPaused:NO];
}

- (void)enforceLoadFailedState{
	[self setServerConnected:YES];
	[self setServerConnecting:NO];
	[self setExperimentLoaded:NO];
	[self setHasExperimentLoadErrors:YES];
	[self setExperimentLoading:NO];
	[self setExperimentRunning:NO];
	[self setExperimentPaused:NO];
	[self setExperimentName:@"Experiment Load Failed..."];
}

- (void)enforceRunningState{
	[self setServerConnected:YES];
	[self setServerConnecting:NO];
	[self setExperimentLoaded:YES];
	[self setHasExperimentLoadErrors:NO];
	[self setExperimentLoading:NO];
	[self setExperimentRunning:YES];
	[self setExperimentPaused:NO];
}

	
- (void)connect{
	
	[self setServerConnecting:YES];
	
	BOOL success = [self.client connectToServer:serverURL port:serverPort.integerValue];
	// If that didn't work, try launching the server remotely
	if((!success || !(self.client.connected)) && [self launchIfNeeded]){
		//NSLog(@"Attempting to remotely launch server");
	
		if([[self serverURL] isEqualToString:@"127.0.0.1"] || [[self serverURL] isEqualToString:@"localhost"]){

			[NSTask launchedTaskWithLaunchPath:@"/usr/bin/open"
                                     arguments:[NSArray arrayWithObject:@"/Applications/MWServer.app"]];

		} else {

			[NSTask launchedTaskWithLaunchPath:@"/usr/bin/ssh" arguments:[NSArray arrayWithObjects:
                                                                          @"-l",
                                                                          NSUserName(),
                                                                          [self serverURL],
                                                                          @"/usr/bin/open",
                                                                          @"/Applications/MWServer.app",
                                                                          nil]];

		}

		[NSThread sleepForTimeInterval:4];
		success = [self.client connectToServer:serverURL port:serverPort.integerValue];
	}
	
	
	// start checking to see if connected
	if(success && self.client.connected){
		
		[self enforceConnectedState];
	
		// get the name of the remote setup
		[self setServerName:@"Unnamed Setup"];
		
		if ([self serverURL] != nil) {
            NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
            NSArray *recentServers = [defaults arrayForKey:@"recentServers"];
            NSMutableArray *recentServersMutable;
            
            if (recentServers) {
                recentServersMutable = [recentServers mutableCopy];
            } else {
                recentServersMutable = [[NSMutableArray alloc] init];
            }
            
            [recentServersMutable removeObject:[self serverURL]];  // In case it's already in the list
            [recentServersMutable insertObject:[self serverURL] atIndex:0];
            
			[defaults setObject:recentServersMutable forKey:@"recentServers"];
			[defaults synchronize];
            
		}
		
		
		// get the name of the remote setup
		[self setServerName:[self.client valueForTag:@"#serverName"].stringValue];
		if([self serverName] == Nil || [[self serverName] length] == 0){
			[self setServerName:@"Unnamed Server"];
		}
		
		
	} else {
		[self enforceDisconnectedState];
	}
	
}

- (void)disconnect{
	

	[self setServerConnecting:YES];
	
	//[self willChangeValueForKey:@"serverConnected"];
	[self.client disconnect];
	[NSThread sleepForTimeInterval:0.5];
	
	// start checking to see if connected
	if(!(self.client.connected)){
		[self setServerConnected:NO];
		[self setServerConnecting:NO];
		
		[self enforceDisconnectedState];
	} else {
		[self setServerConnecting:NO];
	}

}

- (void) loadExperiment {
	
	[self setExperimentLoadProgress:[NSNumber numberWithDouble:0.0]];
	
	[self setExperimentName:@"Loading Experiment..."];
	[self setExperimentLoading:YES];
    self.clientsideExperimentPath = self.experimentPath;
	[self updateRecentExperiments];
	
    [self.client registerCallbackWithKey:CLIENT_LOAD_MESSAGE_CALLBACK_KEY
                                  forTag:@"#announceMessage"
                                callback:^(MWKEvent *event) {
                                    dispatch_async(dispatch_get_main_queue(), ^{
                                        [self handleLoadMessageEvent:event];
                                    });
                                }];
	
	[self startAccumulatingErrors];
  
	bool success = [self.client sendExperiment:self.experimentPath];

    if(!success){
        [self setExperimentLoading:NO];
    }
}

- (void) updateRecentExperiments {
    if ([self experimentPath]) {
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        
        [defaults setObject:[self experimentPath] forKey:@"lastExperiment"];
        
        NSArray *recentExperiments = [defaults arrayForKey:@"recentExperiments"];
        NSMutableArray *recentExperimentsMutable;
        
        if (recentExperiments) {
            recentExperimentsMutable = [recentExperiments mutableCopy];
        } else {
            recentExperimentsMutable = [[NSMutableArray alloc] init];
        }
        
        [recentExperimentsMutable removeObject:[self experimentPath]];  // In case it's already in the list
        [recentExperimentsMutable insertObject:[self experimentPath] atIndex:0];
        while ([recentExperimentsMutable count] > MAX_NUM_RECENT_EXPERIMENTS) {
            [recentExperimentsMutable removeLastObject];
        }
        
        [defaults setObject:recentExperimentsMutable forKey:@"recentExperiments"];
        
        [defaults synchronize];
    }
}


- (void) closeExperiment {
	NSString *experiment_path = [self experimentPath];
	
	if(experiment_path == NULL){
		// TODO: raise an error
		experiment_path = @"";
	}

#ifndef HOLLOW_OUT_FOR_ADC
	[self.client sendCloseExperimentEvent:experiment_path];
#endif
  
	[self setExperimentLoading:NO];
	[self setExperimentPath:Nil];
	[self setExperimentName:Nil];
    
    if (appController.shouldAutoClosePluginWindows) {
        [self hideAllPlugins];
    }
}


- (void) saveVariableSet {
    NSString *variable_save_name = [self variableSetName];
    if(variable_save_name != Nil){
        [self.client sendSaveVariablesEvent:variable_save_name overwrite:YES];
    }
}

- (void) loadVariableSet {
    NSString *variable_load_name = [self variableSetName];
    if(variable_load_name != Nil){
        [self.client sendLoadVariablesEvent:variable_load_name];
    }
}

- (void) openDataFile {

#ifndef HOLLOW_OUT_FOR_ADC
	NSString *filename = [self dataFileName];
	BOOL overwrite = [self dataFileOverwrite];
	
	[self.client sendOpenDataFileEvent:filename overwrite:overwrite];

    [self.notebook addEntry:[NSString stringWithFormat:@"Streaming to data file %@", filename, Nil]];
#endif
}


- (void) closeDataFile {
#ifndef HOLLOW_OUT_FOR_ADC
	NSString *filename = [self dataFileName];
	
	[self.client sendCloseDataFileEvent:filename];
    
    [self.notebook addEntry:[NSString stringWithFormat:@"Closing data file %@", filename, Nil]];
#endif
}



- (void)toggleExperimentRunning:(id)running{
	
	BOOL isit = [running boolValue];

#ifndef HOLLOW_OUT_FOR_ADC
	if(isit){
		//stop
		[self.client sendStopEvent];
        [self.notebook addEntry:@"Experiment stopped"];
	} else {
		//start
        
        if (!(self.dataFileWillAutoOpen || self.dataFileOpen) && appController.shouldAutoOpenDataFile) {
            NSString *userName = NSUserName();
            NSString *experimentBaseName = self.clientsideExperimentPath.lastPathComponent.stringByDeletingPathExtension;
            
            NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
            dateFormatter.dateFormat = @"yyyyMMdd-HHmmss";
            NSString *dateTag = [dateFormatter stringFromDate:[NSDate date]];
            
            self.dataFileName = [NSString stringWithFormat:@"%@-%@-%@", userName, experimentBaseName, dateTag];
            self.dataFileOverwrite = NO;
            [self openDataFile];
        }
        
		if([self currentProtocolName] != Nil){
			[self.client sendProtocolSelectedEvent:self.currentProtocolName];
		}
		[self.client sendRunEvent];
        [self.notebook addEntry:@"Experiment started"];
	}
#endif
}


- (void)toggleExperimentPaused:(id)paused{
	
	BOOL isit = [paused boolValue];
    
#ifndef HOLLOW_OUT_FOR_ADC
	if(isit){
		//resume
        [self.client sendResumeEvent];
        [self.notebook addEntry:@"Experiment resumed"];
	} else {
		//pause
        [self.client sendPauseEvent];
        [self.notebook addEntry:@"Experiment paused"];
	}
#endif
}


- (void)receiveCodec:(MWKEvent *)event {
    // If we've received a new codec, it is possible that some key variables have changed codes
    // As a result, we must find out what they are, and re-register callbacks
    
    [self.client unregisterCallbacksWithKey:EXPERIMENT_LOAD_PROGRESS_KEY];
    [self.client registerCallbackWithKey:EXPERIMENT_LOAD_PROGRESS_KEY
                                  forTag:@"#experimentLoadProgress"
                                callback:^(MWKEvent *event) {
                                    NSNumber *numberValue = event.data.numberValue;
                                    if (numberValue) {
                                        dispatch_async(dispatch_get_main_queue(), ^{
                                            self.experimentLoadProgress = numberValue;
                                        });
                                    }
                                }];
}


- (void)handleSystemEvent:(MWKEvent *)event {
    MWKDatum *data = event.data;
    NSNumber *payloadType = data[MWKSystemEventKeyPayloadType].numberValue;
    
    if (payloadType) {
        NSInteger payloadTypeValue = payloadType.integerValue;
        if (payloadTypeValue == MWKSystemEventPayloadTypeProtocolPackage) {
            MWKDatum *payload = data[MWKSystemEventKeyPayload];
            NSArray<MWKDatum *> *protocols = payload[MWKSystemEventPayloadKeyProtocols].listValue;
            
            if (protocols && protocols.count > 0) {
                [protocolNames removeAllObjects];
                
                for (MWKDatum *protocol in protocols) {
                    NSString *protocolName = protocol[MWKSystemEventPayloadKeyProtocolName].stringValue;
                    if (protocolName) {
                        [protocolNames addObject:protocolName];
                    }
                }
                
                [self willChangeValueForKey:@"protocolNames"];
                [self didChangeValueForKey:@"protocolNames"];
                
                self.currentProtocolName = payload[MWKSystemEventPayloadKeyCurrentProtocolName].stringValue;
                
                if(currentProtocolName == Nil || ([protocolNames indexOfObject:currentProtocolName] == NSNotFound)){
                    [self setCurrentProtocolName:[protocolNames objectAtIndex:0]];
                }
            }
        } else if (payloadTypeValue == MWKSystemEventPayloadTypeDataFileOpened) {
            MWKDatum *payload = data[MWKSystemEventKeyPayload];
            
            // TODO: kludge MUST be fixed
            NSString *filename = payload[1].stringValue;
            NSNumber *success = payload[0].numberValue;
            
            if (!(success && success.integerValue == MWKSystemEventResponseCodeCommandSuccess)) {
                [self setDataFileName:Nil];
                [self setDataFileOpen:NO];
            } else {
                self.dataFileName = filename;
                if([self dataFileName] != Nil){
                    [self setDataFileOpen: YES];
                }
            }
        } else if (payloadTypeValue == MWKSystemEventPayloadTypeDataFileClosed) {
            [self setDataFileName:Nil];
            [self setDataFileOpen: NO];
        } else if (payloadTypeValue == MWKSystemEventPayloadTypeExperimentState) {
            MWKDatum *state = data[MWKSystemEventKeyPayload];
            
            self.experimentLoaded = state[MWKSystemEventPayloadKeyLoaded].numberValue.boolValue;
            if([self experimentLoaded]) {
                [self setExperimentLoading:NO];
                [self.client unregisterCallbacksWithKey:CLIENT_LOAD_MESSAGE_CALLBACK_KEY];
                self.experimentPaused = state[MWKSystemEventPayloadKeyPaused].numberValue.boolValue;
                self.experimentRunning = state[MWKSystemEventPayloadKeyRunning].numberValue.boolValue;
                self.experimentName = state[MWKSystemEventPayloadKeyExperimentName].stringValue;
                
                if([self experimentName] == Nil){
                    [self setExperimentName:@"Unnamed Experiment"];
                }
                
                self.experimentPath = state[MWKSystemEventPayloadKeyExperimentPath].stringValue;
                
                [self willChangeValueForKey:@"serversideVariableSetNames"];
                
                [serversideVariableSetNames removeAllObjects];
                NSArray<MWKDatum *> *svs = state[MWKSystemEventPayloadKeySavedVariableSetNames].listValue;
                if (svs) {
                    for (MWKDatum *set in svs) {
                        NSString *setName = set.stringValue;
                        if (setName) {
                            [serversideVariableSetNames addObject:setName];
                        }
                    }
                }
                
                [self didChangeValueForKey:@"serversideVariableSetNames"];
                
                self.variableSetName = state[MWKSystemEventPayloadKeyCurrentSavedVariableSetName].stringValue;
                if (self.variableSetName) {
                    self.variableSetLoaded = YES;
                }
                
                self.dataFileWillAutoOpen = state[MWKSystemEventPayloadKeyDataFileAutoOpen].numberValue.boolValue;
                
                self.dataFileName = state[MWKSystemEventPayloadKeyDataFileName].stringValue;
                if (self.dataFileName) {
                    [self setDataFileOpen:YES];
                }
                
                if(accumulatingErrors){
                    [self stopAccumulatingErrors];
                }
                
            } else {
                // Not loaded
                if(accumulatingErrors){
                    [self stopAccumulatingErrors];
                }
                
                if([self hasExperimentLoadErrors]){
                    [self enforceLoadFailedState];
                } else if (!self.experimentLoading) {
                    [self enforceConnectedState];
                }
            }
        }
    }
}


- (void) loadPlugins {

    // create a carrier window to hold all of the plugins when they are grouped
    grouped_plugin_controller = [[MWGroupedPluginWindowController alloc] initWithClientInstance:self];
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    [NSBundle loadNibNamed:@"GroupedPluginsWindow" owner:grouped_plugin_controller];
#pragma clang diagnostic pop
    [grouped_plugin_controller loadWindow];
    [[grouped_plugin_controller window] orderOut:self];
    [grouped_plugin_controller setCustomColor:self.headerColor];
    [grouped_plugin_controller setWindowFrameAutosaveName:self.serverURL];
    self.groupedPluginWindow = grouped_plugin_controller.window;
    
    // Add console, which is always available
    MWConsoleController *console = [[MWConsoleController alloc] init];
    [console setDelegate:self];
    [console.window bind:@"displayPatternTitle1"
                toObject:self
             withKeyPath:@"serverURL"
                 options:@{NSDisplayPatternBindingOption: @"Console (%{title1}@)"}];
    [self addPlugin:console withName:@"Console"];
  
    NSFileManager *fm = [NSFileManager defaultManager];
	
	NSError *error = NULL;
	NSString *plugin_directory = @"/Library/Application Support/MWorks/Plugins/Client/";
	NSArray *plugins = [fm contentsOfDirectoryAtPath:plugin_directory error:&error];
	
	for(int i = 0; i < [plugins count]; i++){
		NSString *plugin_file = [plugins objectAtIndex:i];
		//NSLog(@"%@", plugin_file);
		
		NSString *fullpath = [plugin_directory stringByAppendingString:plugin_file];
		NSBundle *plugin_bundle = [[NSBundle alloc] initWithPath:fullpath];
		
        BOOL loaded = false;
        loaded = [plugin_bundle loadAndReturnError:&error];
        
        if(loaded){
      
            NSArray *toplevel;
            NSNib *nib;
      
            BOOL nib_loaded_correctly = true;
            @try{
                nib = [[NSNib alloc] initWithNibNamed:@"Main" bundle:plugin_bundle];
        
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
                // TODO: Replace instantiateNibWithOwner:topLevelObjects: with
                // instantiateWithOwner:topLevelObjects:.  This will require a bit of work,
                // as the latter method requires the caller to explicitly retain all instantiated
                // objects.
                if(![nib instantiateNibWithOwner:self topLevelObjects:&toplevel]){
#pragma clang diagnostic pop
                    NSLog(@"Couldn't instantiate nib for %@", plugin_file);
                    nib_loaded_correctly = false;
                }
            } @catch(NSException *e){
                NSLog(@"exception while loading nib: %@", [e reason]);  
                nib_loaded_correctly = false;
            }
			
            if(!nib_loaded_correctly){
                continue;
            }
      
			for(int j=0; j < [toplevel count]; j++){
				NSObject *obj = [toplevel objectAtIndex:j];
				//NSLog(@"object = %d", obj);
				if([obj isKindOfClass:[NSWindowController class]]){
                    [self addPlugin:(NSWindowController *)obj withName:[plugin_file stringByDeletingPathExtension]];
					//NSLog(@"object is a controller (%d)", [pluginWindows count]);
					break;
				}
			}
			
			
        } else {
            if(error){
                NSLog(@"Couldn't load bundle: %@: %@", [error localizedDescription], [error userInfo]);
            } else {
                NSLog(@"Couldn't load bundle: %@", fullpath);
            }
		}
	}
	
	//NSLog(@"%d plugin windows in instance %d", [pluginWindows count], self);
    
    if (appController.shouldRestoreOpenPluginWindows) {
        [self restoreOpenPluginWindows];
    }
}


- (void)addPlugin:(NSWindowController *)controller withName:(NSString *)name {
    [controller loadWindow];
    [controller setWindowFrameAutosaveName:name];
    [pluginWindows addObject:controller];
    
    // also, add the window to the grouped_plugin_controller
    [grouped_plugin_controller addPluginWindow:[controller window] withName:[[controller window] title]];
    [grouped_plugin_controller setCurrentPluginIndex:0];
}


- (NSArray *)pluginWindows {
	return pluginWindows;
}

- (void)showPlugin:(int)i {
    if(i == [grouped_plugin_controller currentPluginIndex]){
        [grouped_plugin_controller hideWindow];
        //[grouped_plugin_controller incrementPlugin:self];
    }
	
    id controller = [pluginWindows objectAtIndex:i];
    if([controller respondsToSelector:@selector(setInGroupedWindow:)]){
        [(id<MWClientPluginWindowController>)controller setInGroupedWindow:NO];
    }
    
    [[controller window] orderFront:self];
}

- (void)showAllPlugins {
	
    [grouped_plugin_controller hideWindow];
    
	for(int i = 0; i < [pluginWindows count]; i++){
		NSWindowController *controller = [pluginWindows objectAtIndex:i];
		[[controller window] orderFront:self];
	}

	//NSLog(@"showing all (%d) from instance %d", [pluginWindows count], self);
}

- (void)showGroupedPlugins {
    [self hideAllPlugins];
	
    [[grouped_plugin_controller window] orderFront:self];
    [grouped_plugin_controller setCurrentPluginIndex: 0];
}

- (void)hideAllPlugins {
	[grouped_plugin_controller hideWindow];
    
	for(int i = 0; i < [pluginWindows count]; i++){
		NSWindowController *controller = [pluginWindows objectAtIndex:i];
		[[controller window] orderOut:self];
	}
}


- (NSArray *)openPluginWindows {
    NSMutableArray *openPluginWindows = [NSMutableArray array];
    
    for (NSWindowController *controller in pluginWindows) {
        NSString *autoSaveName = [controller windowFrameAutosaveName];
        if ([[controller window] isVisible]) {
            [openPluginWindows addObject:autoSaveName];
        }
    }
    
    return openPluginWindows;
}


- (void)setOpenPluginWindows:(NSArray *)openPluginWindows {
    for (int i = 0; i < [pluginWindows count]; i++) {
        NSWindowController *controller = [pluginWindows objectAtIndex:i];
        if ([openPluginWindows containsObject:[controller windowFrameAutosaveName]]) {
            [self showPlugin:i];
        }
    }
}


- (void)saveOpenPluginWindows {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setBool:[[grouped_plugin_controller window] isVisible] forKey:DEFAULTS_GROUPED_PLUGIN_WINDOW_IS_OPEN_KEY];
    [defaults setObject:[self openPluginWindows] forKey:DEFAULTS_OPEN_PLUGIN_WINDOWS_KEY];
}


- (void)restoreOpenPluginWindows {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    if ([defaults boolForKey:DEFAULTS_GROUPED_PLUGIN_WINDOW_IS_OPEN_KEY]) {
        [self showGroupedPlugins];
    }
    
    NSArray *openPluginWindows = [defaults arrayForKey:DEFAULTS_OPEN_PLUGIN_WINDOWS_KEY];
    if (openPluginWindows) {
        [self setOpenPluginWindows:openPluginWindows];
    }
}


- (void)startAccumulatingErrors{
	[self clearAccumulatedErrors];
	
	accumulatingErrors = YES;
	
    [self.client registerCallbackWithKey:ERROR_MESSAGE_CALLBACK_KEY
                                  forTag:@"#announceMessage"
                                callback:^(MWKEvent *event) {
                                    dispatch_async(dispatch_get_main_queue(), ^{
                                        [self handleErrorMessageEvent:event];
                                    });
                                }];
}

- (void)stopAccumulatingErrors{
	[self.client unregisterCallbacksWithKey:ERROR_MESSAGE_CALLBACK_KEY];
	
	[appController willChangeValueForKey:@"modalClientInstanceInCharge"];
	[appController didChangeValueForKey:@"modalClientInstanceInCharge"];
	
	[self willChangeValueForKey:@"errors"];
	[self didChangeValueForKey:@"errors"];
	
    NSAttributedString *errorString = [[NSAttributedString alloc] initWithString:[errors componentsJoinedByString:@"\n"]
                                                                      attributes:@{ NSForegroundColorAttributeName: [NSColor textColor] }];
	[self setErrorString:errorString];
	
	accumulatingErrors = NO;	
}


- (void)handleLoadMessageEvent:(MWKEvent *)event{
    MWKDatum *data = event.data;
    if (data.dataType != MWKDatumDataTypeDictionary) {
        return;
    }
    
    NSString *message = data[MWKMessageKeyMessage].stringValue;
    if (!(message && message.length > 0)) {
        return;
    }
    
    self.experimentLoadMessage = message;
}

- (void)handleErrorMessageEvent:(MWKEvent *)event{
    MWKDatum *data = event.data;
    if (data.dataType != MWKDatumDataTypeDictionary) {
        return;
    }
    
    NSString *message = data[MWKMessageKeyMessage].stringValue;
    if (!(message && message.length > 0)) {
        return;
    }
    
    NSInteger messageType = data[MWKMessageKeyType].numberValue.integerValue;
    if (messageType != MWKMessageTypeError) {
        return;
    }
    
    [errors addObject:message];
    
    [self setHasExperimentLoadErrors:YES];
}

- (void)clearAccumulatedErrors{
	[errors removeAllObjects];
	[self setHasExperimentLoadErrors:NO];
}


- (NSDictionary *)workspaceInfo {
    NSMutableDictionary *workspaceInfo = [NSMutableDictionary dictionary];
    
    [workspaceInfo setObject:self.serverURL forKey:@"serverURL"];
    [workspaceInfo setObject:self.serverPort forKey:@"serverPort"];
    // clientsideExperimentPath will be nil when the client connects to a server
    // on which an experiment is already loaded
    if (self.clientsideExperimentPath) {
        [workspaceInfo setObject:self.clientsideExperimentPath forKey:@"experimentPath"];
    }
    if (self.variableSetLoaded) {
        [workspaceInfo setObject:self.variableSetName forKey:@"variableSetName"];
    }
    [workspaceInfo setObject:[self openPluginWindows] forKey:@"openPluginWindows"];
    
    NSMutableDictionary *pluginState = [NSMutableDictionary dictionary];
    
    for (NSWindowController *controller in pluginWindows) {
        if ([controller respondsToSelector:@selector(workspaceState)]) {
            NSDictionary *state = [(id<MWClientPluginWorkspaceState>)controller workspaceState];
            if ([state count] > 0) {
                [pluginState setObject:state forKey:[controller windowFrameAutosaveName]];
            }
        }
    }
    
    [workspaceInfo setObject:pluginState forKey:@"pluginState"];
    
    return workspaceInfo;
}


- (void)loadWorkspace:(NSDictionary *)workspaceInfo {
    NSString *newServerURL = [workspaceInfo objectForKey:@"serverURL"];
    NSNumber *newServerPort = [workspaceInfo objectForKey:@"serverPort"];
    if (newServerURL && [newServerURL isKindOfClass:[NSString class]] &&
        newServerPort && [newServerPort isKindOfClass:[NSNumber class]])
    {
        BOOL mustConnect = YES;
        
        if (self.serverConnected) {
            if ([self.serverURL isEqualToString:newServerURL] &&
                [self.serverPort isEqualToNumber:newServerPort])
            {
                mustConnect = NO;
            } else {
                [self disconnect];
            }
        }
        
        if (mustConnect) {
            self.serverURL = newServerURL;
            self.serverPort = newServerPort;
            [self connect];
        }
    }
    
    if (!self.serverConnected) {
        return;
    }
    
    NSString *newExperimentPath = [workspaceInfo objectForKey:@"experimentPath"];
    if (newExperimentPath && [newExperimentPath isKindOfClass:[NSString class]]) {
        self.experimentPath = [newExperimentPath mwk_absolutePath];
        [self loadExperiment];
    }
    
    NSString *newVariableSetName = [workspaceInfo objectForKey:@"variableSetName"];
    if (newVariableSetName && [newVariableSetName isKindOfClass:[NSString class]]) {
        self.variableSetName = newVariableSetName;
        [self loadVariableSet];
    }
    
    NSArray *newOpenPluginWindows = [workspaceInfo objectForKey:@"openPluginWindows"];
    if (newOpenPluginWindows && [newOpenPluginWindows isKindOfClass:[NSArray class]]) {
        [self setOpenPluginWindows:newOpenPluginWindows];
    }
    
    NSDictionary *newPluginState = [workspaceInfo objectForKey:@"pluginState"];
    if (newPluginState && [newPluginState isKindOfClass:[NSDictionary class]]) {
        for (NSWindowController *controller in pluginWindows) {
            NSDictionary *state = [newPluginState objectForKey:[controller windowFrameAutosaveName]];
            if (state &&
                [state isKindOfClass:[NSDictionary class]] &&
                [state count] > 0 &&
                [controller respondsToSelector:@selector(setWorkspaceState:)])
            {
                [(id<MWClientPluginWorkspaceState>)controller setWorkspaceState:state];
            }
        }
    }
}


- (void)closeWorkspace {
    if (self.experimentRunning) {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Experiment is running"];
        [alert setInformativeText:@"Please stop the running experiment before attempting to close the workspace."];
        [alert runModal];
        return;
    }
    
    if (self.dataFileOpen) {
        [self closeDataFile];
    }
    
    if (self.experimentLoaded) {
        [self closeExperiment];
    }
    
    [self hideAllPlugins];
}


@end



























