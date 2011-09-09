//
//  MWPreferencePaneController.h
//  NewEditor
//
//  Created by David Cox on 9/12/08.
//  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWPreferencePanelController : NSObject {
    
    IBOutlet NSWindow *preferencesWindow;
    IBOutlet NSToolbar *toolbar;
    int selectedTab;
}

@property int selectedTab;

-(IBAction) switchToGeneralPane:(id)sender;
-(IBAction) switchToErrorsPane:(id)sender;
-(IBAction) showPreferencesWindow:(id)sender;

@end
