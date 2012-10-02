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

#define DEFAULTS_AUTO_CLOSE_PLUGIN_WINDOWS_KEY @"autoClosePluginWindows"
#define DEFAULTS_RESTORE_OPEN_PLUGIN_WINDOWS_KEY @"restoreOpenPluginWindows"


@implementation AppController


@synthesize preferredWindowHeight;


+ (void)initialize {
    //
    // The class identity test ensures that this method is called only once.  For more info, see
    // http://lists.apple.com/archives/cocoa-dev/2009/Mar/msg01166.html
    //
    if (self == [AppController class]) {
        NSMutableDictionary *defaultValues = [NSMutableDictionary dictionary];
        
        [defaultValues setObject:[NSNumber numberWithBool:YES] forKey:DEFAULTS_AUTO_CLOSE_PLUGIN_WINDOWS_KEY];
        [defaultValues setObject:[NSNumber numberWithBool:NO] forKey:DEFAULTS_RESTORE_OPEN_PLUGIN_WINDOWS_KEY];
        
        [[NSUserDefaults standardUserDefaults] registerDefaults:defaultValues];
    }
}


- (BOOL)shouldAutoClosePluginWindows {
    return [[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_AUTO_CLOSE_PLUGIN_WINDOWS_KEY];
}


- (BOOL)shouldRestoreOpenPluginWindows {
    return [[NSUserDefaults standardUserDefaults] boolForKey:DEFAULTS_RESTORE_OPEN_PLUGIN_WINDOWS_KEY];
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
}

- (void)removeClientInstance:(MWClientInstance *)instance{
    
    [instance hideAllPlugins];
	[instance shutDown];
	[clientInstances removeObject:instance];
}


- (IBAction) openConnectionList:(id)sender {

}

- (IBAction) saveConnectionlist:(id)sender {

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
	
	[NSApp beginSheet: sheet_to_use
            modalForWindow: [self window]
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
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
	
	NSString *experimentPath = [modalClientInstanceInCharge experimentPath];
	
	
	if(experimentPath == Nil){
		experimentPath = @"/";
	}
	
	if(is_loaded){
		sheet_to_use = experimentCloseSheet;
	} else {
		sheet_to_use = experimentLoadSheet;
		//[modalAddressField setStringValue:serverURL];
//		[modalPortField setStringValue:serverPortString];
	}
	
	[NSApp beginSheet: sheet_to_use
            modalForWindow: [self window]
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
}


- (IBAction) closeExperimentLoadSheet: (id)sender{
   [NSApp endSheet: experimentLoadSheet];
}

- (IBAction) closeExperimentCloseSheet: (id)sender{
   [NSApp endSheet: experimentCloseSheet];
}

- (NSOpenPanel *)openPanel {
    if (nil == openPanel) {
        openPanel = [[NSOpenPanel openPanel] retain];
    }
    return openPanel;
}

- (IBAction) chooseExperiment: (id) sender {
  
  MWClientInstance *client_instance = [self modalClientInstanceInCharge];
	
  // Create the File Open Dialog class.
  NSOpenPanel* openDlg = [self openPanel];
  
  // Enable the selection of files in the dialog.
  [openDlg setCanChooseFiles:YES];
  
  // Enable the selection of directories in the dialog.
  [openDlg setCanChooseDirectories:NO];

  [openDlg setAllowsMultipleSelection:NO];
  
  
  // Display the dialog.  If the OK button was pressed,
  // process the files.
  if ( [openDlg runModal] == NSFileHandlingPanelOKButton )
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
    
	[NSApp beginSheet: variableSetSheet
            modalForWindow: [self window]
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
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
    [modalClientInstanceInCharge loadVariableSet];
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
	
	BOOL is_loaded = [modalClientInstanceInCharge dataFileOpen];
		
	NSWindow *sheet_to_use;
	
	NSString *dataFileName = [modalClientInstanceInCharge dataFileName];
	
	
	if(dataFileName == Nil){
		dataFileName = @"/";
	}
	
	if(is_loaded){
		sheet_to_use = dataFileCloseSheet;
	} else {
		sheet_to_use = dataFileOpenSheet;
	}
	
	
	[NSApp beginSheet: sheet_to_use
            modalForWindow: [self window]
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
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
	if([modalDataFileOverwriteCheckBox state] == NSOnState){
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
	
	[NSApp beginSheet: errorSheet
            modalForWindow: [self window]
            modalDelegate: self
            didEndSelector: @selector(didEndSheet:returnCode:contextInfo:)
            contextInfo: nil];
}

- (IBAction)closeErrorSheet:(id)sender{
	[NSApp endSheet: errorSheet];
}

// TODO: kludgey, make better
- (NSColor *)uniqueColor {
    
    int ind = [[clientInstances arrangedObjects] count] % 6;
    //int ind = colorIndex%6;
    //colorIndex++;
    
    
    if(ind == 1){
        return [NSColor colorWithDeviceRed:194./255. green:192./255 blue:129./255 alpha:1.0];
    }
    
    if(ind == 2){
        return [NSColor colorWithDeviceRed:209./255. green:206./255 blue:183./255 alpha:1.0];
    }
    
    if(ind == 3){
        return [NSColor colorWithDeviceRed:189./255. green:181./255 blue:219./255 alpha:1.0];
    }
    
    if(ind == 4){
        return [NSColor colorWithDeviceRed:196./255. green:225./255 blue:178./255 alpha:1.0];
    }
    
    if(ind == 5){
        return [NSColor colorWithDeviceRed:181./255. green:181./255 blue:181./255 alpha:1.0];
    }
    
    return [NSColor colorWithDeviceRed:180./255. green:197./255 blue:211./255 alpha:1.0];
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication {
	return YES;
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    for (MWClientInstance *client in [clientInstances arrangedObjects]) {
        [client shutDown];
    }
}


- (IBAction) launchHelp: (id) sender {
  //NSLog(@"Launching Help...");
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:HELP_URL]];
}


@end


























