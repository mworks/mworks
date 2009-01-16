//
//  MWExperimentGraphController.h
//  NewEditor
//
//  Created by David Cox on 8/17/08.
//  Copyright 2008 Harvard University. All rights reserved.
//

#import "MWExperimentTreeController.h"
#import <Cocoa/Cocoa.h>
#import <Quartz/Quartz.h>
//#import "types.h"
//#import "graph.h"

@interface MWExperimentGraphController : NSObject {

	IBOutlet MWExperimentTreeController *experiment_controller;
	IBOutlet PDFView *pdf_view;	
	IBOutlet NSPanel *experiment_graph_panel;
	PDFDocument *pdf_document;
}


- (NSString *)nodeDefinitionStringFromXML:(NSXMLElement *)element
						  withElementType:(int)type;

@end
