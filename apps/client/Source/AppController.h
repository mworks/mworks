//
//  AppController.h
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

@import Cocoa;

#define MWORKS_DOC_PATH @"/Library/Application Support/MWorks/Documentation/index.html"
#define MWORKS_HELP_URL @"https://mworks.tenderapp.com/"

@class MWClientInstance;

@interface AppController : NSWindowController {
	
  
    IBOutlet NSArrayController *clientInstances;
	
	IBOutlet NSWindow *urlSheet;
	IBOutlet NSWindow *disconnectSheet;
	
	IBOutlet NSWindow *experimentLoadSheet;
	IBOutlet NSWindow *experimentCloseSheet;
	
    IBOutlet NSWindow *variableSetSheet;
    
    IBOutlet NSWindow *dataFileAutoOpenSheet;
	IBOutlet NSWindow *dataFileOpenSheet;
	IBOutlet NSWindow *dataFileCloseSheet;
	
	IBOutlet NSWindow *errorSheet;

	// When we select a modal sheet, we'll keep track of which client instance
	// is "in charge," and where it's sheet should originate from
	NSRect sheetOrigin;
	MWClientInstance *modalClientInstanceInCharge;
	
	// Server connect / disconnect
	IBOutlet NSComboBox *modalAddressField;
	IBOutlet NSComboBox *modalPortField;

	// Experiment load
	IBOutlet NSPathControl *modalExperimentField;
    IBOutlet NSPopUpButton *modalRecentExperimentPopUp;
    NSOpenPanel *openPanel;
	
	// Data File open
	IBOutlet NSComboBox *modalDataFileField;
	IBOutlet NSButton *modalDataFileOverwriteCheckBox;
	
    // Variable Set Fields
    IBOutlet NSTextField *modalNewVariableSetField;
    IBOutlet NSPopUpButton *modalServerSideVariableField;
    
	int preferredWindowHeight;
	
	// Plugins
	IBOutlet NSMenu *pluginsMenu;
	
    // Cosmetics
    int colorIndex;
    
    id<NSObject> ioActivity;
}


@property int preferredWindowHeight;
@property(nonatomic, readonly) BOOL shouldAutoClosePluginWindows;
@property(nonatomic, readonly) BOOL shouldRestoreOpenPluginWindows;
@property(nonatomic, readonly) BOOL shouldAutoOpenDataFile;

- (void) awakeFromNib;

// Adding and removing client instances
- (IBAction)newClientInstance:(id)sender;
- (void)removeClientInstance:(MWClientInstance *)instance;

// Connect / Disconnect from Server
- (IBAction)launchURLSheetForItem:(NSCollectionViewItem *)item;
- (IBAction)closeURLSheet: (id)sender;
- (IBAction)closeDisconnectSheet: (id)sender;

// Open / Close Experiment File
- (IBAction)launchExperimentChooserForItem:(NSCollectionViewItem *)item;
- (IBAction) closeExperimentLoadSheet: (id)sender;
- (IBAction) closeExperimentCloseSheet: (id)sender;
- (IBAction) loadExperiment: (id) sender;
- (IBAction) chooseExperiment: (id) sender;
- (IBAction) closeExperiment: (id) sender;
- (IBAction) loadRecentExperiment: (id) sender;
- (NSOpenPanel *)openPanel;


// Variable sets
- (IBAction) launchVariableSetSheetForItem:(NSCollectionViewItem *)item;
- (IBAction) saveToCurrentVariableSet:(id)sender;
- (IBAction) revertToCurrentVariableSet:(id)sender;
- (IBAction) createNewVariableSet:(id)sender;
- (IBAction) loadServerSideVariableSet:(id)sender;
- (IBAction) loadClientSideVariableSet:(id)sender;
- (IBAction) closeVariableSetSheet:(id)sender;

// Open / Close Data File
- (IBAction)launchDataFileSheetForItem:(NSCollectionViewItem *)item;
- (IBAction) closeDataFileAutoOpenSheet: (id)sender;
- (IBAction) closeDataFileOpenSheet: (id)sender;
- (IBAction) closeDataFileCloseSheet: (id)sender;
- (IBAction) openDataFile: (id) sender;
- (IBAction) closeDataFile: (id) sender;


- (MWClientInstance *)modalClientInstanceInCharge;
- (void)setModalClientInstanceInCharge:(MWClientInstance *)instance;

- (IBAction)connect:(id)sender;
- (IBAction)disconnect:(id)sender;

// Plugins
- (IBAction) launchPluginsMenuForItem:(NSCollectionViewItem *)item;
- (IBAction) showAllPlugins:(id)sender;
- (IBAction) showGroupedPlugins:(id)sender;
- (IBAction) hideAllPlugins:(id)sender;

- (IBAction)closeErrorSheet:(id)sender;


- (IBAction)launchDocs:(id)sender;
- (IBAction)launchHelp:(id)sender;

- (NSColor *)uniqueColor;

- (IBAction)openWorkspace:(id)sender;
- (void)loadWorkspaceFromURL:(NSURL *)workspaceURL;
- (IBAction)saveWorkspace:(id)sender;
- (IBAction)closeWorkspace:(id)sender;

@end




























