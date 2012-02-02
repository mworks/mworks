/**
 * MWSServer.m
 *
 * History:
 * bkennedy on 07/05/07 - Created.
 *
 * Copyright MIT 2007.  All rights reserved.
 */

#import "MWSServer.h"
#import "MWorksCore/PlatformDependentServices.h"
#import "MWorksCocoa/MWCocoaEventFunctor.h"
#import <sys/types.h>
#import <sys/socket.h>
#import <ifaddrs.h>

#define DEFAULT_HOST_IP @"127.0.0.1"
#define LISTENING_ADDRESS_KEY @"listeningAddressKey"

#define HELP_URL @"http://mworks-project.github.com"

@interface MWSServer(PrivateMethods)
- (void)processEvent:(id)cocoaEvent;
@end


@implementation MWSServer

- (id) init {
	self = [super init];
	if (self != nil) {
		core = boost::shared_ptr <Server>(new Server());
        Server::registerInstance(core);
		core_as_esi = static_pointer_cast<EventStreamInterface>(core);
        
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		listeningAddress = [defaults objectForKey:LISTENING_ADDRESS_KEY];

//		// TODO: this is crazy slow on some machines/networks
//		#define ESCHEW_NSHOST	1
//		#if ESCHEW_NSHOST
//			// TODO: double check this
//			struct ifaddrs *addrs;
//			int i = getifaddrs(&addrs);
//			NSMutableArray *netAddresses = [[NSMutableArray alloc] init];
//			while(addrs != NULL){
//				[netAddresses insertObject:[NSString stringWithCString:addrs->ifa_name] atIndex:0];
//				addrs = addrs->ifa_next;
//			}
//		#else	
//			NSArray *netAddresses = [[NSHost currentHost] addresses];
//		#endif
//		
//		NSString *regex  = @"[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}";
//		NSString *predicateFormat = [[@"SELF MATCHES \"" stringByAppendingString:regex] stringByAppendingString:@"\""];
//		
//		NSPredicate *addressPredicate =
//			[NSPredicate predicateWithFormat:predicateFormat];
//		
//		NSArray *filteredArray = [netAddresses filteredArrayUsingPredicate:addressPredicate];
//				
//		if(listeningAddress == nil || ![filteredArray containsObject:listeningAddress]) { 
//			listeningAddress = [[NSString alloc] initWithString:DEFAULT_HOST_IP];
//		}
		
		cc = [[MWConsoleController alloc] init];
	}
	return self;
}



- (void)dealloc {
	[listeningAddress release];
	[cc release];
	[super dealloc];
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
}
	
- (void)awakeFromNib{
	core->setListenLowPort(19989);
    core->setListenHighPort(19999);

	string hostname;
	if(listeningAddress == Nil || [listeningAddress isEqualToString:@""]){
		hostname = "127.0.0.1";
	} else {
		hostname = [listeningAddress cStringUsingEncoding:NSASCIIStringEncoding];
	}
	
	core->setHostname(hostname);
	

	core->startServer();
    core->startAccepting();	
	
    [cc setTitle:@"Server Console"];
	[cc setDelegate:self];
    
    [self updateGUI:nil];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // stop any data files still open.

    // close all open network connections.
    
    // close any applications it owns.
	//delete core;
}

/****************************************************************
*              IBAction methods
***************************************************************/
- (IBAction) launchHelp: (id) sender {
  NSLog(@"Launching Help...");
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:HELP_URL]];
}

- (IBAction) revealSetupVariables: (id)sender {
    NSLog(@"Launching setup variables...");
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:
                                            [NSString stringWithUTF8String:(setupVariablesFile().string().c_str())]]];
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

    int bp = [op runModalForTypes:[NSArray arrayWithObjects:@"xml", nil]];
    if(bp == NSOKButton) {
        NSArray * fn = [op filenames];
        NSEnumerator * fileEnum = [fn objectEnumerator];
        NSString * filename;
        while(filename = [fileEnum nextObject]) {
			if(!core->openExperiment([filename cStringUsingEncoding:NSASCIIStringEncoding])) {
                NSLog(@"Could not open experiment %@", filename);
            }
        }
    }
}

- (IBAction)saveVariables:(id)sender {
    NSSavePanel * save = [[NSSavePanel savePanel] retain];
    [save setAllowedFileTypes:[NSArray arrayWithObject:@"xml"]];
    [save setCanCreateDirectories:NO];
    if([save runModalForDirectory:nil file:nil] ==
	   NSFileHandlingPanelOKButton)  {
		core->saveVariables(boost::filesystem::path([[save filename] cStringUsingEncoding:NSASCIIStringEncoding]));
    }
	
	[save release];	
}

