//
//  MWExperimentTreeController.m
//  NewEditor
//
//  Created by David Cox on 11/1/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWExperimentTreeController.h"
#import "TwoLineImageTextCell.h"
#import "ImageAndTextCell.h"
#import "MWXMLElementAdditions.h"
#import "NSTreeController_Extensions.h"
#import "MWAutocompletingTextField.h"

NSString *MWExperimentTreeNodeType = @"MEExperimentTreeNodeType";


@implementation MWExperimentTreeController

- (NSArray *) allowedTypes {
	return [NSArray arrayWithObjects:MWExperimentTreeNodeType,
									 NSStringPboardType, 
                                     NSFilenamesPboardType,
                                     nil];
}


- (void)awakeFromNib {

	[experiment_tree registerForDraggedTypes: [self allowedTypes]];
	[experiment_tree setDraggingSourceOperationMask:NSDragOperationEvery forLocal:YES];
    [experiment_tree setDraggingSourceOperationMask:NSDragOperationEvery forLocal:NO];
    [experiment_tree setAutoresizesOutlineColumn:NO];
	
	[experiment_tree setTarget:self];
	[experiment_tree setDoubleAction:@selector(expandSelected:)];
	
	NSTableColumn* column = [[experiment_tree tableColumns] objectAtIndex:0];
	
	ImageAndTextCell *cell = [[ImageAndTextCell alloc] init];
	[cell setMenu:context_menu];
	[cell setDelegate:self];
	
	[column setDataCell: cell];			
	[experiment_tree reloadData];
    
    [[NSNotificationCenter defaultCenter] addObserver:experiment_tree 
                                          selector:@selector(reloadData)
                                          name: MW_DOCUMENT_ERRORS_NOTE
                                          object: Nil];

}

//-(void)observeElement:(NSXMLElement *)element {
//    [element addObserver:self forKeyPath:@"object" 
//                              options:(NSKeyValueObservingOptionNew |
//                                       NSKeyValueObservingOptionOld) 
//                              context:Nil];
//}
//
//-(void)recursivelyObserveChildren:(NSTreeNode *)node {
//    
//    NSXMLElement *element = [node representedObject];
//    [self observeElement:element];
//    
//    NSArray *children = [node childNodes];
//    NSEnumerator *child_enumerator = [children objectEnumerator];
//    NSTreeNode *obj;
//    while((obj = (NSTreeNode *)[child_enumerator nextObject]) != Nil){
//        [self recursivelyObserveChildren:obj];
//    }
//}
//-(void) observeValueForKeyPath:(NSString *)keyPath 
//                      ofObject:(id)object
//                      change:(NSDictionary *)change
//                      context:(void *)context{
//
//    NSXMLElement *new_object = (NSXMLElement *)[change valueForKey:NSKeyValueChangeNewKey];
//    NSArray *old_attributes = [(NSXMLElement *)[change valueForKey:NSKeyValueChangeOldKey] attributes];
//    //NSArray *new_attributes = [(NSXMLElement *)[change valueForKey:NSKeyValueChangeNewKey] attributes];
//    
//    [[[document undoManager] prepareWithInvocationTarget:new_object]
//                                         setAttributes:old_attributes];
//}

- (IBAction)updateInspectorView:(id)sender {

	[inspector_view_controller update:sender];
}

- (IBAction)expandSelected:(id)sender{

	id item = [experiment_tree itemAtRow:[experiment_tree selectedRow]];
	
	NSLog(@"%@", [[item representedObject] valueForKey:@"_display_as_group"]);
	if([[item representedObject] valueForKey:@"_display_as_group"] != Nil){
		if([experiment_tree isItemExpanded:item]){
			[experiment_tree collapseItem:item];
		} else {
			[experiment_tree expandItem:item];
		}
	} else {
		NSLog(@"trying");
		[experiment_tree editColumn:0 row:[experiment_tree selectedRow]
						 withEvent:Nil select:YES];
	}
}

- (IBAction)reloadData:(id)sender{
	[experiment_tree reloadData];
}

- (void)takeFocus{
	
	[experiment_tree becomeFirstResponder];
	/*[experiment_tree selectRowIndexes:[experiment_tree selectedRowIndexes] 
					 byExtendingSelection:NO];*/
}


