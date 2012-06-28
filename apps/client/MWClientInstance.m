//
//  MWClientInstance.m
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWClientInstance.h"
#import <MWorksCore/StandardVariables.h>
#import <MWorksCore/Client.h>
#import <MWorksCocoa/MWCocoaEvent.h>
#import <MWorksCocoa/MWCocoaEventFunctor.h>
#import <MWorksCocoa/MWWindowController.h>
#import <MWorksCocoa/MWNotebook.h>

#define ERROR_MESSAGE_CALLBACK_KEY	"MWClientInstance::client_error_message_callback"
#define CLIENT_SYSTEM_EVENT_CALLBACK_KEY  "MWClientInstance::system_event_callback"
#define EXPERIMENT_LOAD_PROGRESS_KEY "MWClientInstance::experiment_load_progress_callback"
#define CLIENT_LOAD_MESSAGE_CALLBACK_KEY    "MWClientInstance::load_message_callback"
#define CLIENT_CODEC_EVENT_CALLBACK_KEY "MWClientInstance::codec_callback"

#define DEFAULTS_OPEN_PLUGIN_WINDOWS_KEY @"openPluginWindows"
#define DEFAULTS_GROUPED_PLUGIN_WINDOW_IS_OPEN_KEY @"groupedPluginWindowIsOpen"



@implementation MWClientInstance


