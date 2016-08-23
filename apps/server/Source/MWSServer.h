/*!
* @header MWSServer
 *
 * @discussion Provides access to a shared instance of the MWSSever object
 * which contains controller objects for important parts of the system.
 *
 * bkennedy on 07/05/07 - Created.
 */

#import <Cocoa/Cocoa.h>

#import <MWorksCocoa/MWClientServerBase.h>


@interface MWSServer : NSObject <NSApplicationDelegate, MWClientServerBase>

@property(nonatomic, weak) IBOutlet NSWindow *preferencesWindow;

- (IBAction)openExperiment:(id)sender;
- (IBAction)closeExperiment:(id)sender;
- (IBAction)saveVariables:(id)sender;
- (IBAction)loadVariables:(id)sender;

- (IBAction)openDataFile:(id)sender;
- (IBAction)closeDataFile:(id)sender;
- (IBAction)stopExperiment:(id)sender;
- (IBAction)startExperiment:(id)sender;
- (IBAction)launchDocs:(id)sender;
- (IBAction)launchHelp:(id)sender;

- (IBAction)togglePreferences:(id)sender;
- (IBAction)toggleConsole:(id)sender;

@end



