#pragma mark -
#pragma mark NSTreeController Overrides for Automatic Undo/Redo


- (void)remove:(id)sender{
	
	NSIndexPath *indexPath = [self selectionIndexPath];
	id obj = [self objectAtArrangedIndexPath:indexPath];
	[obj detach];
	
	[[[document undoManager] prepareWithInvocationTarget:self]
					insertObject:obj atArrangedObjectIndexPath:indexPath];
	
	[super remove:sender];
	
	[experiment_tree reloadData];
}


- (void)moveNode:(NSTreeNode *)node toIndexPath:(NSIndexPath *)destination{
	
	NSIndexPath *old_path = [node indexPath];
	//NSIndexPath *return_path = [destination copyWithZone:Nil];
	int old_index = [old_path indexAtPosition:[old_path length]-1];
	int new_target = [destination indexAtPosition:[destination length]-1];
	
	if([old_path isSiblingOfIndexPath:destination]){
			
		if(old_index > new_target){
			old_index++;
			old_path = [ [old_path indexPathByRemovingLastIndex] indexPathByAddingIndex:old_index];
		} else {
			new_target--;
			//new_index --;
			//return_path = [ [destination indexPathByRemovingLastIndex] indexPathByAddingIndex:new_index];
		}
	}
	
	id treeRoot = [self arrangedObjects];
	
	NSIndexPath *path = [node indexPath];
	
	NSXMLElement *item = [node representedObject];
	
	NSXMLElement *original_parent = [[treeRoot descendantNodeAtIndexPath:[path indexPathByRemovingLastIndex]] representedObject];
	NSXMLElement *new_parent = [[treeRoot descendantNodeAtIndexPath:[destination indexPathByRemovingLastIndex]] representedObject];
	
	[original_parent willChangeValueForKey:@"childrenExt"];
	[original_parent willChangeValueForKey:@"prettyXMLString"];
	[original_parent removeChildAtIndex:[path indexAtPosition:[path length] - 1]];
	[original_parent didChangeValueForKey:@"prettyXMLString"];
	[original_parent didChangeValueForKey:@"childrenExt"];
	
	[item detach];
	
	[new_parent willChangeValueForKey:@"childrenExt"];
	[new_parent willChangeValueForKey:@"prettyXMLString"];
	int index_target = new_target;
	//int index_target = [destination indexAtPosition:[destination length] - 1];
	if(index_target < 0){
		index_target = 0;
	}
	if(index_target >= [new_parent childCount]){
		index_target = [new_parent childCount];
	}
	[new_parent insertChild:item atIndex:index_target];
	[new_parent didChangeValueForKey:@"prettyXMLString"];
	[new_parent didChangeValueForKey:@"childrenExt"];	

	
	NSIndexPath *path_to_item = [self arrangedIndexPathForObject:[new_parent childAtIndex:index_target]];
	NSTreeNode *new_node = [treeRoot descendantNodeAtIndexPath:path_to_item];
	
    // The operation that we've just done might have disrupted the old index path to the parent, so
    // we'll need to rebuild it
    NSIndexPath *new_old_path = [self arrangedIndexPathForObject:original_parent];
    new_old_path = [new_old_path indexPathByAddingIndex:old_index];
    
	[[[document undoManager] prepareWithInvocationTarget:self]
					moveNode:new_node toIndexPath:new_old_path];

	//[super moveNode:node toIndexPath:indexPath];

	[experiment_tree reloadData];
	[self setSelectionIndexPath:destination];
}


- (void)removeObjectAtArrangedObjectIndexPath:(NSIndexPath *)indexPath{

	id obj = [self objectAtArrangedIndexPath:indexPath];
	[obj detach];
	
	[super removeObjectAtArrangedObjectIndexPath:indexPath];
    
	[experiment_tree reloadData];
    [[[document undoManager] prepareWithInvocationTarget:self]
     insertObject:obj atArrangedObjectIndexPath:indexPath];
}


