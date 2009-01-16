//
//  MWGroupedPluginWindowController.h
//  New Client
//
//  Created by David Cox on 5/29/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWGroupedPluginWindowController : NSWindowController {

    NSMutableArray *plugins;
    NSMutableArray *names;
    NSMutableArray *holdFlags;
    
    IBOutlet NSBox *content_box;
    IBOutlet NSPopUpButton *plugin_selector;

    int currentPluginIndex;
    
    id clientInstance;
    
    NSTimer *cycleTimer;
    
    double cycleTime;
    BOOL cycling;
    
    // Cosmetics
    IBOutlet NSBox *headerBox;
    NSColor *headerColor;
}

@property(assign) int currentPluginIndex;
@property(retain) NSArray* plugins;
@property(retain) NSArray* names;
@property(assign) double cycleTime;
@property(assign) BOOL cycling;
@property(assign) id clientInstance;

- (void) addPluginWindow:(NSWindow *)window withName:(NSString *)name;
- (void) changeViewToPlugin:(int)i;

- (IBAction) incrementPlugin:(id)sender;
- (IBAction) previousPlugin:(id)sender;
- (IBAction) nextPlugin:(id)sender;

// - (NSString *)clientName;

- (void)hideWindow;

- (void)setCustomColor:(NSColor *)color;



@end
