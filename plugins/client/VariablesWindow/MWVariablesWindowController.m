#import "MWVariablesWindowController.h"

#include <MWorksCore/ExpressionVariable.h>

#import <MWorksCocoa/MWCodec.h>

#import "MWVariableDisplayItem.h"

#define DEFAULTS_EXPANDED_ITEMS_KEY @"Variables Window - expanded items"
#define VARIABLES_WINDOW_CALLBACK_KEY "MWorksVariableWindowCallbackKey"

#define NAME_COLUMN_INDEX 0
#define VALUE_COLUMN_INDEX 1


@implementation MWVariablesWindowController {
    MWCodec *variables;
    
    NSMutableArray<MWVariableDisplayItem *> *rootItems;
    NSMutableArray<NSString *> *expandedItems;
    
    NSMutableArray<MWVariableDisplayItem *> *allVariablesItems;
    NSString *searchText;
    NSArray<MWVariableDisplayItem *> *searchResultItems;
    
    BOOL alreadyAwake;
}


- (void)awakeFromNib {
    // NSTableView's makeViewWithIdentifier:owner: method (invoked via outlineView:viewForTableColumn:item:) invokes
    // awakeFromNib every time it's called.  Check alreadyAwake to ensure that our initialization tasks execute only
    // once.
    if (alreadyAwake) {
        return;
    }
    
    // For some reason, the outline view's target and action aren't set correctly when running under OS X 10.9,
    // so check for and, if necessary, manually make the connections here
    if (!self.outlineView.target) {
        self.outlineView.target = self;
    }
    if (!self.outlineView.doubleAction) {
        self.outlineView.doubleAction = @selector(doubleClickOnOutlineView:);
    }
    
    variables = [self.client variables];
    rootItems = [[NSMutableArray alloc] init];
    expandedItems = [[NSMutableArray alloc] init];
    searchText = @"";
    
    NSArray *restoredExpandedItems = [[NSUserDefaults standardUserDefaults] arrayForKey:DEFAULTS_EXPANDED_ITEMS_KEY];
    if (restoredExpandedItems) {
        [expandedItems addObjectsFromArray:restoredExpandedItems];
    }
    
    NSTimer *reloadTimer = [NSTimer timerWithTimeInterval:0.75
                                                   target:self
                                                 selector:@selector(updateVariableValues:)
                                                 userInfo:nil
                                                  repeats:YES];
    [[NSRunLoop mainRunLoop] addTimer:reloadTimer forMode:NSDefaultRunLoopMode];
    
    [self.client registerEventCallbackWithReceiver:self
                                          selector:@selector(updateRootItems:)
                                       callbackKey:VARIABLES_WINDOW_CALLBACK_KEY
                                   forVariableCode:RESERVED_CODEC_CODE
                                      onMainThread:YES];
    
    alreadyAwake = YES;
}


#pragma mark - Action methods


- (IBAction)doubleClickOnOutlineView:(id)sender {
    NSInteger row = self.outlineView.clickedRow;
    if (row >= 0 && row < self.outlineView.numberOfRows) {
        MWVariableDisplayItem *item = [self.outlineView itemAtRow:row];
        if (item.isGroup) {
            if ([self.outlineView isItemExpanded:item]) {
                [self.outlineView collapseItem:item];
            } else {
                [self.outlineView expandItem:item];
            }
        } else {
            NSTextField *textField = [self editableTextFieldAtRow:row];
            if (textField) {
                [self.window makeFirstResponder:textField];
            }
        }
    }
}


- (IBAction)searchTextChanged:(id)sender {
    searchText = self.searchField.stringValue;
    [self refreshDisplayedItems];
}


#pragma mark - NSOutlineViewDataSource methods


- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item {
    if (item) {
        return ((MWVariableDisplayItem *)item).numberOfChildren;
    }
    if (searchResultItems) {
        return searchResultItems.count;
    }
    return rootItems.count;
}


- (id)outlineView:(NSOutlineView *)outlineView
            child:(NSInteger)index
           ofItem:(id)item
{
    if (item) {
        return ((MWVariableDisplayItem *)item).children[index];
    }
    if (searchResultItems) {
        return searchResultItems[index];
    }
    return rootItems[index];
}


- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item {
    return ((MWVariableDisplayItem *)item).isGroup;
}