- (void)insertObject:(id)obj atArrangedObjectIndexPath:(NSIndexPath *)indexPath{

	id treeRoot = [self arrangedObjects];
	
	NSXMLElement *new_parent = [[treeRoot descendantNodeAtIndexPath:[indexPath indexPathByRemovingLastIndex]] representedObject];
	
	[new_parent willChangeValueForKey:@"childrenExt"];
	[new_parent willChangeValueForKey:@"prettyXMLString"];
	[new_parent insertChild:obj atIndex:[indexPath indexAtPosition:[indexPath length]-1]];
	[new_parent didChangeValueForKey:@"childrenExt"];
	[new_parent didChangeValueForKey:@"prettyXMLString"];
	//[super insertObject:obj atArrangedObjectIndexPath:indexPath];
	
	[self setSelectionIndexPath:indexPath];
	
    [[[document undoManager] prepareWithInvocationTarget:self]
     removeObjectAtArrangedObjectIndexPath:[[self arrangedIndexPathForObject:new_parent] indexPathByAddingIndex:[indexPath indexAtPosition:[indexPath length]-1]]];
}


#pragma mark -
#pragma mark Copy/Paste/Drag/Drop Under-the-hood Methods

- (BOOL)writeNodes:(NSArray *)nodes toPasteboard:(NSPasteboard *)pboard
									asTypes:(NSArray *)types{
	
	// These are the types that we will be putting into the pasteboard
	[pboard declareTypes:types owner:self];
	
	// For now, just move the first item

	if([types containsObject:NSStringPboardType]){
		
        NSMutableString *items_string = [[NSMutableString alloc] init];
        NSXMLElement *item;
        
        NSEnumerator *node_enumerator = [nodes objectEnumerator];
        while(item = [[node_enumerator nextObject] representedObject]){
            [items_string appendString:[item XMLStringWithOptions:NSXMLNodePrettyPrint]];
            [items_string appendString:@"\n"];
        }
		[pboard setString:items_string forType:NSStringPboardType];
	}
	if([types containsObject:MWExperimentTreeNodeType]){
		NSIndexPath *pathToDraggedNode = [[nodes objectAtIndex:0] indexPath];
		NSData *indexPathData = [NSKeyedArchiver archivedDataWithRootObject:pathToDraggedNode];
		[pboard setData:indexPathData forType:MWExperimentTreeNodeType];
		dragged_nodes = nodes;
		//[pboard setData:Nil forType:MWExperimentTreeNodeType];
	}
	
	return YES;
									
}

- (NSIndexPath *)validatePasteFromPasteboard:(NSPasteboard *)pboard
			 toDestinationIndexPath:(NSIndexPath *)inputIndexPath
			 allowPasteInto:(BOOL)allowInto{ 
	
	//return inputIndexPath;
	
	NSIndexPath *indexPath = inputIndexPath;
	if(allowInto){
		indexPath = [self validatePasteFromPasteboard:pboard
							  toDestinationIndexPath:[indexPath indexPathByAddingIndex:0]
							  allowPasteInto:NO];
		if(indexPath != Nil){
			return indexPath;
		}
		
		indexPath = inputIndexPath;
		indexPath = [[indexPath indexPathByRemovingLastIndex] indexPathByAddingIndex:[indexPath lastIndex]+1];
	}
	
	NSXMLElement *target_element = [self objectAtArrangedIndexPath:
									[indexPath indexPathByRemovingLastIndex]];
	int index = [indexPath indexAtPosition:[indexPath length] - 1];
	
	NSString * type = [pboard availableTypeFromArray:[self allowedTypes]];
	
	
	// Rule #1 (easy):
	// Don't drop a top-level group anywhere within the tree (ext app is fine)
	if([type isEqualToString:MWExperimentTreeNodeType] && dragged_nodes != Nil){
		NSEnumerator *enumerator = [dragged_nodes objectEnumerator];
		id el;
		while(el = [enumerator nextObject]){
			if([[el representedObject] attributeForName:@"_display_as_group"] != Nil){
				return Nil;
			}
		}
	}
	
        
	// A little bit of setup for the subsequent rules
	NSXMLElement *dragged_node;
	//NSError *outError;
		
    NSMutableArray *nodes_to_consider = [[NSMutableArray alloc] init];
    
	if([type isEqualToString:NSStringPboardType]){
		[nodes_to_consider addObjectsFromArray:[self convertStringToXMLElements:[pboard stringForType:type]]];
        
    } else if([type isEqualToString:NSFilenamesPboardType]){
        NSArray *files = [pboard propertyListForType:type];
        [nodes_to_consider addObjectsFromArray:[self smartConvertFilenamesToXMLElements:files]];
        
	} else {
        int n;
        for(n=0; n < [dragged_nodes count]; n++){
            [nodes_to_consider addObject:[[dragged_nodes objectAtIndex:n] representedObject]];
        }
	}
			
	// Get the library to do the fancier validation
	NSXMLDocument *lib = [document library];
    
    int i;
    BOOL isokay;
    for(i = 0; i < [nodes_to_consider count]; i++){
        dragged_node = [nodes_to_consider objectAtIndex:i];
        NSString *node_key = [[dragged_node attributeForName:@"_element_key"] stringValue];
        isokay = [self validatePasteOfNodeWithKey:node_key library:lib intoTarget:target_element atIndex:index];
        if(!isokay){
            return Nil;
        }
    }
    
    return indexPath;
}


