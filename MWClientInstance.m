//
//  MWClientInstance.m
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MWClientInstance.h"
#import "MonkeyWorksCore/StandardVariables.h"
#import "MonkeyWorksCocoa/MWCocoaEvent.h"
#import "MonkeyWorksCocoa/MWCocoaEventFunctor.h"
#import <MonkeyWorksCocoa/MWWindowController.h>

#define ERROR_MESSAGE_CALLBACK_KEY	@"ClientErrorMessageCallbackKey"
#define	LOAD_MESSAGE_CALLBACK_KEY	@"LoadMessageCallbackKey"

@implementation MWClientInstance


- (id)initWithAppController:(AppController *)_controller {

	appController = _controller;

	accumulatingErrors = NO;
	
    // Cosmetics
    headerColor = [appController uniqueColor];
    [headerBox setFillColor:headerColor];


	#if HOLLOW_OUT_FOR_ADC
		// no core object
	#else
		core = shared_ptr <mw::Client>(new mw::Client());
		variables = [[MWCodec alloc] initWithClientInstance:self];
	#endif
	
	protocolNames = [[NSMutableArray alloc] init];
	errors = [[NSMutableArray alloc] init];
    serversideVariableSetNames = [[NSMutableArray alloc] init];
	
	serverURL = @"127.0.0.1";
	serverPort = [NSNumber numberWithInteger:19989];
	
    variableSetName = Nil;
	self.variableSetName = Nil;
    self.variableSetLoaded = NO;
    
	[self registerEventCallbackWithReceiver:self 
								andSelector:@selector(processEvent:)
									 andKey:STATE_SYSTEM_CALLBACK_KEY];
									 
	[self setDataFileOpen:false];
	[self setExperimentLoaded:false];
	[self setServerConnected:false];
	[self setLaunchIfNeeded:YES];
	
    // create a notebook
    notebook = [[MWNotebook alloc] init];
    
	// load plugins
	pluginWindows = [[NSMutableArray alloc] init];
	[self loadPlugins];
	
	#if !HOLLOW_OUT_FOR_ADC
		core->startEventListener();
	#endif
	
	
    
	// start updateChangedValues timer
	#define CONNECTION_CHECK_INTERVAL	0.25
	connection_check_timer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)CONNECTION_CHECK_INTERVAL 
						target:self selector:@selector(checkConnection) userInfo:Nil repeats:YES];
	
    
	return self;
}


- (void)finalize {

	if([self serverConnected] || [self serverConnecting]){
		[self disconnect];
	}

    [self hideAllPlugins];
	
	[super finalize];
	
}

/*- (void)connectToURL:(NSString *)URL AtPort:(NSString *)port{
	NSLog([NSString stringWithFormat:@"Connecting beyotch (%@, %@)", URL, port]);
}*/

@synthesize notebook;

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

- (NSArray *)variableNames {
	return [variables variableNames];
}

