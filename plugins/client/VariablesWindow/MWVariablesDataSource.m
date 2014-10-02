//
//  MWVariablesDataSource.m
//  MWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWVariablesDataSource.h"
#import "MWVariableDisplayItem.h"
#import "MWVariablesWindowController.h"

#import <MWorksCore/ExpressionVariable.h>

#define DEFAULTS_EXPANDED_ITEMS_KEY @"Variables Window - expanded items"


@implementation MWVariablesDataSource


- (id)init {
    if ((self = [super init])) {
        rootItems = [[NSMutableArray alloc] init];
        expandedItems = [[NSMutableArray alloc] init];
        
        NSArray *restoredExpandedItems = [[NSUserDefaults standardUserDefaults] arrayForKey:DEFAULTS_EXPANDED_ITEMS_KEY];
        if (restoredExpandedItems) {
            [expandedItems addObjectsFromArray:restoredExpandedItems];
        }
    }
    
    return self;
}


- (MWVariablesWindowController *)delegate {
	return delegate;
}

- (void)setDelegate:(MWVariablesWindowController *)new_delegate {
	delegate = new_delegate;
}


- (void)setRootGroups:(NSDictionary *)rootGroups forOutlineView:(NSOutlineView *)outlineView {
    NSMutableDictionary *oldRootObjects = [NSMutableDictionary dictionaryWithCapacity:[rootItems count]];
    for (MWVariableDisplayItem *item in rootItems) {
        [oldRootObjects setObject:item forKey:item.displayName];
    }
    
	[rootItems removeAllObjects];
    
	for (NSString *key in rootGroups) {
        MWVariableDisplayItem *item = [oldRootObjects objectForKey:key];
        if (!item) {
            item = [[[MWVariableDisplayItem alloc] initWithName:key] autorelease];
        }
        [item setVariables:[rootGroups objectForKey:key]];
		[rootItems addObject:item];
	}
    
    [outlineView reloadData];
    
    for (MWVariableDisplayItem *item in rootItems) {
        if (NSNotFound != [expandedItems indexOfObject:item.displayName]) {
            [outlineView expandItem:item];
        }
    }
}


- (NSArray *)expandedItems
{
    return [[expandedItems copy] autorelease];
}


- (void)setExpandedItems:(NSArray *)items forOutlineView:(NSOutlineView *)outlineView
{
    [expandedItems removeAllObjects];
    [expandedItems addObjectsFromArray:items];
    [[NSUserDefaults standardUserDefaults] setObject:expandedItems forKey:DEFAULTS_EXPANDED_ITEMS_KEY];
    
    for (MWVariableDisplayItem *item in rootItems) {
        if (NSNotFound != [expandedItems indexOfObject:item.displayName]) {
            [outlineView expandItem:item];
        } else {
            [outlineView collapseItem:item];
        }
    }
}


// DataSource overridden methods
- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
    if (item == nil) {
		return [rootItems count];
	} else {
		return [item numberOfChildren];
	}
	
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item
{
	if(item == nil) {
		return NO;
	} else {
		return ([item numberOfChildren] != -1);
	}
	
}

- (id)outlineView:(NSOutlineView *)outlineView
			child:(NSInteger)index
		   ofItem:(id)item
{
	if(item == nil) {
		MWVariableDisplayItem *item = [rootItems objectAtIndex:index];
		return item;
	} else {
		return [item childAtIndex:index];
	}
}

- (id)outlineView:(NSOutlineView *)outlineView
objectValueForTableColumn:(NSTableColumn *)tableColumn
		   byItem:(id)item
{
	if(item == nil) {
		if(tableColumn == nameCol) {
			return @"Variables";
		} else {
			return @"Value";
		}
	} else {
		if(tableColumn == nameCol) {
			if(item == Nil || ![item respondsToSelector:@selector(displayName)]){
				return @"";
			}
			return [item displayName];
		} else {
			if([item numberOfChildren] != -1) { // not a leaf
				return @"";
			} else {
				if(delegate != nil) {
					return [delegate getValueString:[item displayName]];
				} else {
					return @"";
				}
			}
		}
	}
}

- (void)outlineView:(NSOutlineView *)outlineView 
	 setObjectValue:(id)_object 
	 forTableColumn:(NSTableColumn *)tableColumn 
			 byItem:(id)item
{
    if (!delegate || (tableColumn != valueCol)) {
        return;
    }
    
    NSString *object = (NSString *)_object;
    const char *objectUTF8 = [object UTF8String];
    mw::Datum setval;
    
    try {
        setval = ParsedExpressionVariable::evaluateExpression(objectUTF8);
    } catch (const SimpleException &) {
        setval.setString(objectUTF8);
    }
    
    [delegate set:[item displayName] toValue:&setval];
}


- (void)outlineViewItemDidExpand:(NSNotification *)notification
{
    MWVariableDisplayItem *item = [[notification userInfo] objectForKey:@"NSObject"];
    if (NSNotFound == [expandedItems indexOfObject:item.displayName]) {
        [expandedItems addObject:item.displayName];
        [[NSUserDefaults standardUserDefaults] setObject:expandedItems forKey:DEFAULTS_EXPANDED_ITEMS_KEY];
    }
}


- (void)outlineViewItemDidCollapse:(NSNotification *)notification
{
    MWVariableDisplayItem *item = [[notification userInfo] objectForKey:@"NSObject"];
    if (NSNotFound != [expandedItems indexOfObject:item.displayName]) {
        [expandedItems removeObject:item.displayName];
        [[NSUserDefaults standardUserDefaults] setObject:expandedItems forKey:DEFAULTS_EXPANDED_ITEMS_KEY];
    }
}


@end




























