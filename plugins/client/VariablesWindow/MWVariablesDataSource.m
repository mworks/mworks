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

@implementation MWVariablesDataSource
- (void)awakeFromNib {
	rootItems = [[NSMutableArray alloc] init];
}


- (id)delegate {
	return delegate;
}

- (void)setDelegate:(id)new_delegate {
	if(![new_delegate respondsToSelector:@selector(getValueString:)]) {
		[NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWVariablesDataSource"];		
	}
	
	delegate = new_delegate;
}


- (void)setRootGroups:(NSDictionary *)rootGroups {
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
}



// DataSource overridden methods
- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
	if(item == nil) {
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
    NSString *object = (NSString *)_object;
    
	if(delegate != nil) {
		if(tableColumn == valueCol) {
			mw::Datum setval;
			NSScanner *scanner = [NSScanner scannerWithString:object];
			
			double possibleDoubleValue;
			
			if([scanner scanDouble:&possibleDoubleValue]) {
				if([scanner isAtEnd]) {
					setval.setFloat(possibleDoubleValue);
				} else {
					setval.setString([object cStringUsingEncoding:NSASCIIStringEncoding]);				
				}
			} else {
			// this is kind of shitty, but it will keep a user from shooting themselves right now
				if([object compare:@"DICT"] == NSOrderedSame && [delegate isTagDictionary:[item displayName]] ) {
					return;
				} else if([object compare:@"LIST"] == NSOrderedSame && [delegate isTagList:[item displayName]]) {
					return;
				}
				setval.setString([object cStringUsingEncoding:NSASCIIStringEncoding]);
			}
			
			[delegate set:[item displayName] toValue:&setval];
		}
	}
}

@end