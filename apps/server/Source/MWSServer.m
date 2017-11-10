/**
 * MWSServer.m
 *
 * History:
 * bkennedy on 07/05/07 - Created.
 *
 * Copyright MIT 2007.  All rights reserved.
 */

#import "MWSServer.h"

#import <MWorksCocoa/MWCocoaEventFunctor.h>
#import <MWorksCocoa/MWConsoleController.h>
#import <MWorksCore/Server.h>

#define LISTENING_ADDRESS_KEY @"listeningAddressKey"
#define LISTENING_PORT_KEY @"listeningPortKey"

#define DEFAULTS_AUTO_OPEN_CLIENT @"autoOpenClient"
#define DEFAULTS_AUTO_OPEN_CONSOLE @"autoOpenConsole"

#define MWORKS_DOC_PATH @"/Library/Application Support/MWorks/Documentation/index.html"
#define MWORKS_HELP_URL @"https://mworks.tenderapp.com/"


@implementation MWSServer {
    NSString *listeningAddress;
    NSInteger listeningPort;
    
    boost::shared_ptr<Server> core;
    MWConsoleController *cc;
}


+ (void)initialize {
    //
    // The class identity test ensures that this method is called only once.  For more info, see
    // http://lists.apple.com/archives/cocoa-dev/2009/Mar/msg01166.html
    //
    if (self == [MWSServer class]) {
        NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
        
        defaultValues[LISTENING_ADDRESS_KEY] = @"127.0.0.1";
        defaultValues[LISTENING_PORT_KEY] = @19989;
        defaultValues[DEFAULTS_AUTO_OPEN_CLIENT] = @NO;
        defaultValues[DEFAULTS_AUTO_OPEN_CONSOLE] = @NO;
        
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
    }
}


- (instancetype)init {
    if ((self = [super init])) {
        NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
        listeningAddress = [defaults objectForKey:LISTENING_ADDRESS_KEY];
        listeningPort = [defaults integerForKey:LISTENING_PORT_KEY];
        
        core = boost::make_shared<Server>();
        Server::registerInstance(core);
        
        cc = [[MWConsoleController alloc] init];
        [cc setTitle:@"Server Console"];
        [cc setDelegate:self];
    }
    
    return self;
}


/****************************************************************
 *              NSApplicationDelegate methods
 ***************************************************************/


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    core->setHostname(listeningAddress.UTF8String);
    core->setListenPort(listeningPort);
    core->startServer();
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_AUTO_OPEN_CLIENT]) {
        [NSTask launchedTaskWithLaunchPath:@"/usr/bin/open"
                                 arguments:@[@"/Applications/MWClient.app"]];
    }
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_AUTO_OPEN_CONSOLE]) {
        [self toggleConsole:nil];
    }
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    core->stopServer();
}


/****************************************************************
*              IBAction methods
***************************************************************/


- (IBAction)launchDocs:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:MWORKS_DOC_PATH isDirectory:NO]];
}


- (IBAction) launchHelp: (id) sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:MWORKS_HELP_URL]];
}


- (IBAction)togglePreferences:(id)sender {
    if (self.preferencesWindow.isVisible) {
        [self.preferencesWindow orderOut:self];
    } else {
        [self.preferencesWindow makeKeyAndOrderFront:self];
    }
}


- (IBAction)toggleConsole:(id)sender {
	if (cc.window.visible) {
		[cc close];
	} else {
		[cc showWindow:nil];	
	}
}


////////////////////////////////////////////////////////////////////////////////
// MWClientServerBase methods
////////////////////////////////////////////////////////////////////////////////


- (NSNumber *)codeForTag:(NSString *)tag {
	return @(core->lookupCodeForTag([tag cStringUsingEncoding:NSASCIIStringEncoding]));
}


- (void)unregisterCallbacksWithKey:(const char *)key {
	core->unregisterCallbacks(key);
}


- (void)registerEventCallbackWithReceiver:(id)receiver
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                             onMainThread:(BOOL)on_main
{
    core->registerCallback(create_cocoa_event_callback(receiver, selector, on_main), key);
}


- (void)registerEventCallbackWithReceiver:(id)receiver
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                          forVariableCode:(int)code
                             onMainThread:(BOOL)on_main
{
    if (code >= 0) {
        core->registerCallback(code, create_cocoa_event_callback(receiver, selector, on_main), key);
    }
}


@end


