- (BOOL)validatePasteOfNodeWithKey:(NSString *)node_key 
                    library:(NSXMLDocument *)lib 
                    intoTarget:(NSXMLElement *)target_element 
                    atIndex:(int)index{    
    
    NSError *outError;
    
    NSString *xpath = [NSString stringWithFormat:@"/MWElementLibrary/MWElementGroup[@name = 'All']/MWElement[@name = '%@']/allowed_parent", node_key];
	NSArray *allowed_parent_nodes = [lib nodesForXPath:xpath error:&outError];
	
	// Rule #2:
	// Obey <allowed_parent> tags from the library
	NSString *target_parent_element_key = [[target_element attributeForName:@"_element_key"] stringValue];
	
	NSEnumerator *parent_enumerator = [allowed_parent_nodes objectEnumerator];
	id node;
	BOOL okay = NO;
	while(node =  [parent_enumerator nextObject]){
		NSXMLNode *allowed_parent_node = [node childAtIndex:0];
		NSString *allowed_parent = [allowed_parent_node stringValue];
		//NSLog(@"allow %@", allowed_parent);
		if([allowed_parent isEqualToString:target_parent_element_key]){
			okay = YES;
			break;
		}
	}
	
	if(!okay){
		return NO;
	}
	
	
	NSArray *siblings = [target_element children];
		
	if([siblings count]){
	
		// Rule #3:
		// Obey <disallow_sibling_before> tags from the library
		xpath = [NSString stringWithFormat:@"/MWElementLibrary/MWElementGroup[@name = 'All']/MWElement[@name = '%@']/disallowed_sibling_before", node_key];
		NSArray *disallowed_sibling_before_nodes = [lib nodesForXPath:xpath error:&outError];
		
		// check disallowed siblings (before)
		if([disallowed_sibling_before_nodes count]){
		
			int i;
			for(i =0; i < index; i++){
				NSXMLElement *sib = [siblings objectAtIndex:i];
				NSString *sib_element_key = [[sib attributeForName:@"_element_key"] stringValue];
				NSEnumerator *sib_enumerator = [disallowed_sibling_before_nodes objectEnumerator];
				NSXMLElement *disallowed_sib_element;
				while(disallowed_sib_element = [sib_enumerator nextObject]){
					NSString *disallowed_string = [[disallowed_sib_element childAtIndex:0] stringValue];
					
					
					if([sib_element_key isEqualToString:disallowed_string]){
						return NO;
					}
				}
			}
		}
	
	
		// Rule #4:
		// Obey <disallow_sibling_after> tags from the library
		xpath = [NSString stringWithFormat:@"/MWElementLibrary/MWElementGroup[@name = 'All']/MWElement[@name = '%@']/disallowed_sibling_after", node_key];
		NSArray *disallowed_sibling_after_nodes = [lib nodesForXPath:xpath error:&outError];

		// check disallowed siblings (before)
		if([disallowed_sibling_after_nodes count]){
		
			int i;
			for(i =index; i < [siblings count]; i++){
				NSXMLElement *sib = [siblings objectAtIndex:i];
				NSString *sib_element_key = [[sib attributeForName:@"_element_key"] stringValue];
				NSEnumerator *sib_enumerator = [disallowed_sibling_after_nodes objectEnumerator];
				NSXMLElement *disallowed_sib_element;
				while(disallowed_sib_element = [sib_enumerator nextObject]){
					NSString *disallowed_string = [[disallowed_sib_element childAtIndex:0] stringValue];
					
					
					if([sib_element_key isEqualToString:disallowed_string]){
						return NO;
					}
				}
			}
		}
	}

	return YES;
}


