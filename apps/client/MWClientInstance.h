//
//  MWClientInstance.h
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <MWorksCore/Client.h>
#import <MWorksCocoa/MWCocoaEvent.h>
#import <MWorksCocoa/MWCoreContainer.h>
#import <MWorksCocoa/MWClientProtocol.h>
#import "MWCodec.h"
#import "MWGroupedPluginWindowController.h"
//#import "MWNotebook.h"
#import "AppController.h"


//#define HOLLOW_OUT_FOR_ADC	0


#define STATE_SYSTEM_CALLBACK_KEY "MWorksClient::StateSystemCallbackKey"

@interface MWClientInstance : NSObject <MWCoreContainer, MWClientProtocol> {	

	// The core object that actually does all of the work
	#ifndef HOLLOW_OUT_FOR_ADC
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

// Accessors
- (shared_ptr<mw::Client>) coreClient;
- (shared_ptr<mw::EventStreamInterface>) eventStreamInterface;
@property(retain) MWCodec *variables;
- (NSArray *)variableNames;
@property(assign)	NSMutableArray *errors;

@property(copy, readwrite) NSString *errorString;

//@property(readonly) MWNotebook *notebook;
- (MWNotebook *)notebook;


// Connect / disconnect
@property BOOL serverConnected;
@property BOOL serverConnecting;
@property(copy, readwrite) NSString *serverName;
@property(nonatomic, copy, readwrite) NSString *serverURL;
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
@property(assign) NSMutableArray *serversideVariableSetNames;

// Data File state
@property BOOL dataFileOpen;
@property(copy, readwrite) NSString *dataFileName;
@property BOOL dataFileOverwrite;

// Protocol state
@property(assign)	NSMutableArray *protocolNames;
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
- (void)updateVariableWithCode:(int)code withData:(mw::Datum *)data;
- (void)updateVariableWithTag:(NSString *)tag withData:(mw::Datum *)data;

// Methods for registering to be notified of events from the core object
- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                                   onMainThread:(BOOL)on_main;

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
						  forVariableCode:(int)code
                             onMainThread:(BOOL)on_main;

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
							  forVariable:(NSString *)tag
                             onMainThread:(BOOL)on_main;

- (void)registerBindingsBridgeWithReceiver:(id)receiver 
							bindingsKey:(NSString *)bindings_key
									callbackKey:(const char *)key
							   forVariable:(NSString *)tag;


- (void)unregisterCallbacksWithKey:(const char *)key;
- (void)unregisterCallbacksWithKey:(const char *)key locking:(BOOL)locking;

// The client object's own method for dealing with events from the core object
// (delivered via callback)
//- (void)processEvent:(MWCocoaEvent *)event;


// Actions
- (void)connect;
- (void)disconnect;
- (void)shutDown;

- (void)loadExperiment;
- (void)openDataFile;
- (void)closeDataFile;
- (void)closeExperiment;
- (void)updateRecentExperiments;

- (void)toggleExperimentRunning:(id)running;
- (void)toggleExperimentPaused:(id)paused;
						  
- (void) loadVariableSet;
- (void) saveVariableSet;

// Plugin-related
- (void) loadPlugins;
- (void)showAllPlugins;
- (void)showPlugin:(int)i;
- (void)showGroupedPlugins;
- (NSWindow *)groupedPluginWindow;
- (void)hideAllPlugins;
- (NSArray *)pluginWindows;
- (void)saveOpenPluginWindows;
- (void)restoreOpenPluginWindows;

// Error Handling
//- (void)launchErrorSheet;
//- (void)closeErrorSheet;
- (void)startAccumulatingErrors;
- (void)stopAccumulatingErrors;
- (void)clearAccumulatedErrors;

//- (void)handleMessageEvent:(MWCocoaEvent *)event;

@end
