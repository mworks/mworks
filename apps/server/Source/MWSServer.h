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


@interface MWSServer : NSObject <NSApplicationDelegate, NSTabViewDelegate, MWClientServerBase>

@property(nonatomic, weak) IBOutlet NSWindow *preferencesWindow;
@property(nonatomic, weak) IBOutlet NSTabView *preferencesWindowTabView;
@property(nonatomic, weak) IBOutlet NSWindow *advancedDisplayPreferencesWindow;

- (IBAction)launchDocs:(id)sender;
- (IBAction)launchHelp:(id)sender;

- (IBAction)togglePreferences:(id)sender;
- (IBAction)showAdvancedDisplayPreferences:(id)sender;
- (IBAction)hideAdvancedDisplayPreferences:(id)sender;
- (IBAction)toggleConsole:(id)sender;

@end



