- (IBAction)loadVariables:(id)sender {
	NSOpenPanel * op = [[NSOpenPanel openPanel] retain];
    [op setCanChooseDirectories:NO];
    // it is important that you never allow multiple files to be selected!
    [op setAllowsMultipleSelection:NO];
	
    int bp = [op runModalForTypes:[NSArray arrayWithObjects:@"xml", nil]];
    if(bp == NSOKButton) {
        NSArray * fn = [op filenames];
        NSEnumerator * fileEnum = [fn objectEnumerator];
        NSString * filename;
        while(filename = [fileEnum nextObject]) {			
			core->loadVariables(boost::filesystem::path([filename cStringUsingEncoding:NSASCIIStringEncoding]));
        }
    }
	
	[op release];
}

- (IBAction)openDataFile:(id)sender {
    NSSavePanel * save = [[NSSavePanel savePanel] retain];
    [save setAllowedFileTypes:[NSArray arrayWithObject:@"mwk"]];
    [save setCanCreateDirectories:NO];
    if([save runModalForDirectory:nil file:nil] ==
	   NSFileHandlingPanelOKButton)  {
        core->openDataFile([[[save filename] lastPathComponent]
                            cStringUsingEncoding:NSASCIIStringEncoding]);
    }
	
	[save release];
}

- (IBAction)closeDataFile:(id)sender {
	core->closeFile();
}


- (IBAction)startExperiment:(id)delegate {
	if(!core->isExperimentRunning()) {
		core->startExperiment();
	}
	[self updateGUI:nil];
}

- (IBAction)stopExperiment:(id)delegate {
	if(core->isExperimentRunning()) {
		core->stopExperiment();
	}
	[self updateGUI:nil];
}



////////////////////////////////////////////////////////////////////////////////
// Delegate Methods
////////////////////////////////////////////////////////////////////////////////
- (NSNumber *)codeForTag:(NSString *)tag {
	return [NSNumber numberWithInt:core->lookupCodeForTag([tag cStringUsingEncoding:NSASCIIStringEncoding])];
}

- (void)startServer {
	core->startServer();
	[self setListeningAddress:listeningAddress];
	[self updateGUI:nil];
}

- (void)stopServer {
	core->stopServer();
	[self updateGUI:nil];
}

- (void)startAccepting {
	core->startAccepting();
	[self updateGUI:nil];
}

- (void)stopAccepting {
	core->stopAccepting();	
	[self updateGUI:nil];
}

- (NSNumber *)experimentLoaded {
	return [NSNumber numberWithBool:core->isExperimentLoaded()];
}

- (NSNumber *)experimentRunning {
	return [NSNumber numberWithBool:core->isExperimentRunning()];
}

- (NSNumber *)serverAccepting {
	return [NSNumber numberWithBool:core->isAccepting()];
}

- (NSNumber *)serverStarted {
	return [NSNumber numberWithBool:core->isStarted()];	
}

- (void)updateGUI:(id)arg {
	[mc updateDisplay];
	[tc updateDisplay];
}

- (void)unregisterCallbacksWithKey:(const char *)key {
	core->unregisterCallbacks(key);
}


- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                          forVariableCode:(int)code {
	
	if(code >= 0) {
		
		core->registerCallback(code, create_cocoa_event_callback(receiver, selector), key);
	}
	
}

- (void)openNetworkPreferences:(id)sender {
	[nc openAndInitWindow:sender];
}

- (NSString *)currentNetworkAddress:(id)sender {
    if(listeningAddress == Nil){
        return DEFAULT_HOST_IP;
    }
    
	return listeningAddress;
}

- (NSString *)defaultNetworkAddress:(id)sender {
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	NSString *defaultAddress = [defaults objectForKey:LISTENING_ADDRESS_KEY];
	
	if(defaultAddress == nil) { 
		defaultAddress = DEFAULT_HOST_IP;
        [self setListeningAddress:defaultAddress];
	}
	
	return defaultAddress;
}

- (void)setListeningAddress:(NSString *)address {
	NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
	[defaults setObject:address forKey:LISTENING_ADDRESS_KEY];
	[defaults synchronize];
	
//	listeningAddress = address;
	
//	core->stopAccepting();
//	core->stopServer();
//	core->setHostname([listeningAddress cStringUsingEncoding:NSASCIIStringEncoding]);
//	core->startServer();
//	core->startAccepting();	
	[self updateGUI:nil];	
}

- (NSArray *)variableNames {
	std::vector<std::string> varTagNames(core->getVariableTagNames());
	NSMutableArray *varNames = [[[NSMutableArray alloc] init] autorelease];
	
	for(std::vector<std::string>::iterator iter = varTagNames.begin();
		iter != varTagNames.end(); 
		++iter) {
		[varNames addObject:[NSString stringWithCString:iter->c_str() 
											   encoding:NSASCIIStringEncoding]];
	}
	
	return varNames;	
}



////////////////////////////////////////////////////////////////////////////////
// Private Methods
////////////////////////////////////////////////////////////////////////////////
- (void)processEvent:(id)event {
}


@end
