/**
 * MWSServer.m
 *
 * History:
 * bkennedy on 07/05/07 - Created.
 *
 * Copyright MIT 2007.  All rights reserved.
 */

#import "MWSServer.h"

@import MWorksSwift;

#define LISTENING_ADDRESS_KEY @"listeningAddressKey"
#define LISTENING_PORT_KEY @"listeningPortKey"

#define DEFAULTS_AUTO_OPEN_CLIENT @"autoOpenClient"
#define DEFAULTS_AUTO_OPEN_CONSOLE @"autoOpenConsole"
#define DEFAULTS_CURRENT_PREFERENCES_TAB_INDEX @"currentPreferencesTabIndex"

#define MWORKS_DOC_PATH @"/Library/Application Support/MWorks/Documentation/index.html"
#define MWORKS_HELP_URL @"https://mworks.discourse.group/"


@implementation MWSServer {
    NSString *listeningAddress;
    NSInteger listeningPort;
    
    MWKServer *server;
    MWConsoleController *cc;
    
    id<NSObject> ioActivity;
}


+ (void)initialize {
    //
    // The class identity test ensures that this method is called only once.  For more info, see
    // http://lists.apple.com/archives/cocoa-dev/2009/Mar/msg01166.html
    //
    if (self == [MWSServer class]) {
        NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
        
        defaultValues[LISTENING_ADDRESS_KEY] = @"localhost";
        defaultValues[LISTENING_PORT_KEY] = @19989;
        defaultValues[DEFAULTS_AUTO_OPEN_CLIENT] = @NO;
        defaultValues[DEFAULTS_AUTO_OPEN_CONSOLE] = @NO;
        defaultValues[DEFAULTS_CURRENT_PREFERENCES_TAB_INDEX] = @0;
        
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
    }
}


- (instancetype)init {
    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
    NSString *listeningAddress = [defaults objectForKey:LISTENING_ADDRESS_KEY];
    NSInteger listeningPort = [defaults integerForKey:LISTENING_PORT_KEY];
    
    // TODO: handle server creation failure!
    MWKServer *server = [MWKServer serverWithListeningAddress:listeningAddress listeningPort:listeningPort error:NULL];
    
    if ((self = [super initWithCore:server])) {
        self->listeningAddress = listeningAddress;
        self->listeningPort = listeningPort;
        
        self->server = server;
        
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
    //
    // Without an active power assertion, calls to poll() inside ZeroMQ (specifically,
    // in zmq::signaler_t::wait()) will eventually take longer than requested to time out,
    // which makes the server unresponsive to client requests.
    //
    // I suspect this is a consequence of App Nap, but I'm not sure:
    // https://developer.apple.com/library/archive/documentation/Performance/Conceptual/power_efficiency_guidelines_osx/AppNap.html
    //
    ioActivity = [NSProcessInfo.processInfo beginActivityWithOptions:(NSActivityBackground | NSActivityIdleSystemSleepDisabled)
                                                              reason:@"Prevent I/O throttling"];
    
    [server start];
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_AUTO_OPEN_CLIENT]) {
        [NSTask launchedTaskWithLaunchPath:@"/usr/bin/open"
                                 arguments:@[@"/Applications/MWClient.app"]];
    }
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_AUTO_OPEN_CONSOLE]) {
        [self toggleConsole:nil];
    }
    
    [self.preferencesWindowTabView selectTabViewItemAtIndex:[[NSUserDefaults standardUserDefaults]
                                                             integerForKey:DEFAULTS_CURRENT_PREFERENCES_TAB_INDEX]];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    [server stop];
    [NSProcessInfo.processInfo endActivity:ioActivity];
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


- (IBAction)showAdvancedDisplayPreferences:(id)sender {
    [self.preferencesWindow beginSheet:self.advancedDisplayPreferencesWindow completionHandler:NULL];
}


- (IBAction)hideAdvancedDisplayPreferences:(id)sender {
    [self.preferencesWindow endSheet:self.advancedDisplayPreferencesWindow];
}


- (IBAction)toggleConsole:(id)sender {
	if (cc.window.visible) {
		[cc close];
	} else {
		[cc showWindow:nil];	
	}
}


////////////////////////////////////////////////////////////////////////////////
// NSTabViewDelegate methods
////////////////////////////////////////////////////////////////////////////////


- (void)tabView:(NSTabView *)tabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem {
    NSInteger index = [tabView indexOfTabViewItem:tabViewItem];
    [[NSUserDefaults standardUserDefaults] setInteger:index forKey:DEFAULTS_CURRENT_PREFERENCES_TAB_INDEX];
}


@end
