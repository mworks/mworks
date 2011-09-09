//
//  MWInspectorViewController.m
//  NewEditor
//
//  Created by David Cox on 11/5/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWInspectorViewController.h"
#import "MWCustomInspectorSubviewController.h"
#import "MWXMLElementAdditions.h"

@implementation MWInspectorViewController


-(id)init{
	current_view_index = 1;
	[self setAttributeFilter:[NSPredicate predicateWithFormat:@"!(%K BEGINSWITH \"_\")", @"name"]];
	return self;
}

- (void)awakeFromNib{
}

- (int)currentViewIndex{
	return current_view_index;
}

- (IBAction)update:(id)sender {
	NSArray *selected = [experiment_controller selectedObjects];
	
	if([selected count] == 0){
		[self setCurrentViewIndex:MWGenericInspectorKey];
		return;
	}
	
	NSXMLElement *current_element = [selected objectAtIndex:0];
	
	if([current_element hasAttribute:@"_inspector"]){
		[self setCurrentViewIndex:MWCustomInspectorKey];
	} else {
		[self setCurrentViewIndex:[self currentViewIndex]];
	}
}

- (void)setCurrentViewIndex:(int)ind{

	NSArray *selected;
	NSXMLElement *current_element;
	
	NSLog(@"Set index %d", ind);

	current_view_index = ind;
	switch([self currentViewIndex]){
		case MWCustomInspectorKey:
		
			selected = [experiment_controller selectedObjects];
			current_element = [selected objectAtIndex:0];
			
			NSXMLNode *att = [current_element attributeForName:@"_inspector"];
			
			if(att){
				NSString *inspector_nib = [att stringValue];
				
				if(inspector_nib == Nil){
					return [self setCurrentViewIndex:MWGenericInspectorKey];
				}
				
				NSLog(@"Inspector nib: %@", inspector_nib);
				
				if(![self loadNib:inspector_nib]){
					return [self setCurrentViewIndex:MWGenericInspectorKey];
				}
		
				// custom_view should have be set from inside the loaded Nib
				if(custom_view == Nil){
					return [self setCurrentViewIndex:MWGenericInspectorKey];
				}
			
                NSSize custom_inspector_size = [custom_view frame].size;
                NSSize new_window_size = custom_inspector_size;
                new_window_size.height += 40;
                NSRect new_window_frame = [inspector_panel frame];
                new_window_frame.size = new_window_size;
                [inspector_panel setFrame:new_window_frame display:YES 
                                                           animate:YES];
            
				[inspector_view setInspectorView:custom_view];
			} else {
				return [self setCurrentViewIndex:MWGenericInspectorKey];
			}
		
			break;
		case MWXMLInspectorKey:
			[inspector_view setInspectorView:xml_view];
			return;
		case MWGenericInspectorKey:
			[inspector_view setInspectorView:generic_view];
			return;
	}
}

- (IBAction)switchView:(id)sender{
	[self setCurrentViewIndex:[sender selectedSegment]];
}


- (BOOL)loadNib:(NSString *)name{

	// TODO: error check name
	if (![NSBundle loadNibNamed:name owner:self]){
        NSLog(@"Warning! Could not load nib file.\n");
		return NO;
    }
		
	// experiment_selection_controller should have been set from inside of the 
	if(selected_object_controller == Nil){
		return NO;
	}
	
	NSDictionary *options = 
		[NSDictionary dictionaryWithObjectsAndKeys:
			[NSNumber numberWithBool:YES], NSConditionallySetsEditableBindingOption,
			[NSNumber numberWithBool:YES], NSContinuouslyUpdatesValueBindingOption, 
												   nil];
	
	// Hook up the bindings programmatically
	[selected_object_controller bind:@"contentObject"
									 toObject:experiment_controller 
									 withKeyPath:@"selection.object"
									 options:options];
									 
	/*[selected_object_controller bind:@"managedObjectContext"
									 toObject:experiment_controller
									 withKeyPath:@""
									 options:nil];*/
									 
/*	[selected_object_controller addObserver:experiment_controller
		 forKeyPath:@"content"
			 options:(NSKeyValueObservingOptionNew |
						NSKeyValueObservingOptionOld)
				context:NULL];*/
	
	return YES;
}


- (NSPredicate *)attributeFilter {
	return attributeFilter;
}

- (void)setAttributeFilter: (NSPredicate *)_pred{
	attributeFilter = _pred;
}


- (NSArray *)control:(NSControl *)control 
			 textView:(NSTextView *)textView 
			 completions:(NSArray *)words 
			 forPartialWordRange:(NSRange)charRange 
			 indexOfSelectedItem:(int *)index{

	
	return [experiment_controller control:control 
			 textView:textView 
			 completions:words 
			 forPartialWordRange:charRange 
			 indexOfSelectedItem:index];

}


- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)control
{
	
	return[ experiment_controller numberOfItemsInComboBox:control];
}

- (id)comboBox:(NSComboBox *)control objectValueForItemAtIndex:(NSInteger)index
{

	return [experiment_controller comboBox:control 
								  objectValueForItemAtIndex:index];

}

- (NSString *)comboBox:(NSComboBox *)control 
			  completedString:(NSString *)uncompletedString
{
	
	return [experiment_controller comboBox:control 
								  completedString:uncompletedString];
}


- (IBAction)addAttribute:(id)sender {
	if(attributesController != Nil){
		int n = 0;
		NSArray *attributes = [attributesController arrangedObjects];
		BOOL okay = NO;
		NSString *new_attribute_name;
		while(!okay){
			okay = YES;
			new_attribute_name = [NSString stringWithFormat:@"new_attribute_%d", n++];
			int i;
			for(i = 0; i < [attributes count]; i++){
				NSString *name = [[attributes objectAtIndex:i] name];
				if([name isEqualToString:new_attribute_name]){
					okay = NO;
					break;
				}
			}
		}
		
		NSXMLNode *node = [NSXMLNode attributeWithName:new_attribute_name stringValue:@""];
		[attributesController addObject:node];
	}
}


/*#pragma mark -
#pragma mark Textfield Autocompletion Delegate Methods

- (NSArray *)control:(NSControl *)control 
			 textView:(NSTextView *)textView 
			 completions:(NSArray *)words 
			 forPartialWordRange:(NSRange)charRange 
			 indexOfSelectedItem:(int *)index{

	// TODO: also check for a xPathForPartialTag: withNodeID: selector
	//		 get the node ID from the currently selected
	
	if(![control respondsToSelector:@selector(xPathForPartialTag:)]){
		return [NSArray array];
	}
	
	NSError *outError;
	NSString *partialword = [[control stringValue] substringWithRange:charRange];
	
	NSString *xPath = [control xPathForPartialTag:partialword];
	
	return [experiment_controller completionsForXPath:xPath];
}*/


@end
