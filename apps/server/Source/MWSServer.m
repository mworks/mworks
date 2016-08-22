/**
 * MWSServer.m
 *
 * History:
 * bkennedy on 07/05/07 - Created.
 *
 * Copyright MIT 2007.  All rights reserved.
 */

#import "MWSServer.h"
#import "MWorksCocoa/MWCocoaEventFunctor.h"

#define LISTENING_ADDRESS_KEY @"listeningAddressKey"
#define LISTENING_PORT_KEY @"listeningPortKey"

#define DEFAULTS_AUTO_OPEN_CLIENT @"autoOpenClient"
#define DEFAULTS_AUTO_OPEN_CONSOLE @"autoOpenConsole"

#define MWORKS_DOC_PATH @"/Library/Application Support/MWorks/Documentation/index.html"
#define MWORKS_HELP_URL @"http://help.mworks-project.org/"


@implementation MWSServer


+ (void)initialize {
    //
    // The class identity test ensures that this method is called only once.  For more info, see
    // http://lists.apple.com/archives/cocoa-dev/2009/Mar/msg01166.html
    //
    if (self == [MWSServer class]) {
        NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
        
        [defaultValues setObject:@"127.0.0.1" forKey:LISTENING_ADDRESS_KEY];
        [defaultValues setObject:@19989 forKey:LISTENING_PORT_KEY];
        [defaultValues setObject:[NSNumber numberWithBool:NO] forKey:DEFAULTS_AUTO_OPEN_CLIENT];
        [defaultValues setObject:[NSNumber numberWithBool:NO] forKey:DEFAULTS_AUTO_OPEN_CONSOLE];
        
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
    }
}


- (id) init {
	self = [super init];
	if (self != nil) {
		core = boost::shared_ptr <Server>(new Server());
        Server::registerInstance(core);
        
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		listeningAddress = [defaults objectForKey:LISTENING_ADDRESS_KEY];
        listeningPort = [defaults integerForKey:LISTENING_PORT_KEY];
		
		cc = [[MWConsoleController alloc] init];
	}
	return self;
}





/****************************************************************
 *              NSApplication Delegate Methods
 ***************************************************************/
- (void)setError:(NSError *)error{
    err = error;
}
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {

	if(err != Nil){
        NSApplication *app = [aNotification object];
        [app presentError:err];
        [app terminate:self];
    }
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_AUTO_OPEN_CLIENT]) {
        [NSTask launchedTaskWithLaunchPath:@"/usr/bin/open"
                                 arguments:[NSArray arrayWithObject:@"/Applications/MWClient.app"]];
    }
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_AUTO_OPEN_CONSOLE]) {
        [self toggleConsole:nil];
    }
}

- (void)awakeFromNib{
	core->setListenPort(listeningPort);
	core->setHostname([listeningAddress UTF8String]);

	core->startServer();
	
    [cc setTitle:@"Server Console"];
	[cc setDelegate:self];
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
    if (preferencesWindow.isVisible) {
        [preferencesWindow orderOut:self];
    } else {
        [preferencesWindow makeKeyAndOrderFront:self];
    }
}

- (IBAction)toggleConsole:(id)sender {
	if([[cc window] isVisible]) {
		[cc close];
	} else {
		[cc showWindow:nil];	
	}
}

- (IBAction)closeExperiment:(id)sender {
	core->closeExperiment();
}

- (IBAction)openExperiment:(id)sender {
    NSOpenPanel * op = [NSOpenPanel openPanel];
    [op setCanChooseDirectories:NO];
    // it is important that you never allow multiple files to be selected!
    [op setAllowsMultipleSelection:NO];
    [op setAllowedFileTypes:[NSArray arrayWithObjects:@"xml", nil]];

    int bp = [op runModal];
    if(bp == NSFileHandlingPanelOKButton) {
        NSArray * fn = [op URLs];
        NSEnumerator * fileEnum = [fn objectEnumerator];
        NSURL * filename;
        while(filename = [fileEnum nextObject]) {
			if(!core->openExperiment([[filename path] cStringUsingEncoding:NSASCIIStringEncoding])) {
                NSLog(@"Could not open experiment %@", filename);
            }
        }
    }
}

- (IBAction)saveVariables:(id)sender {
    NSSavePanel * save = [NSSavePanel savePanel];
    [save setAllowedFileTypes:[NSArray arrayWithObject:@"xml"]];
    [save setCanCreateDirectories:NO];
    if([save runModal] == NSFileHandlingPanelOKButton) {
		core->saveVariables(boost::filesystem::path([[[save URL] path] cStringUsingEncoding:NSASCIIStringEncoding]));
    }
	
}

- (IBAction)loadVariables:(id)sender {
	NSOpenPanel * op = [NSOpenPanel openPanel];
    [op setCanChooseDirectories:NO];
    // it is important that you never allow multiple files to be selected!
    [op setAllowsMultipleSelection:NO];
    [op setAllowedFileTypes:[NSArray arrayWithObjects:@"xml", nil]];
	
    int bp = [op runModal];
    if(bp == NSFileHandlingPanelOKButton) {
        NSArray * fn = [op URLs];
        NSEnumerator * fileEnum = [fn objectEnumerator];
        NSURL * filename;
        while(filename = [fileEnum nextObject]) {			
			core->loadVariables(boost::filesystem::path([[filename path] cStringUsingEncoding:NSASCIIStringEncoding]));
        }
    }
	
}

- (IBAction)openDataFile:(id)sender {
    NSSavePanel * save = [NSSavePanel savePanel];
    [save setAllowedFileTypes:[NSArray arrayWithObject:@"mwk"]];
    [save setCanCreateDirectories:NO];
    if([save runModal] == NSFileHandlingPanelOKButton) {
        core->openDataFile([[[[save URL] path] lastPathComponent]
                            cStringUsingEncoding:NSASCIIStringEncoding]);
    }
	
}

- (IBAction)closeDataFile:(id)sender {
	core->closeFile();
}


- (IBAction)startExperiment:(id)delegate {
	if(!core->isExperimentRunning()) {
		core->startExperiment();
	}
}

- (IBAction)stopExperiment:(id)delegate {
	if(core->isExperimentRunning()) {
		core->stopExperiment();
	}
}



////////////////////////////////////////////////////////////////////////////////
// Delegate Methods
////////////////////////////////////////////////////////////////////////////////
- (NSNumber *)codeForTag:(NSString *)tag {
	return [NSNumber numberWithInt:core->lookupCodeForTag([tag cStringUsingEncoding:NSASCIIStringEncoding])];
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


- (NSArray *)variableNames {
	std::vector<std::string> varTagNames(core->getVariableTagNames());
	NSMutableArray *varNames = [[NSMutableArray alloc] init];
	
	for(std::vector<std::string>::iterator iter = varTagNames.begin();
		iter != varTagNames.end(); 
		++iter) {
		[varNames addObject:[NSString stringWithCString:iter->c_str() 
											   encoding:NSASCIIStringEncoding]];
	}
	
	return varNames;	
}


@end
