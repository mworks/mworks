//
//  MWVariableList.m
//  MWorksMATLABWindow
//
//  Created by Ben Kennedy on 9/22/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWVariableList.h"
#import "MWMATLABWindowController.h"


@implementation MWVariableList


- (id) init {
	self = [super init];
	if (self != nil) {
		current_vars = [[NSMutableArray alloc] init];
		syncEventIndex = -1;
	}
	return self;
}


- (id)delegate {
	return delegate;
}

- (void)setDelegate:(id)new_delegate {
	if(![new_delegate respondsToSelector:@selector(updateVariableFilter)]) {
		[NSException raise:NSInternalInconsistencyException
					format:@"Delegate doesn't respond to required methods for MWVariableList"];		
	}
	
	delegate = new_delegate;
}


- (void)addVariable:(MWVarEntry *)var {
	[current_vars addObject:var];
}

- (void)reloadData {
	[variablesView reloadData];
}

- (void)clear {
	[current_vars removeAllObjects];
}

- (void)setSyncEventName:(NSString *)syncEventName {
	syncEventIndex = 0;
	
	NSEnumerator *enumerator = [current_vars objectEnumerator];
	MWVarEntry *mve;
	
	while(mve = [enumerator nextObject]) {
		if([syncEventName isEqualToString:[mve name]]) {
			[mve setSelected:YES];
			break;
		}
		++syncEventIndex;
	}	
}

- (NSArray *)current_vars {
	NSArray *return_variables = [NSArray arrayWithArray:current_vars];
	return return_variables;
}

- (NSArray *)currentSelectedVariables {
	NSMutableArray *current_selected_variables = [NSMutableArray array];
	
	for (MWVarEntry *mve in current_vars) {
		if([[mve selected] boolValue]) {
			[current_selected_variables addObject:[mve name]];
		}
	}
	
	return current_selected_variables;
}


// delegate methods
- (int)numberOfRowsInTableView:(NSTableView *)aTableView
{
    return [current_vars count];
}

- (id)tableView:(NSTableView *)aTableView
      objectValueForTableColumn:(NSTableColumn *)aTableColumn
			row:(int)rowIndex
{
	if(aTableColumn == nameCol) {
		return [[current_vars objectAtIndex:rowIndex] name];
	} else if(aTableColumn == selectedCol) {
		if(rowIndex == syncEventIndex) {
			return [NSNumber numberWithInt:NSControlStateValueOn];
		} else {
			if([[[current_vars objectAtIndex:rowIndex] selected] boolValue]) {
				return [NSNumber numberWithInt:NSControlStateValueOn];
			} else {
				return [NSNumber numberWithInt:NSControlStateValueOff];
			}
		}
	}
	return nil;
}

- (void)tableView:(NSTableView *)aTableView 
   setObjectValue:(id)anObject 
   forTableColumn:(NSTableColumn *)aTableColumn 
			  row:(int)rowIndex {
	if(aTableColumn == selectedCol) {
		[[current_vars objectAtIndex:rowIndex] setSelected:[anObject boolValue]];
		if(delegate != nil) {
			[delegate updateVariableFilter];
		}
	}
}

- (BOOL)tableView:(NSTableView *)aTableView 
shouldEditTableColumn:(NSTableColumn *)aTableColumn 
			  row:(int)rowIndex {
	if(aTableColumn == selectedCol) {
		return rowIndex != syncEventIndex;
	} else {
		return NO;
	}
}

@end
