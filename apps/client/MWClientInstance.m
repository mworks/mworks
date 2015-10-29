//
//  MWClientInstance.m
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWClientInstance.h"
#import "OrderedDictionary.h"
#import <MWorksCore/StandardVariables.h>
#import <MWorksCore/Client.h>
#import <MWorksCocoa/MWCocoaEvent.h>
#import <MWorksCocoa/MWCocoaEventFunctor.h>
#import <MWorksCocoa/MWWindowController.h>
#import <MWorksCocoa/MWNotebook.h>
#import <MWorksCocoa/NSString+MWorksCocoaAdditions.h>

#define ERROR_MESSAGE_CALLBACK_KEY	"MWClientInstance::client_error_message_callback"
#define CLIENT_SYSTEM_EVENT_CALLBACK_KEY  "MWClientInstance::system_event_callback"
#define EXPERIMENT_LOAD_PROGRESS_KEY "MWClientInstance::experiment_load_progress_callback"
#define CLIENT_LOAD_MESSAGE_CALLBACK_KEY    "MWClientInstance::load_message_callback"
#define CLIENT_CODEC_EVENT_CALLBACK_KEY "MWClientInstance::codec_callback"

#define DEFAULTS_OPEN_PLUGIN_WINDOWS_KEY @"openPluginWindows"
#define DEFAULTS_GROUPED_PLUGIN_WINDOW_IS_OPEN_KEY @"groupedPluginWindowIsOpen"

#define MAX_NUM_RECENT_EXPERIMENTS 20



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
	
    NSArray *recentServers = [[NSUserDefaults standardUserDefaults] arrayForKey:@"recentServers"];
    if (recentServers && ([recentServers count] > 0)) {
        serverURL = [[recentServers objectAtIndex:0] copy];
    } else {
        serverURL = @"127.0.0.1";
    }
    
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
    return boost::static_pointer_cast<EventStreamInterface>(core);
}

- (NSArray *)variableNames {
    // Return a copy of MWCodec's internal variable names array 
    return [[[variables variableNames] copy] autorelease];
}

- (NSDictionary *)varGroups {
  
	unsigned int nVars = core->numberOfVariablesInRegistry();
	
    // Use OrderedDictionary so that iterating over all groups preserves their ordering (which
    // is by order of first appearance in the experiment file)
	OrderedDictionary *allGroups = [[OrderedDictionary alloc] init];

  	for (int i=N_RESERVED_CODEC_CODES; i < nVars + N_RESERVED_CODEC_CODES; ++i) {
		shared_ptr <mw::Variable> var = core->getVariable(i);
		
        if ((var == NULL) || (var->getProperties() == NULL)) {
            continue;
        }
        
		std::vector <std::string> groups(var->getProperties()->getGroups());
		std::vector<std::string>::iterator iter = groups.begin();
		while (iter != groups.end()) {
			NSString *groupName = [NSString stringWithUTF8String:iter->c_str()];
			
			NSMutableArray *aGroup = [allGroups valueForKey:groupName];
			if(aGroup == nil) {
				aGroup = [[NSMutableArray alloc] init];
                [allGroups setValue:aGroup forKey:groupName];
                [aGroup release];  // allGroups retains aGroup, so we can safely release it
			}
			
			[aGroup addObject:[NSString stringWithUTF8String:var->getProperties()->getTagName().c_str()]];
			
			++iter;
		}
	}
	
	return [allGroups autorelease];
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
	[self setExperimentPaused:NO];
	
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
            
            [recentServersMutable release];
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
    self.clientsideExperimentPath = self.experimentPath;
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
        [recentExperimentsMutable release];
        
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
        
        if (!self.dataFileOpen && appController.shouldAutoOpenDataFile) {
            NSString *userName = NSUserName();
            NSString *experimentBaseName = self.clientsideExperimentPath.lastPathComponent.stringByDeletingPathExtension;
            
            NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
            dateFormatter.dateFormat = @"yyyyMMdd-HHmmss";
            NSString *dateTag = [dateFormatter stringFromDate:[NSDate date]];
            [dateFormatter release];
            
            self.dataFileName = [NSString stringWithFormat:@"%@-%@-%@", userName, experimentBaseName, dateTag];
            self.dataFileOverwrite = NO;
            [self openDataFile];
        }
        
		if([self currentProtocolName] != Nil){
			core->sendProtocolSelectedEvent([[self currentProtocolName] cStringUsingEncoding:NSASCIIStringEncoding]);
		}
		core->sendRunEvent();
        [notebook addEntry:@"Experiment started"];
	}
#endif
}


- (void)toggleExperimentPaused:(id)paused{
	
	BOOL isit = [paused boolValue];
    
#ifndef HOLLOW_OUT_FOR_ADC
	if(isit){
		//resume
        core->sendResumeEvent();
        [notebook addEntry:@"Experiment resumed"];
	} else {
		//pause
        core->sendPauseEvent();
        [notebook addEntry:@"Experiment paused"];
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

	
    core->registerCallback(create_cocoa_event_callback(receiver, selector, on_main, self), string(key));
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
                    
                    if([self hasExperimentLoadErrors]){
                        [self enforceLoadFailedState];
                    } else if (!self.experimentLoading) {
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
      
			NSWindowController *controller = Nil;
			
			for(int j=0; j < [toplevel count]; j++){
				NSObject *obj = [toplevel objectAtIndex:j];
				//NSLog(@"object = %d", obj);
				if([obj isKindOfClass:[NSWindowController class]]){
					controller = (NSWindowController *)obj;
					[controller loadWindow];
                    [controller setWindowFrameAutosaveName:[plugin_file stringByDeletingPathExtension]];
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


- (NSDictionary *)workspaceInfo {
    NSMutableDictionary *workspaceInfo = [NSMutableDictionary dictionary];
    
    [workspaceInfo setObject:self.serverURL forKey:@"serverURL"];
    [workspaceInfo setObject:self.serverPort forKey:@"serverPort"];
    [workspaceInfo setObject:self.clientsideExperimentPath forKey:@"experimentPath"];
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
        [alert release];
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



