- (NSDictionary *)varGroups {
	unsigned int nVars = core->numberOfVariablesInRegistry();
	
	NSMutableDictionary *allGroups = [[NSMutableDictionary alloc] init];
	
	for(int i=M_MAX_RESERVED_EVENT_CODE; i<nVars; ++i) {
		shared_ptr <mw::Variable> var = core->getVariable(i);
		
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


- (void)checkConnection {
	BOOL actually_connected = core->isConnected();
	
	if(actually_connected != [self serverConnected]){
		[self setServerConnected:actually_connected];
		
		if(![self serverConnected]){
			[self disconnect];
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
	[self hideAllPlugins];
	
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
		NSLog(@"Attempting to remotely launch server");
		NSString *username = @"labuser@";  //TODO
		NSString *combined_url = [username stringByAppendingString:serverURL];
		NSString *command = @"/Applications/MWServer.app/Contents/MacOS/MWServer";
		NSArray *arguments = [NSArray arrayWithObjects:combined_url, 
								command,
								Nil]; 
	
		NSTask *task;
		if([[self serverURL] isEqualToString:@"127.0.0.1"] || [[self serverURL] isEqualToString:@"localhost"]){
			NSMutableArray *local_launch_arguments = [[NSMutableArray alloc] init];
			task = [NSTask launchedTaskWithLaunchPath:command arguments:local_launch_arguments];
		} else {
			[NSTask launchedTaskWithLaunchPath:@"/usr/bin/ssh"  arguments:arguments];
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
								andSelector:@selector(handleLoadMessageEvent:)
									 andKey:LOAD_MESSAGE_CALLBACK_KEY
							forVariable:@"#announceMessage"];
	
	// Bind the messages variable temporarily to the loadingMessages key
//	[self registerBindingsBridgeWithReceiver:self 
//							andBindingsKey:@"experimentLoadMessage"
//								andKey:LOAD_MESSAGE_CALLBACK_KEY
//								forVariable:@"#announceMessage"];
	
	[self startAccumulatingErrors];
	core->sendExperiment(path);
	
}

- (void) updateRecentExperiments {
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSArray *recentExperiments = [defaults arrayForKey:@"recentExperiments"];
	if([self experimentPath] != Nil && ![recentExperiments containsObject:[self experimentPath]]){
		NSMutableArray *recentExperimentsMutable = [[NSMutableArray alloc] init];
		[recentExperimentsMutable addObjectsFromArray:recentExperiments];
		[recentExperimentsMutable addObject:[self experimentPath]];
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
	
	core->sendCloseExperimentEvent(path);
	
	[self setExperimentLoading:NO];
	[self setExperimentPath:Nil];
	[self setExperimentName:Nil];
	[self hideAllPlugins];
}


- (void) saveVariableSet {

    NSString *variable_save_name = [self variableSetName];
    if(variable_save_name != Nil){
        core->sendSaveVariablesEvent([variable_save_name cStringUsingEncoding:NSASCIIStringEncoding], 1);
        [notebook addEntry:[NSString stringWithFormat:@"Loaded variable set %@", variable_save_name, Nil]];
    } else {
        // TODO
    }
}

- (void) loadVariableSet {
    NSString *variable_load_name = [self variableSetName];
    if(variable_load_name != Nil){
        core->sendLoadVariablesEvent([variable_load_name cStringUsingEncoding:NSASCIIStringEncoding]);
        // TODO: should get confirmation!
        self.variableSetLoaded = YES;
    } else {
        //TODO
    }
}

- (void) openDataFile {

	NSString *filename = [self dataFileName];
	BOOL overwrite = [self dataFileOverwrite];
	
	core->sendOpenDataFileEvent([filename cStringUsingEncoding:NSASCIIStringEncoding],
								[[NSNumber numberWithBool:overwrite] intValue]);

    [notebook addEntry:[NSString stringWithFormat:@"Streaming to data file %@", filename, Nil]];
}


- (void) closeDataFile {

	NSString *filename = [self dataFileName];
	
	core->sendCloseDataFileEvent([filename cStringUsingEncoding:NSASCIIStringEncoding]);
    
    [notebook addEntry:[NSString stringWithFormat:@"Closing data file %@", filename, Nil]];
}



- (void)toggleExperimentRunning:(id)running{
	
	BOOL isit = [running boolValue];

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
}


// Iteraction with core client

- (NSNumber *)codeForTag:(NSString *)tag {
	
	NSNumber *code = [[NSNumber alloc] initWithInt:core->getCode([tag cStringUsingEncoding:NSASCIIStringEncoding])];
	return code;
}

- (void)updateVariableWithCode:(int)code withData:(mw::Data *)data {
	core->updateValue(code, *data);
}

- (void)updateVariableWithTag:(NSString *)tag withData:(mw::Data *)data {
	[self updateVariableWithCode:[[self codeForTag:tag] intValue] withData:data];
}


- (void)unregisterCallbacksWithKey:(NSString *)key {
	core->unregisterCallbacks([key cStringUsingEncoding:NSASCIIStringEncoding]);
}


- (void)registerEventCallbackWithReceiver:(id)receiver 
							  andSelector:(SEL)selector
								   andKey:(NSString *)key{

	boost::shared_ptr <mw::CocoaEventFunctor> cef;
	cef = boost::shared_ptr <mw::CocoaEventFunctor>(new mw::CocoaEventFunctor(receiver,
							selector,
							[key cStringUsingEncoding:NSASCIIStringEncoding]));
	core->registerCallback(cef);
}

- (void)registerEventCallbackWithReceiver:(id)receiver 
							  andSelector:(SEL)selector
								   andKey:(NSString *)key
						  forVariableCode:(NSNumber *)_code {
						  
	int code = [_code intValue];
	if(code >= 0) {
		boost::shared_ptr <mw::CocoaEventFunctor> cef;
		cef = boost::shared_ptr <mw::CocoaEventFunctor>(new mw::CocoaEventFunctor(receiver,
																			selector,
																			[key cStringUsingEncoding:NSASCIIStringEncoding]));
		core->registerCallback(cef, code);
	}
}

- (void)registerEventCallbackWithReceiver:(id)receiver 
							  andSelector:(SEL)selector
								   andKey:(NSString *)key
						  forVariable:(NSString *)tag {
	
	
	boost::shared_ptr <mw::CocoaEventFunctor> cef;
	cef = boost::shared_ptr <mw::CocoaEventFunctor>(new mw::CocoaEventFunctor(receiver,
																		selector,
																		[key cStringUsingEncoding:NSASCIIStringEncoding]));
	string tag_str([tag cStringUsingEncoding:NSASCIIStringEncoding]);
	core->registerCallback(cef, tag_str);
	
}


- (void)registerBindingsBridgeWithReceiver:(id)receiver 
							  andBindingsKey:(NSString *)bindings_key
								   andKey:(NSString *)key
							  forVariable:(NSString *)tag {
	
	
	boost::shared_ptr <mw::CocoaBindingsBridgeFunctor> cef;
	cef = boost::shared_ptr <mw::CocoaBindingsBridgeFunctor>(new mw::CocoaBindingsBridgeFunctor(receiver,
																		bindings_key,
																		[key cStringUsingEncoding:NSASCIIStringEncoding]));
	string tag_str([tag cStringUsingEncoding:NSASCIIStringEncoding]);
	core->registerCallback(cef, tag_str);
	
}




- (void)processEvent:(MWCocoaEvent *)event {
	
	int code = [event code];
	
	if(code == RESERVED_CODEC_CODE) {
		
		systemCodecCode = [[self codeForTag:[NSString stringWithCString:SYSTEM_VAR_TAGNAME
																   encoding:NSASCIIStringEncoding]] intValue];
		
		[self unregisterCallbacksWithKey:STATE_SYSTEM_CALLBACK_KEY];
		[self registerEventCallbackWithReceiver:self 
										andSelector:@selector(processEvent:)
											 andKey:STATE_SYSTEM_CALLBACK_KEY
									forVariableCode:[NSNumber numberWithInt:RESERVED_CODEC_CODE]];
		
		[self registerEventCallbackWithReceiver:self 
										andSelector:@selector(processEvent:)
											 andKey:STATE_SYSTEM_CALLBACK_KEY
									forVariableCode:[NSNumber numberWithInt:systemCodecCode]];
		#define EXPERIMENT_LOAD_PROGRESS_KEY @"experiment_load_progress_callback_key"
		[self registerBindingsBridgeWithReceiver:self
										andBindingsKey:@"experimentLoadProgress"
												andKey:EXPERIMENT_LOAD_PROGRESS_KEY
									forVariable:@"#experimentLoadProgress"]; // TODO: use EXPERIMENT_LOAD_PROGRESS_TAGNAME instead
		
	} else {
		if(systemCodecCode < RESERVED_CODEC_CODE) {
			systemCodecCode = [[self codeForTag:[NSString stringWithCString:SYSTEM_VAR_TAGNAME
																	   encoding:NSASCIIStringEncoding]] intValue];
		}
		
		
		if (code == systemCodecCode && systemCodecCode > RESERVED_CODEC_CODE) {
			mw::Data payload(*[event data]);
			mw::Data sysEventType(payload.getElement(M_SYSTEM_PAYLOAD_TYPE));
			
			if(!sysEventType.isUndefined()) {
				// The F swich statement ... it's gotta be somewhere
				switch((mw::SystemPayloadType)sysEventType.getInteger()) {
					case M_PROTOCOL_PACKAGE:
					{
						mw::Data pp(payload.getElement(M_SYSTEM_PAYLOAD));
						
						[protocolNames removeAllObjects];
						
						mw::Data protocols(pp.getElement(M_PROTOCOLS));
						
						for(int i=0; i<protocols.getNElements(); ++i) {
							mw::Data protocol(protocols[i]);
							
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
						mw::Data event(payload.getElement(M_SYSTEM_PAYLOAD));
						
						// TODO: kludge MUST be fixed
						mw::Data file(event.getElement(1));
                        mw::Data success(event.getElement(0));
						
                        if((int)success != M_COMMAND_SUCCESS){
                            [self setDataFileName:Nil];
                            [self setDataFileOpen:NO];
                            break;
                        }
                        
						[self setDataFileName:[NSString stringWithCString:file.getString()]];
						if([self dataFileName] != Nil){
							[self setDataFileOpen: YES];
						}
					}
						break;
					
					case M_DATA_FILE_CLOSED:
					{
						mw::Data event(payload.getElement(M_SYSTEM_PAYLOAD));
						
						// TODO: kludge MUST be fixed
						mw::Data file(event.getElement(1));
						
						[self setDataFileName:Nil];
						[self setDataFileOpen: NO];
						
					}
						break;
					
					case M_EXPERIMENT_STATE:
					{
						mw::Data state(payload.getElement(M_SYSTEM_PAYLOAD));
						
						[self setExperimentLoaded:state.getElement(M_LOADED).getBool()];
						if([self experimentLoaded]) {
							[self setExperimentLoading:NO];
							core->unregisterCallbacks("LoadMessageCallbackKey"); // TODO: kludgey
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
							mw::Data svs(state.getElement(M_SAVED_VARIABLES));
							
							if(svs.isList()) {
								for(int i=0; i<svs.getNElements(); ++i) {
									mw::Data set(svs.getElement(i));
									
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
	}

}

- (void) loadPlugins {

    // create a carrier window to hold all of the plugins when they are grouped
    grouped_plugin_controller = [[MWGroupedPluginWindowController alloc] initWithClientInstance:self];
    [NSBundle loadNibNamed:@"GroupedPluginsWindow" owner:grouped_plugin_controller];
    [grouped_plugin_controller loadWindow];
    [[grouped_plugin_controller window] orderOut:self];
    [grouped_plugin_controller setCustomColor:self.headerColor];
	NSFileManager *fm = [NSFileManager defaultManager];
	
	NSError *error;
	NSString *plugin_directory = @"/Library/Application Support/NewClient/Plugins/";
	NSArray *plugins = [fm contentsOfDirectoryAtPath:plugin_directory error:&error];
	
	for(int i = 0; i < [plugins count]; i++){
		NSString *plugin_file = [plugins objectAtIndex:i];
		NSLog(plugin_file);
		
		NSString *fullpath = [plugin_directory stringByAppendingString:plugin_file];
		NSBundle *plugin_bundle = [[NSBundle alloc] initWithPath:fullpath];
		if([plugin_bundle load]){
			NSNib *nib = [[NSNib alloc] initWithNibNamed:@"Main" bundle:plugin_bundle];
			NSArray *toplevel;
			if(![nib instantiateNibWithOwner:self topLevelObjects:&toplevel]){
				NSLog(@"Couldn't instantiate Nib");
				continue;
			}
			
			NSWindowController *controller = Nil;
			
			for(int j=0; j < [toplevel count]; j++){
				NSObject *obj = [toplevel objectAtIndex:j];
				NSLog(@"object = %d", obj);
				if([obj isKindOfClass:[NSWindowController class]]){
					controller = (NSWindowController *)obj;
					[controller loadWindow];
					[pluginWindows addObject:controller];
                    
                    // also, add the window to the grouped_plugin_controller
                    [grouped_plugin_controller addPluginWindow:[controller window] withName:[[controller window] title]];
                    [grouped_plugin_controller setCurrentPluginIndex:0];
                    
					NSLog(@"object is a controller (%d)", [pluginWindows count]);
					break;
				}
			}
			
			
		} else {
			NSLog(@"Couldn't load bundle");
		}
	}
	
	NSLog(@"%d plugin windows in instance %d", [pluginWindows count], self);
}


- (NSArray *)pluginWindows {
	return pluginWindows;
}

- (void)showPlugin:(int)i {
		NSWindowController *controller = [pluginWindows objectAtIndex:i];
		[[controller window] orderFront:self];
}

- (void)showAllPlugins {
	
    [grouped_plugin_controller hideWindow];
    
	for(int i = 0; i < [pluginWindows count]; i++){
		NSWindowController *controller = [pluginWindows objectAtIndex:i];
		[[controller window] orderFront:self];
	}

	NSLog(@"showing all (%d) from instance %d", [pluginWindows count], self);
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


- (void)startAccumulatingErrors{

	
	[self clearAccumulatedErrors];
	
	accumulatingErrors = YES;
	
	NSString *messageTag = [NSString stringWithCString:ANNOUNCE_MESSAGE_VAR_TAGNAME
									 encoding:NSASCIIStringEncoding];
	[self registerEventCallbackWithReceiver:self 
						andSelector:@selector(handleErrorMessageEvent:)
						andKey:ERROR_MESSAGE_CALLBACK_KEY
						forVariable:messageTag];

}

- (void)stopAccumulatingErrors{
	[self unregisterCallbacksWithKey:ERROR_MESSAGE_CALLBACK_KEY];
	
	[appController willChangeValueForKey:@"modalClientInstanceInCharge"];
	[appController didChangeValueForKey:@"modalClientInstanceInCharge"];
	
	[self willChangeValueForKey:@"errors"];
	[self didChangeValueForKey:@"errors"];
	
	[self setErrorString:[errors componentsJoinedByString:@"\n"]];
	
	accumulatingErrors = NO;	
}


- (void)handleLoadMessageEvent:(MWCocoaEvent *)event{
	mw::Data payload(*[event data]);
	
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


	//mw::Data *pl = [event data];
	mw::Data payload(*[event data]);
	
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
	
//	MonkeyWorksTime eventTime = [event time];
	
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
