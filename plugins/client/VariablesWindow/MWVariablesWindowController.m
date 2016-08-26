#import "MWVariablesWindowController.h"

#include <MWorksCore/ExpressionVariable.h>

#import <MWorksCocoa/MWCodec.h>

#import "MWVariableDisplayItem.h"

#define DEFAULTS_EXPANDED_ITEMS_KEY @"Variables Window - expanded items"
#define VARIABLES_WINDOW_CALLBACK_KEY "MWorksVariableWindowCallbackKey"


@implementation MWVariablesWindowController {
    MWCodec *variables;
    NSMutableArray *rootItems;
    NSMutableArray *expandedItems;
}


- (void)awakeFromNib {
    variables = [self.client variables];
    rootItems = [[NSMutableArray alloc] init];
    expandedItems = [[NSMutableArray alloc] init];
    
    NSArray *restoredExpandedItems = [[NSUserDefaults standardUserDefaults] arrayForKey:DEFAULTS_EXPANDED_ITEMS_KEY];
    if (restoredExpandedItems) {
        [expandedItems addObjectsFromArray:restoredExpandedItems];
    }
    
    NSTimer *reloadTimer = [NSTimer timerWithTimeInterval:0.75
                                                   target:self
                                                 selector:@selector(causeDataReload:)
                                                 userInfo:nil
                                                  repeats:YES];
    [[NSRunLoop mainRunLoop] addTimer:reloadTimer forMode:NSDefaultRunLoopMode];
    
    [self.client registerEventCallbackWithReceiver:self
                                          selector:@selector(updateRootItems:)
                                       callbackKey:VARIABLES_WINDOW_CALLBACK_KEY
                                   forVariableCode:RESERVED_CODEC_CODE
                                      onMainThread:YES];
}


- (void)causeDataReload:(NSTimer *)timer {
    [self.outlineView setNeedsDisplay:YES];
}


- (void)updateRootItems:(MWCocoaEvent *)event {
    NSMutableDictionary *oldRootObjects = [NSMutableDictionary dictionaryWithCapacity:[rootItems count]];
    for (MWVariableDisplayItem *item in rootItems) {
        [oldRootObjects setObject:item forKey:item.displayName];
    }
    
    [rootItems removeAllObjects];
    
    NSDictionary *rootGroups = [self.client varGroups];
    for (NSString *key in rootGroups) {
        MWVariableDisplayItem *item = [oldRootObjects objectForKey:key];
        if (!item) {
            item = [[MWVariableDisplayItem alloc] initWithDisplayName:key];
        }
        [item setVariables:[rootGroups objectForKey:key]];
        [rootItems addObject:item];
    }
    
    [self.outlineView reloadData];
    
    for (MWVariableDisplayItem *item in rootItems) {
        if (NSNotFound != [expandedItems indexOfObject:item.displayName]) {
            [self.outlineView expandItem:item];
        }
    }
}


//
// NSOutlineViewDataSource methods
//


- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item {
    if (item) {
        return [item numberOfChildren];
    }
    return [rootItems count];
}


- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item {
    return ([item numberOfChildren] != -1);
}


- (id)outlineView:(NSOutlineView *)outlineView
            child:(NSInteger)index
           ofItem:(id)item
{
    if (item) {
        return [item childAtIndex:index];
    }
    return [rootItems objectAtIndex:index];
}


- (id)outlineView:(NSOutlineView *)outlineView
objectValueForTableColumn:(NSTableColumn *)tableColumn
           byItem:(id)item
{
    if (tableColumn == self.nameColumn) {
        return [item displayName];
    }
    
    if ([item numberOfChildren] != -1) { // not a leaf
        return @"";
    }
    
    mw::Datum value = [variables valueForVariable:[item displayName]];
    return [NSString stringWithUTF8String:(value.toString(true).c_str())];
}


- (void)outlineView:(NSOutlineView *)outlineView
     setObjectValue:(id)object
     forTableColumn:(NSTableColumn *)tableColumn
             byItem:(id)item
{
    if (tableColumn != self.valueColumn) {
        return;
    }
    
    const char *objectUTF8 = [(NSString *)object UTF8String];
    mw::Datum value;
    
    try {
        value = mw::ParsedExpressionVariable::evaluateExpression(objectUTF8);
    } catch (const mw::SimpleException &) {
        value.setString(objectUTF8);
    }
    
    [variables setValue:value forVariable:[item displayName]];
}


//
// NSOutlineViewDelegate methods
//


- (void)outlineViewItemDidExpand:(NSNotification *)notification {
    MWVariableDisplayItem *item = [[notification userInfo] objectForKey:@"NSObject"];
    if (NSNotFound == [expandedItems indexOfObject:item.displayName]) {
        [expandedItems addObject:item.displayName];
        [[NSUserDefaults standardUserDefaults] setObject:expandedItems forKey:DEFAULTS_EXPANDED_ITEMS_KEY];
    }
}


- (void)outlineViewItemDidCollapse:(NSNotification *)notification {
    MWVariableDisplayItem *item = [[notification userInfo] objectForKey:@"NSObject"];
    if (NSNotFound != [expandedItems indexOfObject:item.displayName]) {
        [expandedItems removeObject:item.displayName];
        [[NSUserDefaults standardUserDefaults] setObject:expandedItems forKey:DEFAULTS_EXPANDED_ITEMS_KEY];
    }
}


- (BOOL)outlineView:(NSOutlineView *)outlineView
shouldEditTableColumn:(NSTableColumn *)tableColumn
               item:(id)item
{
    return (tableColumn == self.valueColumn) && ([item numberOfChildren] == -1);
}


//
// MWClientPluginWorkspaceState methods
//


- (NSDictionary *)workspaceState {
    NSMutableDictionary *workspaceState = [NSMutableDictionary dictionary];
    
    NSArray *expandedGroups = [expandedItems copy];
    if ([expandedGroups count] > 0) {
        [workspaceState setObject:expandedGroups forKey:@"expandedGroups"];
    }
    
    return workspaceState;
}


- (void)setWorkspaceState:(NSDictionary *)workspaceState {
    NSArray *expandedGroups = [workspaceState objectForKey:@"expandedGroups"];
    if (expandedGroups && [expandedGroups isKindOfClass:[NSArray class]]) {
        [expandedItems removeAllObjects];
        [expandedItems addObjectsFromArray:expandedGroups];
        [[NSUserDefaults standardUserDefaults] setObject:expandedItems forKey:DEFAULTS_EXPANDED_ITEMS_KEY];
        
        for (MWVariableDisplayItem *item in rootItems) {
            if (NSNotFound != [expandedItems indexOfObject:item.displayName]) {
                [self.outlineView expandItem:item];
            } else {
                [self.outlineView collapseItem:item];
            }
        }
    }
}


@end


























