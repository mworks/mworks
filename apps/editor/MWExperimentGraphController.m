//
//  MWExperimentGraphController.m
//  NewEditor
//
//  Created by David Cox on 8/17/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWExperimentGraphController.h"


@implementation MWExperimentGraphController

enum MWGraphOutlineType { CURRENT_ELEMENT = 0, PARENT_ELEMENT=1, CHILD_ELEMENT=2 };


- (void)awakeFromNib {
	
	[pdf_view setAutoScales:YES];
	
	NSString *dot = @"digraph G{ experiment[shape=box, style=bold]; }";	
	[self renderDot:dot];
	
	[self registerObservers];
}


- (void) renderDot:(NSString *)dot{
	NSData *pdf_data = [self callGraphvizDot:dot];
    
    if(pdf_data == Nil){
        NSString *noGraphvizPath = [[NSBundle mainBundle]  
										pathForResource:@"no_graphviz" 
										ofType:@"pdf"];
        
        pdf_document = [[PDFDocument alloc] initWithURL:[NSURL fileURLWithPath:noGraphvizPath]];
    } else {
        pdf_document = [[PDFDocument alloc] initWithData:pdf_data];
	}
    
	//pdf_document = [[PDFDocument alloc] initWithURL:[NSURL URLWithString:@"file:///Users/davidcox/Desktop/CalibImage_32cm.pdf"]];
	[pdf_view setDocument:pdf_document];
	
	PDFPage *first_page = [pdf_document pageAtIndex:0];
	NSRect page_bounds = [first_page boundsForBox:kPDFDisplayBoxCropBox];
	NSSize custom_view_size = page_bounds.size;
	NSSize new_window_size = custom_view_size;
	new_window_size.height += 60;
	NSRect new_window_frame = [experiment_graph_panel frame];
	new_window_frame.size = new_window_size;
	[experiment_graph_panel setFrame:new_window_frame display:YES 
					  animate:YES];
	
	[pdf_view setNeedsDisplay:YES];
}


- (void)registerObservers {
	
	[experiment_controller addObserver:self
							forKeyPath:@"selection"
							options: (NSKeyValueObservingOptionNew | NSKeyValueObservingOptionOld)
							   context:Nil];
}


- (void)observeValueForKeyPath:(NSString *)keyPath

					  ofObject:(id)object

                        change:(NSDictionary *)change

                       context:(void *)context

{

	NSLog(@"Graph viewer: something changed");
    
    if([[experiment_controller selectedNodes] count] <= 0){
        return;
    }
	NSXMLElement *element = [experiment_controller valueForKeyPath:@"selection.self"];
	if(element == Nil){
        return;
    }
	NSLog(@"Name: %@", [element valueForKey:@"tag"]);
	
	NSString *dot = [self dotForElement:element];
	[self renderDot:dot];
}




- (NSString *)dotForElement:(NSXMLElement *)element {
	
	NSMutableString *outstring = [[NSMutableString alloc] init];
	[outstring appendString:@"digraph G { \n"];
	
	// define the node itself

	[outstring appendString:[self nodeDefinitionStringFromXML:element withElementType:CURRENT_ELEMENT]];
	
	// define parent if there is one
	NSXMLElement *parent = (NSXMLElement *)[element parent];
	if(parent != Nil){
		[outstring appendString:[self nodeDefinitionStringFromXML:parent withElementType:PARENT_ELEMENT]];
		[outstring appendFormat:@"%@ -> %@ [color=gray];\n", [parent valueForKey:@"_id"], 
                                    [element valueForKey:@"_id"], Nil];
	}
	
	// define children
	NSArray *children = [element children];
	if([children count]){
		NSEnumerator *child_enumerator = [children objectEnumerator];
		NSXMLElement *child;
		while(child = [child_enumerator nextObject]){
			[outstring appendString:[self nodeDefinitionStringFromXML:child withElementType:CHILD_ELEMENT]];
			[outstring appendFormat:@"%@ -> %@;\n", [element valueForKey:@"_id"], [child valueForKey:@"_id"], Nil];
		}
	}
	
	[outstring appendString:@"\n}"];
	return outstring;
}