- (BOOL)pasteDataFromPasteboard:(NSPasteboard *)pboard 
		toDestinationIndexPath:(NSIndexPath *)destination{

	NSArray * accepted_types = [self allowedTypes];
												
    //NSError *error;
	
	// get pasteboard type and data
	NSString * type = [pboard availableTypeFromArray:accepted_types];

	// An internal drag-and-drop
	if([type isEqualToString:MWExperimentTreeNodeType]){
		
        int i;
        for(i = [dragged_nodes count]-1; i>=0; i--){
            [self moveNode:[dragged_nodes objectAtIndex:i] toIndexPath:destination];
		}
		// clip

		[self reloadData:self];
		[inspector_view_controller update:self];
		return YES;
		
	// XML code from somewhere external
	} else if([type isEqualToString:NSStringPboardType]){
		// create new XML element(s)
        NSArray *elements_to_paste = [self convertStringToXMLElements:[pboard stringForType:type]];        
        [self pasteInElements:elements_to_paste atDestination:destination];
        
		dragged_nodes = Nil;
		[self reloadData:self];
		[inspector_view_controller update:self];
		return YES;
	} else if([type isEqualToString:NSFilenamesPboardType]){
        NSArray *files = [pboard propertyListForType:NSFilenamesPboardType];
        NSArray *elements_to_paste = [self smartConvertFilenamesToXMLElements:files];
        [self pasteInElements:elements_to_paste atDestination:destination];
        return YES;
    }
	
	dragged_nodes = Nil;
	return NO;
}


- (void) pasteInElements:(NSArray *)elements_to_paste atDestination:destination {
    NSEnumerator *element_enumerator = [elements_to_paste reverseObjectEnumerator];
    
    NSXMLElement *new_element;
    while(new_element = [element_enumerator nextObject]){
        [new_element detach];
        
        // TODO: take the dragged node and fix any duplicate tags
        
        [new_element setValue:[document newInternalID] forKey:@"_id"];
        /*if(error != Nil){
            NSLog(@"An error occurred");
            dragged_nodes = Nil;
            return NO;
        }*/
        
        NSString *tag;
        NSXMLNode *tag_att;
        if((tag_att = [new_element attributeForName:@"tag"]) != Nil){
            tag = [tag_att stringValue];
            
            // check to see if another object in the hierarchy already has the same tag name
            BOOL okay = NO;
            while(!okay){
                NSError *error;
                NSString *duplicate_tag_xpath = [NSString stringWithFormat:@"//*[@tag=\"%@\"]", tag, Nil];
                NSArray *nodes = [[document document] nodesForXPath:duplicate_tag_xpath error:&error];
                
                if([nodes count]){
                    okay = NO;
                    // modify the name
                    // check if it already ends in a number
                    unichar lastchar = [tag characterAtIndex:[tag length]-1];
                    if([[NSCharacterSet decimalDigitCharacterSet] characterIsMember:lastchar]){
                        NSString *to_convert = [[NSString alloc] initWithCharacters:&lastchar length:1];
                        int intversion = [to_convert integerValue];
            
                        tag = [[tag substringToIndex:[tag length]-1] stringByAppendingFormat:@"%d", intversion+1];
                    } else {
                        tag = [tag stringByAppendingString:@" 2"];
                    }
                } else {
                    okay = YES;
                }
            }
            
            [tag_att setStringValue:tag];
        }
                
                
        // insert it into the tree using the NSTreeController methods
        [self insertObject:new_element atArrangedObjectIndexPath:destination];
    }
}

