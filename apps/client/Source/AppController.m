//
//  AppController.m
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "AppController.h"
#import "MWClientInstance.h"
#import "NSMenuExtensions.h"

#define DEFAULTS_AUTO_CONNECT_TO_LAST_SERVER_KEY @"autoConnectToLastServer"
#define DEFAULTS_AUTO_CLOSE_PLUGIN_WINDOWS_KEY @"autoClosePluginWindows"
#define DEFAULTS_RESTORE_OPEN_PLUGIN_WINDOWS_KEY @"restoreOpenPluginWindows"
#define DEFAULTS_AUTO_OPEN_DATA_FILE_KEY @"autoOpenDataFile"


@implementation AppController


@synthesize preferredWindowHeight;


+ (void)initialize {
    //
    // The class identity test ensures that this method is called only once.  For more info, see
    // http://lists.apple.com/archives/cocoa-dev/2009/Mar/msg01166.html
    //
    if (self == [AppController class]) {
        NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
        
        [defaultValues setObject:[NSNumber numberWithBool:NO] forKey:DEFAULTS_AUTO_CONNECT_TO_LAST_SERVER_KEY];
        [defaultValues setObject:[NSNumber numberWithBool:YES] forKey:DEFAULTS_AUTO_CLOSE_PLUGIN_WINDOWS_KEY];
        [defaultValues setObject:[NSNumber numberWithBool:NO] forKey:DEFAULTS_RESTORE_OPEN_PLUGIN_WINDOWS_KEY];
        [defaultValues setObject:[NSNumber numberWithBool:YES] forKey:DEFAULTS_AUTO_OPEN_DATA_FILE_KEY];
        
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
    }
}


