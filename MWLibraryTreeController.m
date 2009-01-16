//
//  MWLibraryTreeContoller.m
//  NewEditor
//
//  Created by David Cox on 11/3/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWLibraryTreeController.h"
#import "MWXMLElementAdditions.h"
#import "ImageAndTextCell.h"

@implementation MWLibraryTreeController


- (void)awakeFromNib {

	//[library_tree setVerticalMotionCanBeginDrag:YES];
}


- (void)browser:(NSBrowser *)sender willDisplayCell:(id)cell 
									atRow:(NSInteger)row 
									column:(NSInteger)column {
	NSXMLElement *element = [[cell representedObject] representedObject];
	
	NSString *name = [element valueForKey:@"icon"];
	NSString *fullpath = [[NSBundle mainBundle] pathForResource:name ofType:@"tif"];
	NSLog(@"full icon path: %@", fullpath);
    NSImage *img = [[NSImage alloc] initWithContentsOfFile:fullpath];
	
	[cell setImage:img];
	
	[cell setStringValue:[element valueForKey:@"name"]];
}

- (void)browser:(NSBrowser *)sender createRowsForColumn:(NSInteger)column inMatrix:(NSMatrix *)matrix{
	return;
}


- (IBAction)copy:(id)sender{
	NSArray *objects = [self selectedObjects];
	[self writeElement:[objects objectAtIndex:0] toPasteboard:[NSPasteboard generalPasteboard]];
}


- (BOOL)browser:(NSBrowser *)sender 
	    writeRowsWithIndexes:(NSIndexSet *)rowIndexes 
		inColumn:(NSInteger)columnIndex 
		toPasteboard:(NSPasteboard *)pboard {


	id cell = [sender loadedCellAtRow:[rowIndexes firstIndex] column:columnIndex];
	NSXMLElement *element = [[cell representedObject] representedObject];

	return [self writeElement:element toPasteboard:pboard];
}



- (BOOL) writeElement:(NSXMLElement *)element toPasteboard:(NSPasteboard *)pboard {

	NSError *outError;

	NSXMLNode *name_att = [element attributeForName:@"name"];
	NSString *name_string = [name_att stringValue];
	
	NSArray *code_elements = [element elementsForName:@"code"];
	if([code_elements count] == 0){
		return NO;
	} else {
		NSXMLElement *code_element = [[code_elements objectAtIndex:0] copyWithZone:Nil];
		
		[code_element detach];
		// Build a temporary document so that we can apply an XSLT transform
		NSXMLDocument *temp_doc = [[NSXMLDocument alloc] initWithRootElement:code_element];
		NSString *display_hint_xslt_string = [document displayHintTransformString];
		NSXMLDocument *transformed_doc = [temp_doc objectByApplyingXSLTString:display_hint_xslt_string
												   arguments:Nil
												   error:&outError];
		
		code_element = [transformed_doc rootElement];
		NSArray *code_element_children = [code_element children];
		
		// These are the types that we will be putting into the pasteboard
		[pboard declareTypes:[NSArray arrayWithObjects:NSStringPboardType, nil] owner:self];
		
		
		// Tag the to-be-dragged code element with an _element_key hint before
		// we write it to the pasteboard.  This will aid in determining
		// what can be drag-n-dropped where.
		NSXMLElement *code_element_child = [code_element_children objectAtIndex:0];
		[code_element_child addAttribute:[NSXMLNode attributeWithName:@"_element_key"
											  stringValue:name_string]];
											  

		NSString *new_id = [document newInternalID];
		[code_element_child addAttribute:[NSXMLNode attributeWithName:@"_id"
											  stringValue:new_id]];
											  
		// TODO: add display hints
		
		[pboard setString:[code_element_child prettyXMLString] forType:NSStringPboardType];
		return YES;
	}
	
	return NO;
}


@end
