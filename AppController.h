//
//  AppController.h
//  New Client
//
//  Created by David Cox on 3/10/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <MonkeyWorksCocoa/MWToolbarItem.h>

@class MWClientInstance;

@interface AppController : NSWindowController {
	IBOutlet NSArrayController *clientInstances;
	
	IBOutlet NSWindow *urlSheet;
	IBOutlet NSWindow *disconnectSheet;
	
	IBOutlet NSWindow *experimentLoadSheet;
	IBOutlet NSWindow *experimentCloseSheet;
	
    IBOutlet NSWindow *variableSetSheet;
    
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
	IBOutlet NSComboBox *modalExperimentField;
	
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
}


@property int preferredWindowHeight;

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
- (IBAction) openExperimentChooserOpenPanel: (id)sender;
- (IBAction) loadExperiment: (id) sender;
- (IBAction) closeExperiment: (id) sender;


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


- (NSColor *)uniqueColor;

@end
