//
//  MWFindController.h
//  NewEditor
//
//  Created by David Cox on 6/12/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MWExperimentTreeController.h"
#import "MWXMLDocument.h"


@interface MWFindController : NSObject {

    NSString *searchString;
    NSArray *resultNodes;
    
    IBOutlet MWExperimentTreeController *experimentController;
    IBOutlet MWXMLDocument *document;
    
    int currentResult;
    
    BOOL hasResults;
}

@property(copy, readwrite) NSString *searchString;
@property int currentResult;
@property BOOL hasResults;

-(void)setSearchString:(NSString *)newstring;

-(IBAction)nextResult:(id)sender;
-(IBAction)previousResult:(id)sender;

@end