- (NSArray *)convertStringToXMLElements:(NSString *)raw_string{
    NSError *error;
    
    NSMutableString *xml_string = [[NSMutableString alloc] init];
    [xml_string appendString:@"<root>"];
    [xml_string appendString:raw_string];
    [xml_string appendString:@"</root>"];
    
    NSXMLDocument *new_doc = [[NSXMLDocument alloc] initWithXMLString:xml_string options:0 error:&error];
    
    NSXMLDocument *new_doc_hinted = [new_doc
						objectByApplyingXSLT:[[document displayHintTransform] XMLData]
													arguments:nil
													error:&error];
    
    NSXMLElement *root = [new_doc_hinted rootElement];
    [root detach];
    NSArray *elements_to_paste = [root children];
    
    return elements_to_paste;
}


- (NSArray *)smartConvertFilenamesToXMLElements:(NSArray *)files {
    //NSMutableArray *xml_elements = [[NSMutableArray alloc] init];
    NSMutableString *full_xml_string = [[NSMutableString alloc] init];
    
    NSEnumerator *file_enumerator = [files objectEnumerator];
    
    NSString *file;
    while(file = (NSString *)[file_enumerator nextObject]){
        NSString *extension = [file pathExtension];
        
        NSError *error;
        
        if([extension isEqualToString:@"tif"] ||
           [extension isEqualToString:@"tiff"] ||
           [extension isEqualToString:@"png"] ||
           [extension isEqualToString:@"jpg"] ||
           [extension isEqualToString:@"bmp"]){
            
            // TODO: get this from the library
            NSString *xml_equivalent = 
                [NSString stringWithFormat:@"<stimulus tag=\"%@\" type=\"image_file\" path=\"%@\" x_size=\"5.0\" y_size=\"5.0\" x_position=\"0.0\" y_position=\"0.0\" rotation=\"0.0\" alpha_multiplier=\"1.0\"/>",
                            [file lastPathComponent], file, nil];
            
            [full_xml_string appendString:xml_equivalent];    
        }
        
        if([extension isEqualToString:@"wav"]){
            
            // TODO: get this from the library
            NSString *xml_equivalent = 
                [NSString stringWithFormat:@"<sound tag=\"%@\" type=\"wav_file\" path=\"%@\"/>",
                            [file lastPathComponent], file, nil];
            
            [full_xml_string appendString:xml_equivalent];    
        }
    }

    return [self convertStringToXMLElements:full_xml_string];
}


/*- (IBAction)undo:(id)sender{
    [document undo:sender];
}

- (IBAction)redo:(id)sender{
    [document redo:sender];
}*/

#pragma mark -
#pragma mark Copy and Paste Actions (forwarded from the responder chain)



- (BOOL)validateMenuItem:(NSMenuItem *)item {

    if ([item action] == @selector(copy:)){
		return YES;
    }

    if ([item action] == @selector(cut:)){
		return YES;
    }
	
	if ([item action] == @selector(paste:)){
		return YES;
    }  

    return YES;

}

- (IBAction)copy:(id)sender{
	
	NSPasteboard *pboard = [NSPasteboard generalPasteboard];
	
	[self writeNodes:[self selectedNodes] toPasteboard:pboard 
							  asTypes:[NSArray arrayWithObjects:NSStringPboardType,Nil]];
}

- (IBAction)cut:(id)sender{

	[self copy:sender];

    NSArray *paths = [self selectionIndexPaths];
	//NSIndexPath *path = [self selectionIndexPath];

    if(paths != Nil){
        [self removeObjectsAtArrangedObjectIndexPaths:paths];
        [self setSelectionIndexPath:Nil];
    }
}


- (IBAction)paste:(id)sender {

	
	NSIndexPath *target_path = [self selectionIndexPath];
	
	target_path = [self validatePasteFromPasteboard:[NSPasteboard generalPasteboard]
			 toDestinationIndexPath:target_path
			 allowPasteInto:YES];
			 
	if(target_path != Nil){
		[self pasteDataFromPasteboard:[NSPasteboard generalPasteboard] 
			toDestinationIndexPath:target_path];
	
		[self setSelectionIndexPath:target_path];
		[self updateInspectorView:self];
	}
}



#pragma mark -
#pragma mark NSOutlineView Drag and Drop Methods

- (BOOL)outlineView:(NSOutlineView *)outlineView writeItems:(NSArray *)items 
										  toPasteboard:(NSPasteboard *)pboard {
    
	return [self writeNodes:items toPasteboard:pboard 
											asTypes:[self allowedTypes]];
}


