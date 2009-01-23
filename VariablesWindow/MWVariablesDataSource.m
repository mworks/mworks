//
//  MWVariablesDataSource.m
//  MonkeyWorksVariablesWindow
//
//  Created by Ben Kennedy on 8/28/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWVariablesDataSource.h"
#import "MWVariableDisplayItem.h"
#import "MWVariablesWindowController.h"

@implementation MWVariablesDataSource
- (void)awakeFromNib {
	rootGroups = nil;
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


- (void)addRootGroups:(NSDictionary *)groups {
	rootGroups = groups;
	[rootItems removeAllObjects];
	for(int index = 0; index < [rootGroups count]; index++){
		NSString *key = [[rootGroups allKeys] objectAtIndex:index];
		MWVariableDisplayItem *item = [[MWVariableDisplayItem alloc] initWithGroupName:key andVariables:[rootGroups objectForKey:key]];
		[rootItems insertObject:item atIndex:index];
	}
}



// DataSource overridden methods
- (int)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item
{
	if(item == nil) {
		int numGroups = [[rootGroups allKeys] count];
		return numGroups;
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
			child:(int)index
		   ofItem:(id)item
{
	if(item == nil) {
		//NSString *key = [[rootGroups allKeys] objectAtIndex:index];
		//MWVariableDisplayItem *item = [[MWVariableDisplayItem alloc] initWithGroupName:key andVariables:[rootGroups objectForKey:key]];
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
	 setObjectValue:(id)object 
	 forTableColumn:(NSTableColumn *)tableColumn 
			 byItem:(id)item {
	if(delegate != nil) {
		if(tableColumn == valueCol) {
			mw::Data setval;
			NSScanner *scanner = [[NSScanner alloc] initWithString:object];
			
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