- (NSString *)nodeDefinitionStringFromXML:(NSXMLElement *)element
                        withElementType:(int)type {

	NSString *_id = [element valueForKey:@"_id"];
	NSString *tag = [element valueForKey:@"tag"];
        
    NSString *options_string;
    
    switch(type){
        case CURRENT_ELEMENT:
            options_string = @"fontname=\"LucidaGrande-Bold\", shape=rectangle, fontsize=12, fillcolor=lightblue, style=filled,\n";
            break;
        case CHILD_ELEMENT:
            options_string = @"fontname=\"LucidaGrande\", height=0.1, shape=rectangle, fontsize=10";
            break;
        case PARENT_ELEMENT:
        default:
            options_string = @"fontname=\"LucidaGrande\", height=0.1, shape=rectangle,  style=filled, fillcolor=gray, color=gray, fontsize=10";
            break;
    }
    

	NSMutableString *outstring = [[NSMutableString alloc] init];
	
	
	
	if([[element name] isEqualToString:@"task_system_state"]){
		NSLog(@"task_system_state");
		//[outstring appendFormat:@"subgraph %@ {label = \"%@\"; %@", [element valueForKey:@"_id"], [element valueForKey:@"tag"], options_string, Nil];
		
		
		

		NSString *head = @"%@[shape=plaintext, label=<"
                "<table border=\"1\" cellborder=\"0\" cellspacing=\"0\" cellpadding=\"2\">"
                "<tr><td cellpadding=\"8\" bgcolor=\"lightblue\" colspan=\"3\" border=\"0\"><font color=\"black\" face=\"LucidaGrande-Bold\">%@</font></td></tr>"
                "<tr><td colspan=\"3\"> </td></tr>"
                "<tr><td rowspan=\"3\"> </td>"
                "<td bgcolor=\"black\" border=\"1\"><font color=\"white\" face=\"LucidaGrande\" point-size=\"10\">Actions</font></td>"
                "<td rowspan=\"3\"> </td></tr>";
        
        [outstring appendFormat:head, _id, tag, Nil];
                
        NSString *actions = @"<tr><td border=\"1\" align=\"left\"><font face=\"LucidaGrande\" point-size=\"10\"> %@ </font></td></tr>"
                            "<tr><td> </td></tr>";
                            
        NSMutableString *action_list = [[NSMutableString alloc] init];
        NSArray *children = [element children];
		NSEnumerator *child_enumerator = [children objectEnumerator];
		NSXMLElement *child;
		
		// Start actions node
        BOOL first = YES;
		while(child = [child_enumerator nextObject]){

			if([[child name] isEqualToString:@"action"]){
                if(first){
                    first = NO;
                } else {
                    [action_list appendString:@"<br/>\n"];
                }
				[action_list appendFormat:@"%@", [child valueForKey:@"tag"], Nil];
			}
		}
        
        [outstring appendFormat:actions, action_list];
                
        NSString *middle = @"<tr><td rowspan=\"2\"> </td>"
                "<td bgcolor=\"black\" border=\"1\"><font color=\"white\" face=\"LucidaGrande\" point-size=\"10\">Transitions</font></td>"
                "<td rowspan=\"2\"> </td></tr>";
		
        [outstring appendString:middle];

        NSString *transitions = @"<tr><td border=\"1\" port=\"t\"><font face=\"LucidaGrande\" point-size=\"10\"> %@ </font></td></tr>";
		NSMutableString *transition_list = [[NSMutableString alloc] init];
        
        child_enumerator = [children objectEnumerator];
        first = YES;
        while(child = [child_enumerator nextObject]){
            
            if([[child name] isEqualToString:@"transition"]){
                if(first){
                    first = NO;
                } else {
                    [transition_list appendString:@"<br/>\n"];
                }
				[transition_list appendFormat:@"%@", [child valueForKey:@"tag"], Nil];
			}
		}
        
        [outstring appendFormat:transitions, transition_list];
        
        NSString *finish = @"<tr><td colspan=\"3\"> </td></tr></table>>];";
		
		[outstring appendString:finish];
        
		
	} else {
		[outstring appendFormat:@"%@[label=\"%@\", %@]\n;", 
                                                    [element valueForKey:@"_id"],
                                                    [element valueForKey:@"tag"], 
                                                    options_string,
                                                    Nil];
	
	}
	return outstring;
}



- (NSData*)callGraphvizDot:(NSString *)dot_input {
	NSTask *task;
    task = [[NSTask alloc] init];
    
    [task setLaunchPath: @"/usr/local/bin/dot"];
        
    NSLog(dot_input);
    
    NSArray *arguments;
    arguments = [NSArray arrayWithObjects: @"-Tpdf", nil];
    [task setArguments: arguments];
	
	NSPipe *inputPipe;
    inputPipe = [NSPipe pipe];
    [task setStandardInput: inputPipe];
	
    NSPipe *pipe;
    pipe = [NSPipe pipe];
    [task setStandardOutput: pipe];
	
	
	NSFileHandle *inFile;
	inFile = [inputPipe fileHandleForWriting];
	
    NSFileHandle *file;
    file = [pipe fileHandleForReading];
	
    
    @try{
        [task launch];
	} @catch (id theexception){
        return Nil;
    }

    
    
	[inFile writeData:[dot_input dataUsingEncoding:NSASCIIStringEncoding]];
	[inFile closeFile];
	
    NSData *data;
    data = [file readDataToEndOfFile];
	
	[file closeFile];
	
	return data;
	
}	



@end
