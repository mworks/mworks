//
//  MWErrorViewController.m
//  NewEditor
//
//  Created by David Cox on 11/13/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWErrorArrayController.h"
#import "ImageAndTextCell.h"

@implementation MWErrorArrayController


- (void)awakeFromNib {

	NSTableColumn* column = [error_table_view tableColumnWithIdentifier:@"message"];
	
	ImageAndTextCell *cell = [[ImageAndTextCell alloc] init];
	
	[column setDataCell: cell];			
	[error_table_view reloadData];

}

- (void)tableView:(NSTableView *)aTableView 
				willDisplayCell:(id)cell 
				forTableColumn:(NSTableColumn *)aTableColumn 
				row:(NSInteger)rowIndex{


	if([[aTableColumn identifier] isEqualToString:@"message"]){
		NSImage * img;
		
		NSString *fullpath = [[NSBundle mainBundle] pathForResource:@"errorGlyphSmaller" ofType:@"tif"];
		img = [[NSImage alloc] initWithContentsOfFile:fullpath];
			
		[cell setEditable:NO];
		[(ImageAndTextCell *)cell setAccessoryImage:img];
	}
	

}

- (BOOL)tableView:(NSTableView *)aTableView shouldSelectTableColumn:(NSTableColumn *)aTableColumn{
	return NO;
}

- (BOOL)tableView:(NSTableView *)aTableView shouldEditTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex{
	return NO;
}

- (BOOL)tableView:(NSTableView *)aTableView shouldSelectRow:(NSInteger)rowIndex{
	return NO;
}

- (BOOL)tableView:(NSTableView *)tableView shouldShowCellExpansionForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row{
	return NO;
}

- (BOOL)tableView:(NSTableView *)tableView shouldTrackCell:(NSCell *)cell forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row{
	return YES;
}

- (IBAction)hideErrorPanel:(id)sender{
	[errorPanel orderOut:sender];
}

- (IBAction)showErrorPanel:(id)sender{
	[errorPanel orderFront:sender];
}

- (IBAction)toggleErrorPanel:(id)sender{
	if([errorPanel isVisible]){
		[self hideErrorPanel:sender];
	} else {
		[self showErrorPanel:sender];
	}
}

- (IBAction)triggerGoToError:(id)sender{
	int error_index = [sender clickedRow];
	
	NSError *outError;
	
	NSArray *content = [self content];
	NSLog(@"number of errors: %d", [content count]);
	
	NSXMLElement *error_node = [content objectAtIndex:error_index];
	NSString *node_id = [error_node valueForKey:@"id"];
	
	NSString *xpath = [NSString stringWithFormat:@"//*[@_id = '%@']", node_id]; 
		
	NSArray *matching_nodes = [[doc document] nodesForXPath:xpath error:&outError];
	
	
	NSXMLElement *matching_node = [matching_nodes objectAtIndex:0];
	
	NSLog(@"matching node = %@", [matching_node XMLString]);
	
	//NSIndexPath *pathToMatch = [experiment_tree_controller arrangedIndexPathForObject:matching_node];
//	
//	NSLog(@"indexPath length = %d", [pathToMatch length]);
//	int i;
//	for(i = 0; i < [pathToMatch length]; i++){
//		NSLog(@"path [%d] = %d", i, [pathToMatch indexAtPosition:i]);
//	}
	
	
	// This is a dark-magic opaque interface call at its root
	// If an OS update breaks this function, this is the reason.
	NSTreeNode *root = [experiment_tree_controller arrangedRoot];
	NSIndexPath *match_path = [self findIndexPathForNodeID:node_id inTreeNode:root];
	
	[experiment_tree_controller setSelectionIndexPath:match_path];
	[experiment_tree_controller reloadData:self];
	[experiment_tree_controller takeFocus];

}

- (NSIndexPath *)findIndexPathForNodeID:(NSString *)node_id inTreeNode:(NSTreeNode *)tree_node{

	if([[[tree_node representedObject] valueForKey:@"_id"] isEqualToString:node_id]){
		return [tree_node indexPath];
	}
	
	NSArray *child_nodes = [tree_node childNodes];
	NSEnumerator *child_node_enumerator = [child_nodes objectEnumerator];
	NSTreeNode *child_node;
	while(child_node = [child_node_enumerator nextObject]){
		NSIndexPath *path = [self findIndexPathForNodeID:node_id inTreeNode:child_node];
		
		if(path != Nil){
			return path;
		}
	}


	return Nil;
}


@end