- (BOOL)shouldAutoClosePluginWindows {
    return [[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_AUTO_CLOSE_PLUGIN_WINDOWS_KEY];
}


- (BOOL)shouldRestoreOpenPluginWindows {
    return [[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_RESTORE_OPEN_PLUGIN_WINDOWS_KEY];
}


- (BOOL)shouldAutoOpenDataFile {
    return [[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_AUTO_OPEN_DATA_FILE_KEY];
}


- (void) awakeFromNib {
	[self newClientInstance:self];
    [self setWindowFrameAutosaveName:@"Main Window"];
}

- (IBAction)newClientInstance:(id)sender {

	MWClientInstance *newInstance = [[MWClientInstance alloc] initWithAppController:self];
	[clientInstances addObject:newInstance];
	
	#define preferred_height_offset	40
	#define preferred_height_per_instance 183
	#define max_height  preferred_height_per_instance * 5 + preferred_height_offset
	
	int n_instances = [[clientInstances arrangedObjects] count];
	int preferred_height = preferred_height_per_instance * n_instances + preferred_height_offset;
	if(preferred_height > max_height){
		preferred_height = max_height;
	}
	
	[self setPreferredWindowHeight:preferred_height];
    
    [self setModalClientInstanceInCharge:newInstance];
}

- (void)removeClientInstance:(MWClientInstance *)instance{
    
    [instance hideAllPlugins];
	[instance shutDown];
	[clientInstances removeObject:instance];
}

- (IBAction)launchURLSheetForItem:(NSCollectionViewItem *)item {
		
	sheetOrigin = [[[self window] contentView] convertRect:[[item view] bounds] fromView:[item view]];
	[self setModalClientInstanceInCharge:[item representedObject]];
	
	BOOL is_connected = [modalClientInstanceInCharge serverConnected];
	
	//NSLog(@"is it?: %d", is_connected);
	//NSLog(@"%d", [item representedObject]);
	
	NSWindow *sheet_to_use;
	
	NSString *serverURL = [modalClientInstanceInCharge serverURL];
	NSNumber *serverPort = [modalClientInstanceInCharge serverPort];
	if(serverPort == Nil){
		serverPort = [NSNumber numberWithInteger:0];
	}
	
	NSString *serverPortString = [serverPort stringValue];
	if(serverPortString == Nil){
		serverPortString = @"0";
	}
	
	if(is_connected){
		sheet_to_use = disconnectSheet;
	} else {
		sheet_to_use = urlSheet;
		[modalAddressField setStringValue:serverURL];
		[modalPortField setStringValue:serverPortString];
	}
	
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	[NSApp beginSheet: sheet_to_use
            modalForWindow: [self window]
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
#pragma clang diagnostic pop
}

- (IBAction)closeURLSheet: (id)sender
{
    [NSApp endSheet: urlSheet];
}

- (IBAction)closeDisconnectSheet: (id)sender
{
    [NSApp endSheet: disconnectSheet];
}


- (IBAction)launchExperimentChooserForItem:(NSCollectionViewItem *)item{
	sheetOrigin = [[[self window] contentView] convertRect:[[item view] bounds] fromView:[item view]];
	[self setModalClientInstanceInCharge:[item representedObject]];
	
	BOOL is_loaded = [modalClientInstanceInCharge experimentLoaded];
		
	NSWindow *sheet_to_use;
	
	if(is_loaded){
		sheet_to_use = experimentCloseSheet;
	} else {
		sheet_to_use = experimentLoadSheet;
		//[modalAddressField setStringValue:serverURL];
//		[modalPortField setStringValue:serverPortString];
	}
	
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	[NSApp beginSheet: sheet_to_use
            modalForWindow: [self window]
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
#pragma clang diagnostic pop
}


- (IBAction) closeExperimentLoadSheet: (id)sender{
   [NSApp endSheet: experimentLoadSheet];
}

- (IBAction) closeExperimentCloseSheet: (id)sender{
   [NSApp endSheet: experimentCloseSheet];
}

- (NSOpenPanel *)openPanel {
    if (nil == openPanel) {
        openPanel = [NSOpenPanel openPanel];
    }
    return openPanel;
}

- (IBAction) chooseExperiment: (id) sender {
  
  MWClientInstance *client_instance = [self modalClientInstanceInCharge];
	
  // Create the File Open Dialog class.
  NSOpenPanel* openDlg = [self openPanel];
    
  [openDlg setTitle:@"Choose Experiment"];
  
  // Enable the selection of files in the dialog.
  [openDlg setCanChooseFiles:YES];
  
  // Enable the selection of directories in the dialog.
  [openDlg setCanChooseDirectories:NO];

  [openDlg setAllowsMultipleSelection:NO];
  
  
  // Display the dialog.  If the OK button was pressed,
  // process the files.
  if ( [openDlg runModal] == NSModalResponseOK )
  {
    // Get an array containing the full filenames of all
    // files and directories selected.
    NSArray* files = [openDlg URLs];
    
    if([files count] != 1){
      // TODO: raise hell
      return;
    }
    
    // Loop through all the files and process them.
    for(int i = 0; i < [files count]; i++ )
    {
      NSString* file_name = [[files objectAtIndex:i] path];
      
      if(file_name == Nil){
        return;
      }
      
      [client_instance setExperimentPath:file_name];
      [client_instance loadExperiment];
      [self closeExperimentLoadSheet:self];
      
    }
  }

}

- (IBAction) loadExperiment: (id) sender {

	MWClientInstance *client_instance = [self modalClientInstanceInCharge];
	[client_instance setExperimentPath:[[modalExperimentField URL] relativePath]];

	[client_instance loadExperiment];
	
	[self closeExperimentLoadSheet:self];
}

- (IBAction) loadRecentExperiment: (id) sender {
    NSString *selected_path = [modalRecentExperimentPopUp titleOfSelectedItem];
    if (selected_path == nil)
        return;

	MWClientInstance *client_instance = [self modalClientInstanceInCharge];
	[client_instance setExperimentPath:selected_path];
	[client_instance loadExperiment];
	[self closeExperimentLoadSheet:self];  
}

- (IBAction) closeExperiment: (id) sender {
	MWClientInstance *client_instance = [self modalClientInstanceInCharge];
	
	[client_instance closeExperiment];
	
	[self closeExperimentCloseSheet:self];
}


- (IBAction) launchVariableSetSheetForItem:(NSCollectionViewItem *)item{
    sheetOrigin = [[[self window] contentView] convertRect:[[item view] bounds] fromView:[item view]];
	[self setModalClientInstanceInCharge:[item representedObject]];
	
    [modalNewVariableSetField setStringValue:@""];
    
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	[NSApp beginSheet: variableSetSheet
            modalForWindow: [self window]
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
#pragma clang diagnostic pop
}


- (IBAction) saveToCurrentVariableSet:(id)sender{
    [modalClientInstanceInCharge saveVariableSet];
    [self closeVariableSetSheet:self];
}

- (IBAction) revertToCurrentVariableSet:(id)sender{

    [modalClientInstanceInCharge loadVariableSet];
    [self closeVariableSetSheet:self];
}

- (IBAction) createNewVariableSet:(id)sender{
    NSString *new_set = [modalNewVariableSetField stringValue];

    [modalClientInstanceInCharge setVariableSetName:new_set];
    [modalClientInstanceInCharge saveVariableSet];
    [self closeVariableSetSheet:self];
}

- (IBAction) loadServerSideVariableSet:(id)sender{

    int index = [modalServerSideVariableField indexOfSelectedItem];
    NSString *set_name = [[modalClientInstanceInCharge serversideVariableSetNames] objectAtIndex:index];
    [modalClientInstanceInCharge setVariableSetName:set_name];
    
    [modalClientInstanceInCharge loadVariableSet];
    [self closeVariableSetSheet:self];
}

- (IBAction) loadClientSideVariableSet:(id)sender{

    [self closeVariableSetSheet:self];
}

- (IBAction) closeVariableSetSheet:(id)sender{
    [NSApp endSheet: variableSetSheet];
}


- (IBAction)launchDataFileSheetForItem:(NSCollectionViewItem *)item{
	sheetOrigin = [[[self window] contentView] convertRect:[[item view] bounds] fromView:[item view]];
	[self setModalClientInstanceInCharge:[item representedObject]];
	
	NSWindow *sheet_to_use;
    if (modalClientInstanceInCharge.dataFileWillAutoOpen) {
        sheet_to_use = dataFileAutoOpenSheet;
    } else if (modalClientInstanceInCharge.dataFileOpen) {
		sheet_to_use = dataFileCloseSheet;
	} else {
		sheet_to_use = dataFileOpenSheet;
	}
	
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	[NSApp beginSheet: sheet_to_use
            modalForWindow: [self window]
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
#pragma clang diagnostic pop
}

- (IBAction) closeDataFileAutoOpenSheet: (id)sender {
    [NSApp endSheet: dataFileAutoOpenSheet];
}
- (IBAction) closeDataFileOpenSheet: (id)sender {
	[NSApp endSheet: dataFileOpenSheet];
}
- (IBAction) closeDataFileCloseSheet: (id)sender{
	[NSApp endSheet: dataFileCloseSheet];
}


- (IBAction) openDataFile: (id) sender{
	MWClientInstance *client_instance = [self modalClientInstanceInCharge];
	//NSLog(@"Client instance: %d", client_instance);
	[client_instance setDataFileName:[modalDataFileField stringValue]];
	
	BOOL overwrite = NO;
	if([modalDataFileOverwriteCheckBox state] == NSControlStateValueOn){
		overwrite = YES;
	}
	
	[client_instance setDataFileOverwrite:overwrite];

	[client_instance openDataFile];
	
	[self closeDataFileOpenSheet:self];
}

- (IBAction) closeDataFile: (id) sender{
    MWClientInstance *client_instance = [self modalClientInstanceInCharge];
	[client_instance setDataFileName:[modalDataFileField stringValue]];
	
	[client_instance closeDataFile];
	
	[self closeDataFileCloseSheet:self];
}


- (void)didEndSheet:(NSWindow *)sheet returnCode:(int)returnCode contextInfo:(void *)contextInfo
{
    [sheet orderOut:Nil];
}

- (NSRect)window:(NSWindow *)window willPositionSheet:(NSWindow *)sheet
        usingRect:(NSRect)rect {
    
	NSRect targetRect = sheetOrigin;
    
	targetRect.origin.y = sheetOrigin.origin.y + sheetOrigin.size.height;
	targetRect.size.height = 0;
    return targetRect;
}

- (MWClientInstance *)modalClientInstanceInCharge{
	return modalClientInstanceInCharge;
}

- (void)setModalClientInstanceInCharge:(MWClientInstance *)_instance{
	modalClientInstanceInCharge = _instance;
}

- (IBAction)connect:(id)sender{
	[self closeURLSheet:sender];
	
	[modalClientInstanceInCharge setServerURL:[modalAddressField stringValue]];
	[modalClientInstanceInCharge setServerPort:[NSNumber numberWithInteger:[modalPortField intValue]]];
	[modalClientInstanceInCharge connect];
}

- (IBAction)disconnect:(id)sender{
	[self closeDisconnectSheet:sender];
	
	[modalClientInstanceInCharge disconnect];
}

// Plugins
- (IBAction) launchPluginsMenuForItem:(NSCollectionViewItem *)item{
	[self setModalClientInstanceInCharge:[item representedObject]];
	NSMenu *tempMenu = [pluginsMenu copyWithZone:Nil];
	
	MWClientInstance *a_client = [item representedObject];
	NSArray *pluginWindows = [a_client pluginWindows];
	for(int i = 0; i < [pluginWindows count]; i++){
		NSWindow *window = [[pluginWindows objectAtIndex:i] window];
		
		NSMenuItem *item = [[NSMenuItem alloc] initWithTitle:[window title]
												action:@selector(showPlugin:)
												keyEquivalent:@""];
		[item setTag:i];
		
		[tempMenu insertItem:item atIndex:0];
	}
	[NSMenu popUpMenu:tempMenu forView:[item view] pullsDown:NO];
}

- (IBAction) showPlugin:(id)sender {
	int tag = [sender tag];
    
	MWClientInstance *instance = [self modalClientInstanceInCharge];
	[instance showPlugin:tag];
}

- (IBAction) showAllPlugins:(id)sender {
	MWClientInstance *instance = [self modalClientInstanceInCharge];
	[instance showAllPlugins];
	
}

- (IBAction) showGroupedPlugins:(id)sender {
	MWClientInstance *instance = [self modalClientInstanceInCharge];
	[instance showGroupedPlugins];
}


- (IBAction) hideAllPlugins:(id)sender {
	MWClientInstance *instance = [self modalClientInstanceInCharge];
	[instance hideAllPlugins];
}


// Error Handling
- (void)launchErrorSheetForItem:(NSCollectionViewItem *)item{

	sheetOrigin = [[[self window] contentView] convertRect:[[item view] bounds] fromView:[item view]];
	
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
	[NSApp beginSheet: errorSheet
            modalForWindow: [self window]
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
#pragma clang diagnostic pop
}

- (IBAction)closeErrorSheet:(id)sender{
	[NSApp endSheet: errorSheet];
}


- (NSColor *)uniqueColor {
    int ind = [[clientInstances arrangedObjects] count] % 6;
    return [NSColor colorNamed:[NSString stringWithFormat:@"uniqueColor%d", ind]];
}


- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
	return YES;
}


- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
    // We need to check for a loaded experiment to prevent the user from using the recent documents menu to open
    // a new workspace on top of an existing one.  A better approach would be to disable all items in the recent
    // documents menu when a workspace is loaded (as we do with the "Open Workspace" menu item); however, I haven't
    // been able to figure out how to do that.
    
    if (![modalClientInstanceInCharge experimentLoaded]) {
        
        [self loadWorkspaceFromURL:[NSURL fileURLWithPath:filename]];
        
    } else {
        
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setMessageText:@"Workspace already loaded"];
        [alert setInformativeText:@"Please close the current experiment before attempting to load a new workspace."];
        [alert runModal];
        
    }
    
    return YES;
}


- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Take a power assertion to prevent the OS from throttling long-running event listener and I/O threads
    ioActivity = [NSProcessInfo.processInfo beginActivityWithOptions:(NSActivityBackground | NSActivityIdleSystemSleepDisabled)
                                                              reason:@"Prevent I/O throttling"];
    
    if ([[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_AUTO_CONNECT_TO_LAST_SERVER_KEY]) {
        [modalClientInstanceInCharge connect];
    }
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    for (MWClientInstance *client in [clientInstances arrangedObjects]) {
        [client shutDown];
    }
    
    [NSProcessInfo.processInfo endActivity:ioActivity];
}


- (IBAction)launchDocs:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL:[NSURL fileURLWithPath:MWORKS_DOC_PATH isDirectory:NO]];
}


- (IBAction) launchHelp: (id) sender {
  //NSLog(@"Launching Help...");
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:MWORKS_HELP_URL]];
}


- (IBAction)openWorkspace:(id)sender {
    NSOpenPanel *openDlg = [self openPanel];
    [openDlg setTitle:@"Open Workspace"];
    if ([openDlg runModal] == NSModalResponseOK) {
        [self loadWorkspaceFromURL:[[openPanel URLs] lastObject]];
    }
}


- (void)loadWorkspaceFromURL:(NSURL *)workspaceURL {
    [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL:workspaceURL];
    
    NSData *workspaceData;
    NSDictionary *workspaceInfo;
    NSError *error = nil;
    
    if (!(workspaceData = [NSData dataWithContentsOfURL:workspaceURL options:0 error:&error]) ||
        !(workspaceInfo = [NSJSONSerialization JSONObjectWithData:workspaceData options:0 error:nil]) ||
        ![workspaceInfo isKindOfClass:[NSDictionary class]])
    {
        if (!error) {
            error = [NSError errorWithDomain:NSCocoaErrorDomain
                                        code:NSFileReadCorruptFileError
                                    userInfo:[NSDictionary dictionaryWithObject:workspaceURL forKey:NSURLErrorKey]];
        }
        [self.window performSelectorOnMainThread:@selector(presentError:) withObject:error waitUntilDone:NO];
        return;
    }
    
    // While loading the workspace, we set the current directory to the directory in which the workspace
    // definition file resides, so that paths relative to that location will resolve correctly
    
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *oldCurrentDirectoryPath = [fileManager currentDirectoryPath];
    [fileManager changeCurrentDirectoryPath:[[workspaceURL path] stringByDeletingLastPathComponent]];
    
    @try {
        [[self modalClientInstanceInCharge] loadWorkspace:workspaceInfo];
    }
    @finally {
        if (oldCurrentDirectoryPath) {
            [fileManager changeCurrentDirectoryPath:oldCurrentDirectoryPath];
        }
    }
}


- (IBAction)saveWorkspace:(id)sender {
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    [savePanel setTitle:@"Save Workspace"];
    [savePanel setAllowedFileTypes:[NSArray arrayWithObject:@"json"]];
    [savePanel setAllowsOtherFileTypes:YES];
    
    if ([savePanel runModal] != NSModalResponseOK) {
        return;
    }
    
    NSData *workspaceData;
    NSError *error;
    if (!(workspaceData = [NSJSONSerialization dataWithJSONObject:[[self modalClientInstanceInCharge] workspaceInfo]
                                                     options:NSJSONWritingPrettyPrinted
                                                       error:&error]) ||
        ![workspaceData writeToURL:[savePanel URL] options:0 error:&error])
    {
        [self.window performSelectorOnMainThread:@selector(presentError:) withObject:error waitUntilDone:NO];
        return;
    }
}


- (IBAction)closeWorkspace:(id)sender {
    [[self modalClientInstanceInCharge] closeWorkspace];
}


@end


