- (id)outlineView:(NSOutlineView *)outlineView
objectValueForTableColumn:(NSTableColumn *)tableColumn
           byItem:(id)_item
{
    MWVariableDisplayItem *item = _item;
    if (tableColumn == self.outlineView.tableColumns[NAME_COLUMN_INDEX]) {
        return item.name;
    }
    item.value = [self valueForVariable:item.name];
    return item;
}


#pragma mark - NSOutlineViewDelegate methods


- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item {
    return !(((MWVariableDisplayItem *)item).isGroup);
}


- (void)outlineViewItemDidExpand:(NSNotification *)notification {
    MWVariableDisplayItem *item = notification.userInfo[@"NSObject"];
    if (NSNotFound == [expandedItems indexOfObject:item.name]) {
        [expandedItems addObject:item.name];
        [[NSUserDefaults standardUserDefaults] setObject:expandedItems forKey:DEFAULTS_EXPANDED_ITEMS_KEY];
    }
}


- (void)outlineViewItemDidCollapse:(NSNotification *)notification {
    MWVariableDisplayItem *item = notification.userInfo[@"NSObject"];
    if (NSNotFound != [expandedItems indexOfObject:item.name]) {
        [expandedItems removeObject:item.name];
        [[NSUserDefaults standardUserDefaults] setObject:expandedItems forKey:DEFAULTS_EXPANDED_ITEMS_KEY];
    }
}


- (NSView *)outlineView:(NSOutlineView *)outlineView
     viewForTableColumn:(NSTableColumn *)tableColumn
                   item:(id)item
{
    return [outlineView makeViewWithIdentifier:tableColumn.identifier owner:self];
}


#pragma mark - NSTextFieldDelegate methods


- (BOOL)control:(NSControl *)control textShouldBeginEditing:(NSText *)fieldEditor {
    NSInteger row = [self.outlineView rowForView:control];
    MWVariableDisplayItem *item = [self.outlineView itemAtRow:row];
    return !(item.isGroup);
}


- (void)controlTextDidEndEditing:(NSNotification *)notification {
    NSTextField *textField = notification.object;
    int textMovement = [notification.userInfo[@"NSTextMovement"] intValue];
    
    if (textField == self.searchField) {
        
        if (textMovement == NSTabTextMovement) {
            textField.nextKeyView = [self nextEditableTextFieldStartingAtRow:0];
        } else if (textMovement == NSBacktabTextMovement) {
            [self previousEditableTextFieldStartingAtRow:self.outlineView.numberOfRows-1].nextKeyView = textField;
        }
    
    } else {
        
        NSInteger row = [self.outlineView rowForView:textField];
        if (-1 == row) {
            // The text field is not in the outline view, possibly because the outline view is currently
            // executing reloadData (and ending editing as a consequence).  Discard the edits.
            return;
        }
        
        MWVariableDisplayItem *item = [self.outlineView itemAtRow:row];
        if (!(item.isGroup)) {
            [self setValue:textField.stringValue forVariable:item.name];
        }
        
        if (textMovement == NSTabTextMovement) {
            textField.nextKeyView = [self nextEditableTextFieldStartingAtRow:row+1];
        } else if (textMovement == NSBacktabTextMovement) {
            [self previousEditableTextFieldStartingAtRow:row-1].nextKeyView = textField;
        }
        
    }
}


#pragma mark - MWClientPluginWorkspaceState methods


- (NSDictionary *)workspaceState {
    NSMutableDictionary *workspaceState = [NSMutableDictionary dictionary];
    
    NSArray *expandedGroups = [expandedItems copy];
    if (expandedGroups.count > 0) {
        workspaceState[@"expandedGroups"] = expandedGroups;
    }
    
    return workspaceState;
}


- (void)setWorkspaceState:(NSDictionary *)workspaceState {
    NSArray *expandedGroups = workspaceState[@"expandedGroups"];
    if (expandedGroups && [expandedGroups isKindOfClass:[NSArray class]]) {
        [expandedItems removeAllObjects];
        [expandedItems addObjectsFromArray:expandedGroups];
        [[NSUserDefaults standardUserDefaults] setObject:expandedItems forKey:DEFAULTS_EXPANDED_ITEMS_KEY];
        
        for (MWVariableDisplayItem *item in rootItems) {
            if (NSNotFound != [expandedItems indexOfObject:item.name]) {
                [self.outlineView expandItem:item];
            } else {
                [self.outlineView collapseItem:item];
            }
        }
    }
}