- (NSDragOperation)outlineView:(NSOutlineView *)outlineView 
									validateDrop:(id <NSDraggingInfo>)info
									proposedItem:(id)item 
									proposedChildIndex:(int)index {
									
	
	NSIndexPath *indexPath = [[item indexPath] indexPathByAddingIndex:index];
	
	// get pasteboard type and data
	NSPasteboard * pboard = [info draggingPasteboard];
	
	if([self validatePasteFromPasteboard:pboard 
			 toDestinationIndexPath:indexPath
			 allowPasteInto:NO] != Nil){
		return NSDragOperationEvery;
	}
	
	return NSDragOperationNone;
	
}


- (BOOL)outlineView:(NSOutlineView *)outlineView 
						acceptDrop:(id <NSDraggingInfo>)info 
						item:(id)item childIndex:(int)index {
	
	
	
	NSPasteboard * pboard = [info draggingPasteboard];
    
	if(index < 0){
		index = 0;
	}
	
	NSIndexPath *destination = [[item indexPath] indexPathByAddingIndex:index];
	
	return [self pasteDataFromPasteboard:pboard 
				 toDestinationIndexPath:destination];
}
   
   

#pragma mark -
#pragma mark NSOutlineView Delegate Methods

- (void)outlineViewSelectionDidChange:(NSNotification *)aNotification
{
	[inspector_view_controller update:self];
}

// To get the "group row" look, we implement this method.
- (BOOL)outlineView:ov isGroupItem:(id)item {
    
	NSXMLElement *real_item = [item representedObject];
	
	if([real_item attributeForName:@"_display_as_group"]){
		return YES;
	}
	
	return NO;
}

- (BOOL)outlineView:(NSOutlineView *)ov shouldSelectItem:(id)item {
	NSXMLElement *real_item = [item representedObject];
	
	if([real_item attributeForName:@"_display_as_group"]){
		return NO;
	}
	
	if([real_item attributeForName:@"_unmoveable"]){
		return NO;
	}
	
	return YES;
}


- (void)outlineView:(NSOutlineView *)olv willDisplayCell:(NSCell *)cell 
                forTableColumn:(NSTableColumn *)tableColumn item:(id)item {
	// this one allows me to put an image and text into the outline view
    NSImage * img;
	
	NSXMLElement *element = [item representedObject];
	    
	NSString *name = [element valueForKey:@"_icon"];
	NSString *fullpath = [[NSBundle mainBundle] pathForResource:name ofType:@"tif"];
    img = [[NSImage alloc] initWithContentsOfFile:fullpath];
		
	[cell setEditable:YES];
	[(ImageAndTextCell *)cell setAccessoryImage:img];
	
    // XXX
	//[(ImageAndTextCell *)cell setStringValue:[element stringValue]];
    [(ImageAndTextCell *)cell setStringValue:[element displayName]];

	[(ImageAndTextCell *)cell setRepresented:element];
	[(ImageAndTextCell *)cell setNode:item];
	
	
	if([element attributeForName:@"_error"] != Nil){
		[(ImageAndTextCell *)cell setTextColor:[NSColor redColor]];
        [cell setFont:[[NSFontManager sharedFontManager] convertFont:[cell font] toHaveTrait:NSBoldFontMask]];
	} else if([element attributeForName:@"_unmoveable"] != Nil){
		[(ImageAndTextCell *)cell setTextColor:[NSColor lightGrayColor]];
        [cell setFont:[[NSFontManager sharedFontManager] convertFont:[cell font] toHaveTrait:NSBoldFontMask]];
	} else {
		[(ImageAndTextCell *)cell setTextColor:[NSColor blackColor]];
        [cell setFont:[[NSFontManager sharedFontManager] convertFont:[cell font] toNotHaveTrait:NSBoldFontMask]];
	}
	
	[(ImageAndTextCell *)cell setDrawsBackground:NO];
	//[(ImageAndTextCell *)cell setBackgroundColor:[NSColor clearColor]];
	
    if([element attributeForName:@"_unmoveable"]){
        [(ImageAndTextCell *)cell setAccessoryImage:Nil];
    }

}



- (BOOL)outlineView:(NSOutlineView *)outlineView shouldEditTableColumn:(NSTableColumn *)tableColumn item:(id)item{
	NSXMLElement *real_item = [item representedObject];
	
	if([real_item attributeForName:@"_display_as_group"]){
		return NO;
	}
	
	return YES;
}