- (id)initWithAppController:(AppController *)_controller {

	appController = _controller;

	accumulatingErrors = NO;
	
    // Cosmetics
    headerColor = [appController uniqueColor];
    [headerBox setFillColor:headerColor];


	// TODO: this can theoretically throw; display an error without bringing down the whole show
    core = shared_ptr <mw::Client>(new mw::Client());
    
    variables = [[MWCodec alloc] initWithClientInstance:self];
	
	protocolNames = [[NSMutableArray alloc] init];
	errors = [[NSMutableArray alloc] init];
    serversideVariableSetNames = [[NSMutableArray alloc] init];
	
	serverURL = @"127.0.0.1";
	serverPort = [[NSNumber numberWithInteger:19989] retain];
	
    variableSetName = Nil;
	self.variableSetName = Nil;
    self.variableSetLoaded = NO;
    
    // Register a callback to receive codec events
	[self registerEventCallbackWithReceiver:self 
                                   selector:@selector(receiveCodec:)
                                callbackKey:CLIENT_CODEC_EVENT_CALLBACK_KEY
                            forVariableCode:RESERVED_CODEC_CODE
                               onMainThread:NO];
    
    [self registerEventCallbackWithReceiver:self 
                                   selector:@selector(handleSystemEvent:)
                                callbackKey:CLIENT_SYSTEM_EVENT_CALLBACK_KEY
                            forVariableCode:RESERVED_SYSTEM_EVENT_CODE
                               onMainThread:YES];
									 
	[self setDataFileOpen:false];
	[self setExperimentLoaded:false];
	[self setServerConnected:false];
	[self setLaunchIfNeeded:YES];
	
    // create a notebook
    notebook = [[MWNotebook alloc] init];
    
	// load plugins
	pluginWindows = [[NSMutableArray alloc] init];
	[self loadPlugins];
	
    
    // TODO: this can throw; display an error without bringing down the whole show
    core->startEventListener();
	
  
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



@synthesize variables;

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
@synthesize experimentPath;
@synthesize hasExperimentLoadErrors;
@synthesize errors;
@synthesize errorString;

// Variable set
@synthesize variableSetName;
@synthesize serversideVariableSetNames;
@synthesize variableSetLoaded;

// Data File state
@synthesize dataFileOpen;
@synthesize dataFileName;
@synthesize dataFileOverwrite;

// Protocol state
@synthesize protocolNames;
@synthesize currentProtocolName;
@synthesize summaryString;

// Cosmetics
@synthesize headerColor;

- (shared_ptr<mw::Client>)coreClient {
  return core;
}

- (MWNotebook *)notebook {
    return notebook;
}

- (shared_ptr<mw::EventStreamInterface>) eventStreamInterface{
    return static_pointer_cast<EventStreamInterface>(core);
}

- (NSArray *)variableNames {
	return [variables variableNames];
}

- (NSDictionary *)varGroups {
  
	unsigned int nVars = core->numberOfVariablesInRegistry();
	
	NSMutableDictionary *allGroups = [[NSMutableDictionary alloc] init];
	

  
	for(int i=N_RESERVED_CODEC_CODES; i < nVars + N_RESERVED_CODEC_CODES; ++i) {
		shared_ptr <mw::Variable> var = core->getVariable(i);
		
        if((var == NULL) || (var->getProperties() == NULL)){
            continue;
        }
        
		std::vector <std::string> groups(var->getProperties()->getGroups());
		std::vector<std::string>::iterator iter = groups.begin();
		while (iter != groups.end()) {
			NSString *groupName = [[NSString alloc] initWithCString:iter->c_str() 
														   encoding:NSASCIIStringEncoding];
			
			
			
			NSMutableArray *aGroup = [allGroups valueForKey:groupName];
			if(aGroup == nil) {
				aGroup = [[NSMutableArray alloc] init];
			}
			
			[aGroup addObject:[NSString stringWithCString:var->getProperties()->getTagName().c_str() 
												 encoding:NSASCIIStringEncoding]];
			
			[allGroups setValue:aGroup forKey:groupName];
			++iter;
		}
	}
	
	return allGroups;
}


- (void)setServerURL:(NSString *)newServerURL {
    if (![serverURL isEqualToString:newServerURL]) {
        [serverURL release];
        serverURL = [newServerURL copy];
        [grouped_plugin_controller setWindowFrameAutosaveName:serverURL];
    }
}


- (void)checkConnection {
  
  @synchronized(self){
  
    BOOL actually_connected = core->isConnected();

	if(actually_connected != [self serverConnected]){
		[self setServerConnected:actually_connected];
		
		if(![self serverConnected]){
			[self disconnect];
		}
	} else if([self serverConnected]){
        NSString *putative_name = [variables valueForKey:@"_serverName"];
        if(putative_name != [self serverName] && putative_name != Nil){
            [self setServerName: putative_name];
        }
    }
	
  }
//	[self enforceConnectedState];
//	[self setServerName:[variables valueForKey:@"_serverName"]];
}

// Maintaining Overall GUI state
- (void)enforceDisconnectedState{
	[self setServerConnected:NO];
	[self setServerConnecting:NO];
	[self setExperimentLoaded:NO];
	[self setHasExperimentLoadErrors:NO];
	[self setExperimentLoading:NO];
	[self setExperimentRunning:NO];
	
	[self setExperimentName:@""];
	[self setExperimentPath:@""];
    
    if (appController.shouldAutoClosePluginWindows) {
        [self hideAllPlugins];
    }
	
	[self setServerName:Nil];
    [self setVariableSetName:Nil];
    
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
}

- (void)enforceLoadFailedState{
	[self setServerConnected:YES];
	[self setServerConnecting:NO];
	[self setExperimentLoaded:NO];
	[self setHasExperimentLoadErrors:YES];
	[self setExperimentLoading:NO];
	[self setExperimentRunning:NO];
	[self setExperimentName:@"Experiment Load Failed..."];
}

- (void)enforceRunningState{
	[self setServerConnected:YES];
	[self setServerConnecting:NO];
	[self setExperimentLoaded:YES];
	[self setHasExperimentLoadErrors:NO];
	[self setExperimentLoading:NO];
	[self setExperimentRunning:YES];
}

	
- (void)connect{
	
	string url = [serverURL cStringUsingEncoding:NSASCIIStringEncoding];
	[self setServerConnecting:YES];
	
	BOOL success = core->connectToServer(url, [serverPort intValue]);  
	// If that didn't work, try launching the server remotely
	if((!success || !core->isConnected()) && [self launchIfNeeded]){
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
		success = core->connectToServer(url, [serverPort intValue]);
	}
	
	
	// start checking to see if connected
	if(success && core->isConnected()){
		
		[self enforceConnectedState];
	
		// get the name of the remote setup
		[self setServerName:@"Unnamed Setup"];
		
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		NSArray *recentServers = [defaults arrayForKey:@"recentServers"];
		if([self serverURL] != Nil && ![recentServers containsObject:[self serverURL]]){
			NSMutableArray *recentServersMutable = [[NSMutableArray alloc] init];
			[recentServersMutable addObjectsFromArray:recentServers];
			[recentServersMutable addObject:serverURL];
			[defaults setObject:recentServersMutable forKey:@"recentServers"];
			[defaults synchronize];
		}
		
		
		// get the name of the remote setup
		[self setServerName:[variables valueForKey:@"_serverName"]];
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
	core->disconnectClient();
	[NSThread sleepForTimeInterval:0.5];
	
	// start checking to see if connected
	if(!core->isConnected()){
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
	string path = [[self experimentPath] cStringUsingEncoding:NSASCIIStringEncoding];
	[self setExperimentLoading:YES];
	[self updateRecentExperiments];
	
	[self registerEventCallbackWithReceiver:self 
                                   selector:@selector(handleLoadMessageEvent:)
                                callbackKey:CLIENT_LOAD_MESSAGE_CALLBACK_KEY
                                forVariable:@"#announceMessage"
                               onMainThread:YES];
	
	// Bind the messages variable temporarily to the loadingMessages key
//	[self registerBindingsBridgeWithReceiver:self 
//							bindingsKey:@"experimentLoadMessage"
//								callbackKey:LOAD_MESSAGE_CALLBACK_KEY
//								forVariable:@"#announceMessage"];
	
	[self startAccumulatingErrors];
  
	bool success = core->sendExperiment(path);

    if(!success){
        [self setExperimentLoading:NO];
    }
}

- (void) updateRecentExperiments {
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    if([self experimentPath] != Nil){
        [defaults setObject:[self experimentPath] forKey:@"lastExperiment"];
        [defaults synchronize];
    }
    NSArray *recentExperiments = [defaults arrayForKey:@"recentExperiments"];
    if([self experimentPath] != Nil){
      NSMutableArray *recentExperimentsMutable = [NSMutableArray arrayWithArray:recentExperiments];
      [recentExperimentsMutable removeObject:[self experimentPath]];  // In case it's already in the list
      [recentExperimentsMutable insertObject:[self experimentPath] atIndex:0];
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

	string path = [experiment_path cStringUsingEncoding:NSASCIIStringEncoding];
	
#ifndef HOLLOW_OUT_FOR_ADC
	core->sendCloseExperimentEvent(path);
#endif
  
	[self setExperimentLoading:NO];
	[self setExperimentPath:Nil];
	[self setExperimentName:Nil];
    
    if (appController.shouldAutoClosePluginWindows) {
        [self hideAllPlugins];
    }
}


- (void) saveVariableSet {

#ifndef HOLLOW_OUT_FOR_ADC
    NSString *variable_save_name = [self variableSetName];
    if(variable_save_name != Nil){
        core->sendSaveVariablesEvent([variable_save_name cStringUsingEncoding:NSASCIIStringEncoding], 1);
        [notebook addEntry:[NSString stringWithFormat:@"Loaded variable set %@", variable_save_name, Nil]];
    } else {
        // TODO
    }
#endif
}

- (void) loadVariableSet {
#ifndef HOLLOW_OUT_FOR_ADC
    NSString *variable_load_name = [self variableSetName];
    if(variable_load_name != Nil){
        core->sendLoadVariablesEvent([variable_load_name cStringUsingEncoding:NSASCIIStringEncoding]);
        // TODO: should get confirmation!
        self.variableSetLoaded = YES;
    } else {
        //TODO
    }
#endif
}

- (void) openDataFile {

#ifndef HOLLOW_OUT_FOR_ADC
	NSString *filename = [self dataFileName];
	BOOL overwrite = [self dataFileOverwrite];
	
	core->sendOpenDataFileEvent([filename cStringUsingEncoding:NSASCIIStringEncoding],
								[[NSNumber numberWithBool:overwrite] intValue]);

    [notebook addEntry:[NSString stringWithFormat:@"Streaming to data file %@", filename, Nil]];
#endif
}


- (void) closeDataFile {
#ifndef HOLLOW_OUT_FOR_ADC
	NSString *filename = [self dataFileName];
	
	core->sendCloseDataFileEvent([filename cStringUsingEncoding:NSASCIIStringEncoding]);
    
    [notebook addEntry:[NSString stringWithFormat:@"Closing data file %@", filename, Nil]];
#endif
}



- (void)toggleExperimentRunning:(id)running{
	
	BOOL isit = [running boolValue];

#ifndef HOLLOW_OUT_FOR_ADC
	if(isit){
		//stop
		core->sendStopEvent();
        [notebook addEntry:@"Experiment stopped"];
	} else {
		//start
		if([self currentProtocolName] != Nil){
			core->sendProtocolSelectedEvent([[self currentProtocolName] cStringUsingEncoding:NSASCIIStringEncoding]);
		}
		core->sendRunEvent();
        [notebook addEntry:@"Experiment started"];
	}
#endif
}


// Iteraction with core client

- (NSNumber *)codeForTag:(NSString *)tag {
	
  NSNumber *code = [[NSNumber alloc] initWithInt:core->lookupCodeForTag([tag cStringUsingEncoding:NSASCIIStringEncoding])];
  return code;
}

- (void)updateVariableWithCode:(int)code withData:(mw::Datum *)data {
	core->updateValue(code, *data);
}

- (void)updateVariableWithTag:(NSString *)tag withData:(mw::Datum *)data {
	[self updateVariableWithCode:[[self codeForTag:tag] intValue] withData:data];
}


- (void)unregisterCallbacksWithKey:(const char *)key {
	core->unregisterCallbacks(key, true); // short-circuit
}

- (void)unregisterCallbacksWithKey:(const char *)key locking:(BOOL)locking {
	core->unregisterCallbacks(key, true); // short-circuit
}


- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                             onMainThread:(BOOL)on_main{

	
    core->registerCallback(create_cocoa_event_callback(receiver, selector), string(key));
}

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                          forVariableCode:(int)code
                             onMainThread:(BOOL)on_main{
						  
	if(code >= 0) {
        core->registerCallback(code, create_cocoa_event_callback(receiver, selector, on_main, self), key);
	}
}

// Call receiver with a given selector (and the relevant event as an argument) if the mw variable 
// identified by the specified "tag" name changes
- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                              forVariable:(NSString *)tag 
                             onMainThread:(BOOL)on_main
{
	
    core->registerCallback([tag  cStringUsingEncoding:NSASCIIStringEncoding], 
                           create_cocoa_event_callback(receiver, selector, on_main, self), 
                           key);
}


// receiver: the object that will receive the KVC messages
// bindingsKey: the key (in the bindings sense of the word) in question
// callbackKey: a unique string to identify this particular callback
// forVariable: the tag name of the MWorks variable in question
- (void)registerBindingsBridgeWithReceiver:(id)receiver 
                               bindingsKey:(NSString *)bindings_key
                               callbackKey:(const char *)key
                               forVariable:(NSString *)tag {
	
	core->registerCallback([tag  cStringUsingEncoding:NSASCIIStringEncoding], 
                            create_bindings_bridge_event_callback(receiver, bindings_key), 
                            key);
}


- (void)receiveCodec:(MWCocoaEvent *)event {
    
    // If we've received a new codec, it is possible that some key variables have changed codes
    // As a result, we must find out what they are, and re-register callbacks

    [self unregisterCallbacksWithKey:EXPERIMENT_LOAD_PROGRESS_KEY locking:NO]; // locked from outside callback
    [self registerBindingsBridgeWithReceiver:self
                                 bindingsKey:@"experimentLoadProgress"
                                 callbackKey:EXPERIMENT_LOAD_PROGRESS_KEY
                                 forVariable:@"#experimentLoadProgress"]; // TODO: use EXPERIMENT_LOAD_PROGRESS_TAGNAME instead
    
}


- (void)handleSystemEvent:(MWCocoaEvent *)event {
    
    mw::Datum payload(*[event data]);
    mw::Datum sysEventType(payload.getElement(M_SYSTEM_PAYLOAD_TYPE));
    
    if(!sysEventType.isUndefined()) {
        // The F swich statement ... it's gotta be somewhere
        switch((mw::SystemPayloadType)sysEventType.getInteger()) {
            case M_PROTOCOL_PACKAGE:
            {
                mw::Datum pp(payload.getElement(M_SYSTEM_PAYLOAD));
                
                [protocolNames removeAllObjects];
                
                mw::Datum protocols(pp.getElement(M_PROTOCOLS));
                
                for(int i=0; i<protocols.getNElements(); ++i) {
                    mw::Datum protocol(protocols[i]);
                    
                    NSString *protocolName = [[NSString alloc] initWithCString:protocol.getElement(M_PROTOCOL_NAME).getString()
                                                                      encoding:NSASCIIStringEncoding];
                    
                    [protocolNames addObject:protocolName];
                }
                
                [self willChangeValueForKey:@"protocolNames"];
                [self didChangeValueForKey:@"protocolNames"];
                
                string current_protocol_string = pp.getElement(M_CURRENT_PROTOCOL).getString();
                
                [self setCurrentProtocolName:[[NSString alloc] initWithCString:current_protocol_string.c_str()
                                                                      encoding:NSASCIIStringEncoding]];
                
                if(currentProtocolName == Nil || ([protocolNames indexOfObject:currentProtocolName] == NSNotFound)){
                    [self setCurrentProtocolName:[protocolNames objectAtIndex:0]];
                }
            }
                break;
                
            case M_DATA_FILE_OPENED:
            {
                mw::Datum event(payload.getElement(M_SYSTEM_PAYLOAD));
                
                // TODO: kludge MUST be fixed
                mw::Datum file(event.getElement(1));
                mw::Datum success(event.getElement(0));
                
                if((int)success != M_COMMAND_SUCCESS){
                    [self setDataFileName:Nil];
                    [self setDataFileOpen:NO];
                    break;
                }
                
                [self setDataFileName:[NSString stringWithCString:file.getString() encoding:NSASCIIStringEncoding]];
                if([self dataFileName] != Nil){
                    [self setDataFileOpen: YES];
                }
            }
                break;
                
            case M_DATA_FILE_CLOSED:
            {
                mw::Datum event(payload.getElement(M_SYSTEM_PAYLOAD));
                
                // TODO: kludge MUST be fixed
                mw::Datum file(event.getElement(1));
                
                [self setDataFileName:Nil];
                [self setDataFileOpen: NO];
                
            }
                break;
                
            case M_EXPERIMENT_STATE:
            {
                mw::Datum state(payload.getElement(M_SYSTEM_PAYLOAD));
                
                [self setExperimentLoaded:state.getElement(M_LOADED).getBool()];
                if([self experimentLoaded]) {
                    [self setExperimentLoading:NO];
                    [self unregisterCallbacksWithKey:CLIENT_LOAD_MESSAGE_CALLBACK_KEY locking:NO]; // locked from outside callback
                    [self setExperimentPaused:state.getElement(M_PAUSED).getBool()];
                    [self setExperimentRunning: state.getElement(M_RUNNING).getBool()];
                    [self setExperimentName:[[NSString alloc] initWithCString:state.getElement(M_EXPERIMENT_NAME).getString()
                                                                     encoding:NSASCIIStringEncoding]];
                    
                    if([self experimentName] == Nil){
                        [self setExperimentName:@"Unnamed Experiment"];
                    }
                    
                    [self setExperimentPath:[[NSString alloc] initWithCString:state.getElement(M_EXPERIMENT_PATH).getString()
                                                                     encoding:NSASCIIStringEncoding]];
                    
                    
                    [self willChangeValueForKey:@"serversideVariableSetNames"];
                    
                    [serversideVariableSetNames removeAllObjects];
                    mw::Datum svs(state.getElement(M_SAVED_VARIABLES));
                    
                    if(svs.isList()) {
                        for(int i=0; i<svs.getNElements(); ++i) {
                            mw::Datum set(svs.getElement(i));
                            
                            [serversideVariableSetNames addObject:[NSString stringWithCString:set.getString()
                                                                                     encoding:NSASCIIStringEncoding]];
                        }
                    }
                    
                    [self didChangeValueForKey:@"serversideVariableSetNames"];
                    
                    if(accumulatingErrors){
                        [self stopAccumulatingErrors];
                    }
                    
                } else {
                    // Not loaded
                    if(accumulatingErrors){
                        [self stopAccumulatingErrors];
                    }
                    
                    [self setExperimentLoading:NO];
                    [self setExperimentRunning:NO];
                    
                    if([self hasExperimentLoadErrors]){
                        [self enforceLoadFailedState];
                    } else {
                        [self enforceConnectedState];
                    }
                }
            }
                break;
            default:
                break;
        }
        
    }
    
}


// IN THE PROCESS OF BEING DEPRECATED
//- (void)processEvent:(MWCocoaEvent *)event {
//	
//    return;
//    
//	int code = [event code];
//	
//	if(code == RESERVED_CODEC_CODE) {
//		
//		
//		[self unregisterCallbacksWithKey:STATE_SYSTEM_CALLBACK_KEY locking:NO];
//		//[self registerEventCallbackWithReceiver:self 
////										selector:@selector(processEvent:)
////											 callbackKey:STATE_SYSTEM_CALLBACK_KEY
////									forVariableCode:[NSNumber numberWithInt:RESERVED_CODEC_CODE]];
////		
//		
//        
//        // 12/09 DDC: deprecated
//        //[self registerEventCallbackWithReceiver:self 
////										selector:@selector(processEvent:)
////											 callbackKey:STATE_SYSTEM_CALLBACK_KEY
////									forVariableCode:systemCodecCode];
////		
//		
//    
//    [self registerBindingsBridgeWithReceiver:self
//										bindingsKey:@"experimentLoadProgress"
//												callbackKey:EXPERIMENT_LOAD_PROGRESS_KEY
//									forVariable:@"#experimentLoadProgress"]; // TODO: use EXPERIMENT_LOAD_PROGRESS_TAGNAME instead
//		
//	} else {
//		if(systemCodecCode < RESERVED_CODEC_CODE) {
//			systemCodecCode = RESERVED_SYSTEM_EVENT_CODE; // TODO: cleanup
//		}
//		
//		
//		if (code == systemCodecCode && systemCodecCode > RESERVED_CODEC_CODE) {
//			mw::Datum payload(*[event data]);
//			mw::Datum sysEventType(payload.getElement(M_SYSTEM_PAYLOAD_TYPE));
//			
//			if(!sysEventType.isUndefined()) {
//				// The F swich statement ... it's gotta be somewhere
//				switch((mw::SystemPayloadType)sysEventType.getInteger()) {
//					case M_PROTOCOL_PACKAGE:
//					{
//						mw::Datum pp(payload.getElement(M_SYSTEM_PAYLOAD));
//						
//						[protocolNames removeAllObjects];
//						
//						mw::Datum protocols(pp.getElement(M_PROTOCOLS));
//						
//						for(int i=0; i<protocols.getNElements(); ++i) {
//							mw::Datum protocol(protocols[i]);
//							
//							NSString *protocolName = [[NSString alloc] initWithCString:protocol.getElement(M_PROTOCOL_NAME).getString()
//																			  encoding:NSASCIIStringEncoding];
//							
//							[protocolNames addObject:protocolName];
//						}
//						
//						[self willChangeValueForKey:@"protocolNames"];
//						[self didChangeValueForKey:@"protocolNames"];
//												
//						string current_protocol_string = pp.getElement(M_CURRENT_PROTOCOL).getString();
//		
//						[self setCurrentProtocolName:[[NSString alloc] initWithCString:current_protocol_string.c_str()
//																	   encoding:NSASCIIStringEncoding]];
//																	   
//						if(currentProtocolName == Nil || ([protocolNames indexOfObject:currentProtocolName] == NSNotFound)){
//							[self setCurrentProtocolName:[protocolNames objectAtIndex:0]];
//						}
//					}
//						break;
//					
//					case M_DATA_FILE_OPENED:
//					{
//						mw::Datum event(payload.getElement(M_SYSTEM_PAYLOAD));
//						
//						// TODO: kludge MUST be fixed
//						mw::Datum file(event.getElement(1));
//                        mw::Datum success(event.getElement(0));
//						
//                        if((int)success != M_COMMAND_SUCCESS){
//                            [self setDataFileName:Nil];
//                            [self setDataFileOpen:NO];
//                            break;
//                        }
//                        
//						[self setDataFileName:[NSString stringWithCString:file.getString() encoding:NSASCIIStringEncoding]];
//						if([self dataFileName] != Nil){
//							[self setDataFileOpen: YES];
//						}
//					}
//						break;
//					
//					case M_DATA_FILE_CLOSED:
//					{
//						mw::Datum event(payload.getElement(M_SYSTEM_PAYLOAD));
//						
//						// TODO: kludge MUST be fixed
//						mw::Datum file(event.getElement(1));
//						
//						[self setDataFileName:Nil];
//						[self setDataFileOpen: NO];
//						
//					}
//						break;
//					
//					case M_EXPERIMENT_STATE:
//					{
//						mw::Datum state(payload.getElement(M_SYSTEM_PAYLOAD));
//						
//						[self setExperimentLoaded:state.getElement(M_LOADED).getBool()];
//						if([self experimentLoaded]) {
//							[self setExperimentLoading:NO];						
//                            core->unregisterCallbacks(CLIENT_LOAD_MESSAGE_CALLBACK_KEY, false);
//							[self setExperimentPaused:state.getElement(M_PAUSED).getBool()];
//							[self setExperimentRunning: state.getElement(M_RUNNING).getBool()];
//							[self setExperimentName:[[NSString alloc] initWithCString:state.getElement(M_EXPERIMENT_NAME).getString()
//																	  encoding:NSASCIIStringEncoding]];
//							
//							if([self experimentName] == Nil){
//								[self setExperimentName:@"Unnamed Experiment"];
//							}
//							
//							[self setExperimentPath:[[NSString alloc] initWithCString:state.getElement(M_EXPERIMENT_PATH).getString()
//																	  encoding:NSASCIIStringEncoding]];
//							
//							
//                            [self willChangeValueForKey:@"serversideVariableSetNames"];
//
//							[serversideVariableSetNames removeAllObjects];
//							mw::Datum svs(state.getElement(M_SAVED_VARIABLES));
//							
//							if(svs.isList()) {
//								for(int i=0; i<svs.getNElements(); ++i) {
//									mw::Datum set(svs.getElement(i));
//									
//									[serversideVariableSetNames addObject:[NSString stringWithCString:set.getString()
//																					encoding:NSASCIIStringEncoding]];
//								}
//							}
//                            
//                            [self didChangeValueForKey:@"serversideVariableSetNames"];
//							
//							if(accumulatingErrors){
//								[self stopAccumulatingErrors];
//							}
//						
//						} else {
//							// Not loaded
//							if(accumulatingErrors){
//								[self stopAccumulatingErrors];
//							}
//								
//							[self setExperimentLoading:NO];
//							[self setExperimentRunning:NO];
//							
//							if([self hasExperimentLoadErrors]){
//								[self enforceLoadFailedState];
//							} else {
//								[self enforceConnectedState];
//							}
//						}
//					}
//						break;
//					default:
//						break;
//				}
//				
//			} 
//		}
//	}
//
//}

- (void) loadPlugins {

    // create a carrier window to hold all of the plugins when they are grouped
    grouped_plugin_controller = [[MWGroupedPluginWindowController alloc] initWithClientInstance:self];
    [NSBundle loadNibNamed:@"GroupedPluginsWindow" owner:grouped_plugin_controller];
    [grouped_plugin_controller loadWindow];
    [[grouped_plugin_controller window] orderOut:self];
    [grouped_plugin_controller setCustomColor:self.headerColor];
    [grouped_plugin_controller setWindowFrameAutosaveName:self.serverURL];
	
  
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
        
                if(![nib instantiateNibWithOwner:self topLevelObjects:&toplevel]){
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
      
			NSWindowController *controller = Nil;
			
			for(int j=0; j < [toplevel count]; j++){
				NSObject *obj = [toplevel objectAtIndex:j];
				//NSLog(@"object = %d", obj);
				if([obj isKindOfClass:[NSWindowController class]]){
					controller = (NSWindowController *)obj;
					[controller loadWindow];
                    [controller setWindowFrameAutosaveName:plugin_file];
					[pluginWindows addObject:controller];
                    
                    // also, add the window to the grouped_plugin_controller
                    [grouped_plugin_controller addPluginWindow:[controller window] withName:[[controller window] title]];
                    [grouped_plugin_controller setCurrentPluginIndex:0];
                    
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


- (NSArray *)pluginWindows {
	return pluginWindows;
}

- (void)showPlugin:(int)i {
    if(i == [grouped_plugin_controller currentPluginIndex]){
        [grouped_plugin_controller hideWindow];
        //[grouped_plugin_controller incrementPlugin:self];
    }
	
    NSWindowController<MWWindowController> *controller = [pluginWindows objectAtIndex:i];
    if([controller respondsToSelector:@selector(setInGroupedWindow:)]){
        [controller setInGroupedWindow:NO];
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

- (NSWindow *)groupedPluginWindow {
    return [grouped_plugin_controller window];
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


- (void)saveOpenPluginWindows {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    [defaults setBool:[[grouped_plugin_controller window] isVisible] forKey:DEFAULTS_GROUPED_PLUGIN_WINDOW_IS_OPEN_KEY];
    
    NSArray *previousOpenPluginWindows = [defaults arrayForKey:DEFAULTS_OPEN_PLUGIN_WINDOWS_KEY];
    NSMutableArray *openPluginWindows;
    
    if (previousOpenPluginWindows) {
        openPluginWindows = [previousOpenPluginWindows mutableCopy];
    } else {
        openPluginWindows = [NSMutableArray array];
    }
    
    for (NSWindowController *controller in pluginWindows) {
        NSString *autoSaveName = [controller windowFrameAutosaveName];
        [openPluginWindows removeObject:autoSaveName];
        if ([[controller window] isVisible]) {
            [openPluginWindows addObject:autoSaveName];
        }
    }
    
    [defaults setObject:openPluginWindows forKey:DEFAULTS_OPEN_PLUGIN_WINDOWS_KEY];
}


- (void)restoreOpenPluginWindows {
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    
    if ([defaults boolForKey:DEFAULTS_GROUPED_PLUGIN_WINDOW_IS_OPEN_KEY]) {
        [self showGroupedPlugins];
    }
    
    NSArray *openPluginWindows = [defaults arrayForKey:DEFAULTS_OPEN_PLUGIN_WINDOWS_KEY];
    if (openPluginWindows) {
        for (int i = 0; i < [pluginWindows count]; i++) {
            NSWindowController *controller = [pluginWindows objectAtIndex:i];
            if ([openPluginWindows containsObject:[controller windowFrameAutosaveName]]) {
                [self showPlugin:i];
            }
        }
    }
}


- (void)startAccumulatingErrors{

	
	[self clearAccumulatedErrors];
	
	accumulatingErrors = YES;
	
	NSString *messageTag = [NSString stringWithCString:ANNOUNCE_MESSAGE_VAR_TAGNAME
									 encoding:NSASCIIStringEncoding];
	[self registerEventCallbackWithReceiver:self 
						selector:@selector(handleErrorMessageEvent:)
						callbackKey:ERROR_MESSAGE_CALLBACK_KEY
						forVariable:messageTag
                        onMainThread:YES];

}

- (void)stopAccumulatingErrors{
	[self unregisterCallbacksWithKey:ERROR_MESSAGE_CALLBACK_KEY locking:YES];
	
	[appController willChangeValueForKey:@"modalClientInstanceInCharge"];
	[appController didChangeValueForKey:@"modalClientInstanceInCharge"];
	
	[self willChangeValueForKey:@"errors"];
	[self didChangeValueForKey:@"errors"];
	
	[self setErrorString:[errors componentsJoinedByString:@"\n"]];
	
	accumulatingErrors = NO;	
}


- (void)handleLoadMessageEvent:(MWCocoaEvent *)event{
	mw::Datum payload(*[event data]);
	
	if(payload.getDataType() != M_DICTIONARY){
		return;
	}
	
	if(payload.getElement(M_MESSAGE).getDataType() != M_STRING){
		return;
	}
	
	string message_str(payload.getElement(M_MESSAGE).getString());
	if(message_str.empty()){
		return;
	}
	NSString *message = [[NSString alloc] initWithCString:message_str.c_str() 
												 encoding:NSASCIIStringEncoding];
	self.experimentLoadMessage = message;
}

- (void)handleErrorMessageEvent:(MWCocoaEvent *)event{


	//mw::Datum *pl = [event data];
	mw::Datum payload(*[event data]);
	
	if(payload.getDataType() != M_DICTIONARY){
		return;
	}
	
	if(payload.getElement(M_MESSAGE).getDataType() != M_STRING){
		return;
	}
	
	string message_str(payload.getElement(M_MESSAGE).getString());
	if(message_str.empty()){
		return;
	}
	NSString *message = [[NSString alloc] initWithCString:message_str.c_str() 
												encoding:NSASCIIStringEncoding];
	
	int msgType = payload.getElement(M_MESSAGE_TYPE).getInteger();
	
//	MWorksTime eventTime = [event time];
	
//	NSNumber *time = [[[NSNumber alloc] initWithLongLong:eventTime/1000] autorelease];
	
	if(msgType != M_ERROR_MESSAGE){
		return;
	}
	
	[errors addObject:message];
	
	[self setHasExperimentLoadErrors:YES];
	
}

- (void)clearAccumulatedErrors{
	[errors removeAllObjects];
	[self setHasExperimentLoadErrors:NO];
}



@end
