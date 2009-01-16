//
//  MWClientInstance.h
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <MonkeyWorksCore/Client.h>
#import <MonkeyWorksCocoa/MWCocoaEvent.h>
#import "MWCodec.h"
#import "MWGroupedPluginWindowController.h"
#import "MWNotebook.h"
#import "AppController.h"

#define HOLLOW_OUT_FOR_ADC	0


#define STATE_SYSTEM_CALLBACK_KEY @"MonkeyWorksClient::StateSystem callback key"

@interface MWClientInstance : NSObject {	

	// The core object that actually does all of the work
	#if !HOLLOW_OUT_FOR_ADC
		shared_ptr<mw::Client> core;
	#endif
	
	AppController * appController;
	
	int systemCodecCode;

	// A bindings-compatible object in charge of interactions with the codec /
	// variables registry
	MWCodec *variables;

	// Maintained state (kept for the sake of bindings, tightly
	// coupled to state queried from the core object)
	BOOL serverConnected;
	BOOL serverConnecting;
	NSString *serverURL;
	NSNumber *serverPort;
	NSString *serverName;
	BOOL launchIfNeeded;
	
	BOOL experimentLoading;
	NSNumber * experimentLoadProgress;
	NSString * experimentLoadMessage;
	
	BOOL experimentLoaded;
	BOOL experimentRunning;
	BOOL experimentPaused;
	
	NSString *experimentPath;
	NSString *experimentName;
	
    BOOL variableSetLoaded;
    NSString *variableSetName;
    NSMutableArray *serversideVariableSetNames;
    
	BOOL dataFileOpen;
	NSString *dataFileName;
	BOOL dataFileOverwrite;
	
	NSMutableArray *protocolNames;
	NSString *currentProtocolName;
	
	NSString *summaryString;
	
	NSMutableArray *pluginWindows;
	
	NSTimer *connection_check_timer;
	
	NSMutableArray *errors;
	NSString *errorString;
	
	BOOL hasExperimentLoadErrors;
    
    MWGroupedPluginWindowController *grouped_plugin_controller;
    MWNotebook *notebook;
    
    // Cosmetics
    IBOutlet NSBox *headerBox;
    NSColor *headerColor;
	
	BOOL accumulatingErrors;
}

- (id)initWithAppController:(AppController *)_appController;
- (void)finalize;

// Accessors
- (shared_ptr<mw::Client>) coreClient;
@property(retain) MWCodec *variables;
- (NSArray *)variableNames;
@property(assign)	NSArray *errors;

@property(copy, readwrite) NSString *errorString;

@property(readonly) MWNotebook *notebook;

// Connect / disconnect
@property BOOL serverConnected;
@property BOOL serverConnecting;
@property(copy, readwrite) NSString *serverName;
@property(copy, readwrite) NSString *serverURL;
@property(copy, readwrite) NSNumber *serverPort;
@property BOOL launchIfNeeded;

// Experiment state
@property BOOL experimentLoading;
@property(copy, readwrite) NSNumber * experimentLoadProgress;
@property(copy, readwrite) NSString * experimentLoadMessage;
@property BOOL experimentLoaded;
@property BOOL experimentRunning;
@property BOOL experimentPaused;
@property(copy, readwrite) NSString *experimentName;
@property(copy, readwrite) NSString *experimentPath;
@property BOOL hasExperimentLoadErrors;

// Variable set state
@property BOOL variableSetLoaded;
@property(assign, readwrite) NSString *variableSetName;
@property(assign) NSArray *serversideVariableSetNames;

// Data File state
@property BOOL dataFileOpen;
@property(copy, readwrite) NSString *dataFileName;
@property BOOL dataFileOverwrite;

// Protocol state
@property(assign)	NSArray *protocolNames;
@property(copy, readwrite) NSString *currentProtocolName;
@property(copy, readwrite) NSString *summaryString;

// Cosmetics
@property(assign) NSColor *headerColor;

// Maintaining Overall GUI state
- (void)enforceDisconnectedState;
- (void)enforceConnectedState;
- (void)enforceLoadedState;
- (void)enforceRunningState;
- (void)enforceLoadFailedState;


// Interacting with the internal core object
- (NSNumber *)codeForTag:(NSString *)tag;
- (void)updateVariableWithCode:(int)code withData:(mw::Data *)data;
- (void)updateVariableWithTag:(NSString *)tag withData:(mw::Data *)data;

// Methods for registering to be notified of events from the core object
- (void)registerEventCallbackWithReceiver:(id)receiver 
							  andSelector:(SEL)selector
								   andKey:(NSString *)key;
- (void)registerEventCallbackWithReceiver:(id)receiver 
							  andSelector:(SEL)selector
								   andKey:(NSString *)key
						  forVariableCode:(NSNumber *)_code;
- (void)registerEventCallbackWithReceiver:(id)receiver 
							  andSelector:(SEL)selector
								   andKey:(NSString *)key
							  forVariable:(NSString *)tag;

- (void)registerBindingsBridgeWithReceiver:(id)receiver 
							andBindingsKey:(NSString *)bindings_key
									andKey:(NSString *)key
							   forVariable:(NSString *)tag;

// The client object's own method for dealing with events from the core object
// (delivered via callback)
- (void)processEvent:(MWCocoaEvent *)event;


// Actions
- (void)connect;
- (void)disconnect;

- (void)loadExperiment;
- (void)openDataFile;
- (void)closeDataFile;
- (void)closeExperiment;
- (void)updateRecentExperiments;

- (void)toggleExperimentRunning:(id)running;
						  
- (void) loadVariableSet;
- (void) saveVariableSet;

// Plugin-related
- (void) loadPlugins;
- (void)showAllPlugins;
- (void)showPlugin:(int)i;
- (void)showGroupedPlugins;
- (void)hideAllPlugins;
- (NSArray *)pluginWindows;

// Error Handling
//- (void)launchErrorSheet;
//- (void)closeErrorSheet;
- (void)startAccumulatingErrors;
- (void)stopAccumulatingErrors;
- (void)clearAccumulatedErrors;

//- (void)handleMessageEvent:(MWCocoaEvent *)event;

@end