#pragma mark -
#pragma mark NSOutlineView Dummy Delegate Methods

- (BOOL) outlineView: (NSOutlineView *)ov
         isItemExpandable: (id)item { return NO; }

- (int)  outlineView: (NSOutlineView *)ov
         numberOfChildrenOfItem:(id)item { return 0; }

- (id)   outlineView: (NSOutlineView *)ov
         child:(int)index
         ofItem:(id)item { return nil; }

- (id)   outlineView: (NSOutlineView *)ov
         objectValueForTableColumn:(NSTableColumn*)col
         byItem:(id)item {
	return nil;
}


#pragma mark -
#pragma mark Completion methods

- (NSArray *)completionsForXPath:(NSString *)xPath {
			 
	NSMutableArray *return_array = [[NSMutableArray alloc] init];
	
	NSError *outError;

	NSLog(@"xPath = %@", xPath);
	NSArray *nodes = [[document document] nodesForXPath:xPath error:&outError];
	
	if(outError != Nil){
		NSLog(@"There was an error");
	}
	
	if([nodes count] == 0){
		NSLog(@"no matches");
	} else {
		NSLog(@"%d matches", [nodes count]);
	}
	
	int i;
	NSXMLNode *node;
	for(i = 0; i < [nodes count]; i++){
		node = [nodes objectAtIndex:i];
		[return_array addObject:[node stringValue]];
	}
	
	return return_array;		 
}





#pragma mark -
#pragma mark Combobox Datasource Methods

- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)control
{
	NSArray *completions;
	NSString *xPath;
	
	if([control respondsToSelector:@selector(xPathForPartialTag:)]){
		xPath = [(MWAutocompletingTextField *)control xPathForPartialTag:[control stringValue]];
	} else {
		xPath = @"//@tag";
	}
	
	completions = [self completionsForXPath:xPath];
	
	return [completions count];
}

- (id)comboBox:(NSComboBox *)control objectValueForItemAtIndex:(NSInteger)index
{

	NSArray *completions;
	NSString *xPath;
	
	if([control respondsToSelector:@selector(xPathForPartialTag:)]){
		xPath = [(MWAutocompletingTextField *)control xPathForPartialTag:@""];
	} else {
		xPath = @"//@tag";
	}
	
	completions = [self completionsForXPath:xPath];
	
	return [completions objectAtIndex:index];

}

- (NSString *)comboBox:(NSComboBox *)control 
			  completedString:(NSString *)uncompletedString
{
	NSArray *completions;
	NSString *xPath;
	
	if([control respondsToSelector:@selector(xPathForPartialTag:)]){
		xPath = [(MWAutocompletingTextField *)control xPathForPartialTag:uncompletedString];
	} else {
		xPath = @"//@tag";
	}
	
	completions = [self completionsForXPath:xPath];
	
	return [completions objectAtIndex:0];
}


- (void)launchMiniInspector: (NSPoint)loc {
	if(![miniInspector isVisible]){
		[miniInspector orderFront:self];
		[miniInspector setFrameTopLeftPoint:loc];
	} else {
		[miniInspector orderOut:self];
	}
}


- (void)setSelectionWithNode:(id)node{
	NSTreeNode *root = [self arrangedRoot];

	NSIndexPath *match_path = [self findIndexPathForNode:node inTreeNode:root];
	
	NSLog(@"Attempting to change selection");
	[self setSelectionIndexPath:match_path];
	[self reloadData:self];
	[self takeFocus];

}

- (NSIndexPath *)findIndexPathForNode:(NSXMLNode *)node inTreeNode:(NSTreeNode *)tree_node{
	if([tree_node representedObject] == node){
		return [tree_node indexPath];
	}
	
	NSArray *child_nodes = [tree_node childNodes];
	NSEnumerator *child_node_enumerator = [child_nodes objectEnumerator];
	NSTreeNode *child_node;
	while(child_node = [child_node_enumerator nextObject]){
		NSIndexPath *path = [self findIndexPathForNode:node inTreeNode:child_node];
		
		if(path != Nil){
			return path;
		}
	}


	return Nil;
}


@end