#pragma mark - Private methods


- (void)updateVariableValues:(NSTimer *)timer {
    [self.outlineView enumerateAvailableRowViewsUsingBlock:^(__kindof NSTableRowView *rowView, NSInteger row) {
        MWVariableDisplayItem *item = [self.outlineView itemAtRow:row];
        if (!item.isGroup) {
            item.value = [self valueForVariable:item.name];
        }
    }];
}


- (void)updateRootItems:(MWCocoaEvent *)event {
    NSMutableDictionary *oldRootObjects = [NSMutableDictionary dictionaryWithCapacity:rootItems.count];
    for (MWVariableDisplayItem *item in rootItems) {
        oldRootObjects[item.name] = item;
    }
    
    [rootItems removeAllObjects];
    allVariablesItems = nil;
    
    NSDictionary *groups = [self.client varGroups];
    for (NSString *groupName in groups) {
        MWVariableDisplayItem *item = oldRootObjects[groupName];
        if (!item) {
            item = [[MWVariableDisplayItem alloc] initWithName:groupName];
        }
        
        NSArray<NSString *> *varNames = groups[groupName];
        NSMutableArray<MWVariableDisplayItem *> *children = [NSMutableArray arrayWithCapacity:varNames.count];
        for (NSString *varName in varNames) {
            [children addObject:[[MWVariableDisplayItem alloc] initWithName:varName]];
        }
        item.children = children;
        if ([groupName isEqualToString:@(ALL_VARIABLES)]) {
            allVariablesItems = children;
        }
        
        [rootItems addObject:item];
    }
    
    [self refreshDisplayedItems];
}


- (void)refreshDisplayedItems {
    if (searchText.length == 0 || !allVariablesItems) {
        searchResultItems = nil;
    } else {
        NSIndexSet *matchingIndexes = [allVariablesItems indexesOfObjectsPassingTest:^BOOL(MWVariableDisplayItem *item, NSUInteger index, BOOL *stop) {
            return ([item.name rangeOfString:searchText options:NSCaseInsensitiveSearch].location != NSNotFound);
        }];
        searchResultItems = [allVariablesItems objectsAtIndexes:matchingIndexes];
    }
    
    [self.outlineView reloadData];
    
    for (MWVariableDisplayItem *item in rootItems) {
        if (NSNotFound != [expandedItems indexOfObject:item.name]) {
            [self.outlineView expandItem:item];
        }
    }
}


- (NSString *)valueForVariable:(NSString *)name {
    mw::Datum value = [variables valueForVariable:name];
    return @(value.toString(true).c_str());
}


- (void)setValue:(NSString *)value forVariable:(NSString *)name {
    const char *valueUTF8 = value.UTF8String;
    mw::Datum val;
    try {
        val = mw::ParsedExpressionVariable::evaluateExpression(valueUTF8);
    } catch (const mw::SimpleException &) {
        val.setString(valueUTF8);
    }
    if ([variables valueForVariable:name] != val) {
        [variables setValue:val forVariable:name];
    }
}


- (NSTextField *)nextEditableTextFieldStartingAtRow:(NSInteger)row {
    while (row < self.outlineView.numberOfRows) {
        NSTextField *textField = [self editableTextFieldAtRow:row];
        if (textField) {
            return textField;
        }
        row++;
    }
    return self.searchField;
}


- (NSTextField *)previousEditableTextFieldStartingAtRow:(NSInteger)row {
    while (row >= 0) {
        NSTextField *textField = [self editableTextFieldAtRow:row];
        if (textField) {
            return textField;
        }
        row--;
    }
    return self.searchField;
}


- (NSTextField *)editableTextFieldAtRow:(NSInteger)row {
    MWVariableDisplayItem *item = [self.outlineView itemAtRow:row];
    if (!item.isGroup) {
        [self.outlineView selectRowIndexes:[NSIndexSet indexSetWithIndex:row] byExtendingSelection:NO];
        [self.outlineView scrollRowToVisible:row];
        NSTableCellView *rowView = [self.outlineView viewAtColumn:VALUE_COLUMN_INDEX row:row makeIfNecessary:YES];
        return rowView.textField;
    }
    return nil;
}


@end


























