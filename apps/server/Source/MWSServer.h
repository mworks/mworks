/*!
* @header MWSServer
 *
 * @discussion Provides access to a shared instance of the MWSSever object
 * which contains controller objects for important parts of the system.
 *
 * bkennedy on 07/05/07 - Created.
 */

#import <Cocoa/Cocoa.h>
#import "MWSToolbarControl.h"
#import "MWSMenuControl.h"
#import "MWSNetworkPreferencesController.h"
#import "MWorksCocoa/MWConsoleController.h"
#import "MWorksCocoa/MWClientServerBase.h"
#import "MWorksCore/Server.h"

@interface MWSServer : MWClientServerBase {

	boost::shared_ptr<Server> core;
    boost::shared_ptr<EventStreamInterface> core_as_esi; // technical reasons for needing this handle
	MWConsoleController *cc;
	
	IBOutlet MWSToolbarControl *tc;
	IBOutlet MWSMenuControl *mc;
	IBOutlet MWSNetworkPreferencesController *nc;
	
	NSString *listeningAddress;
    
    NSError *err;
}

- (void) setError:(NSError *)error;

- (IBAction)openExperiment:(id)sender;
- (IBAction)closeExperiment:(id)sender;
- (IBAction)saveVariables:(id)sender;
- (IBAction)loadVariables:(id)sender;

- (IBAction)openDataFile:(id)sender;
- (IBAction)closeDataFile:(id)sender;
- (IBAction)stopExperiment:(id)sender;
- (IBAction)startExperiment:(id)sender;
- (IBAction) launchHelp: (id) sender;

@end

// delegate methods
@interface NSObject (MWSDelegateMethods) 
- (void)startServer;
- (void)stopServer;
- (void)startAccepting;
- (void)stopAccepting;
- (NSNumber *)serverAccepting;
- (NSNumber *)serverStarted;
- (NSNumber *)experimentRunning;
- (NSNumber *)experimentLoaded;
- (void)toggleConsole:(id)arg;
- (void)updateGUI:(id)arg;
- (NSNumber *)codeForTag:(NSString *)tag;
- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key;
- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                          forVariableCode:(int)code;
- (void)openNetworkPreferences:(id)sender;
- (NSString *)currentNetworkAddress:(id)sender;
- (NSString *)defaultNetworkAddress:(id)sender;
- (void)setListeningAddress:(NSString *)address;
- (NSArray *)variableNames;

@end