//
//  MWPreferencePaneController.m
//  NewEditor
//
//  Created by David Cox on 9/12/08.
//  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
//

#import "MWPreferencePanelController.h"


@implementation MWPreferencePanelController

@synthesize selectedTab;

-(void) awakeFromNib{

    NSArray *items = [toolbar items];
    [toolbar setSelectedItemIdentifier:[[items objectAtIndex:0] itemIdentifier]];
    
}

-(IBAction) switchToGeneralPane:(id)sender{
    self.selectedTab = 0;
}

-(IBAction) switchToErrorsPane:(id)sender{
    self.selectedTab = 1;
}

- (NSArray *)toolbarSelectableItemIdentifiers:(NSToolbar *)toolbar{
    NSArray *items = [toolbar items];
    NSMutableArray *item_identifiers = [[NSMutableArray alloc] init];
    
    NSEnumerator *e = [items objectEnumerator];
    NSToolbarItem *item;
    while(item = (NSToolbarItem *)[e nextObject]){
        [item_identifiers addObject:[item itemIdentifier]];
    }
    
    return item_identifiers;
}

- (IBAction)showPreferencesWindow:(id)sender{
    [preferencesWindow orderFront:sender];
}


@end
