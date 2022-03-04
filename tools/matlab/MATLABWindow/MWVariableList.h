//
//  MWVariableList.h
//  MWorksMATLABWindow
//
//  Created by Ben Kennedy on 9/22/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MWVarEntry.h"


@interface MWVariableList : NSObject {
	NSMutableArray *current_vars;
	IBOutlet NSTableView *variablesView;
	IBOutlet NSTableColumn *nameCol;
	IBOutlet NSTableColumn *selectedCol;
	IBOutlet id delegate;
	int syncEventIndex;
}

- (id) init;
- (id)delegate;
- (void)setDelegate:(id)new_delegate;
- (void)addVariable:(MWVarEntry *)var;
- (void)reloadData;
- (void)clear;
- (void)setSyncEventName:(NSString *)syncEventName;
- (NSArray *)current_vars;
- (NSArray *)currentSelectedVariables;
@end


@interface MWVariableList(DelegateMethods)
- (int)numberOfRowsInTableView:(NSTableView *)tableView;
- (id)tableView:(NSTableView *)tableView
      objectValueForTableColumn:(NSTableColumn *)tableColumn
			row:(int)row;
- (void)tableView:(NSTableView *)aTableView 
   setObjectValue:(id)anObject 
   forTableColumn:(NSTableColumn *)aTableColumn 
			  row:(int)rowIndex;
- (BOOL)tableView:(NSTableView *)aTableView 
shouldEditTableColumn:(NSTableColumn *)aTableColumn 
			  row:(int)rowIndex;

@end
