//
//  MWFindController.m
//  NewEditor
//
//  Created by David Cox on 6/12/08.
//  Copyright 2008 __MyCompanyName__. All rights reserved.
//

#import "MWFindController.h"


@implementation MWFindController

@synthesize searchString;
@synthesize currentResult;
@synthesize hasResults;

-(id)init {
    resultNodes = [[NSArray alloc] init];
    return self;
}

-(void)setSearchString:(NSString *)newstring{

    searchString = newstring;
    //NSString *xpath = @"//protocol";
    NSString *xpath = [NSString stringWithFormat:@"//*[contains(translate(@tag, 'abcdefghijklmnopqrstuvwxyz','ABCDEFGHIJKLMNOPQRSTUVWXYZ'), \"%@\")]", [searchString uppercaseString], Nil]; 
    //NSString *xpath = [NSString stringWithFormat:@"//[contains(@*,\"%@\")]", [searchString uppercaseString], Nil]; 
    
    NSLog(@"XPath = %@", xpath);
    // do XSLT search and populate resultNodes
    NSXMLDocument *doc = [document document];
    NSError *error = Nil;
    resultNodes = [doc nodesForXPath:xpath error:&error];
    
    if(error != Nil){
        NSLog(@"found %d nodes", [resultNodes count]);
    } else {
        NSLog(@"xpath error: %@", [error localizedDescription]);
    }
    
    if(resultNodes != Nil && [resultNodes count]){
        self.hasResults = YES;
        [experimentController setSelectionWithNode:[resultNodes objectAtIndex:0]];
    } else {
        self.hasResults = NO;
    }
}

-(IBAction)nextResult:(id)sender{
    if(resultNodes == Nil){
        self.currentResult = 0;
        return;
    }

    self.currentResult++;
    if(self.currentResult >= [resultNodes count]){
        self.currentResult = 0;
    }
    
    NSXMLNode *node = [resultNodes objectAtIndex:self.currentResult];
    NSLog([NSString stringWithFormat:@"next result: tag = %@", [node valueForKey:@"tag"],Nil]);
    [experimentController setSelectionWithNode:node];
}

-(IBAction)previousResult:(id)sender{
    if(resultNodes == Nil){
        self.currentResult = 0;
        return;
    }

    self.currentResult--;
    if(self.currentResult < 0){
        self.currentResult = [resultNodes count]-1;
    }
    
    NSXMLNode *node = [resultNodes objectAtIndex:self.currentResult];
    NSLog([NSString stringWithFormat:@"next result: tag = %@", [node valueForKey:@"tag"],Nil]);
    [experimentController setSelectionWithNode:node];
}



-(BOOL)control:(NSControl*)control textView:(NSTextView*)textView doCommandBySelector:(SEL)commandSelector {
    NSLog(@"delegate call");
    BOOL result = NO;	
//    if (commandSelector == @selector(insertNewline:)) {
//		// enter pressed
//        NSLog(@"enter");
//        // write the node to the pasteboard
//        NSXMLElement *element = [[self selectedObjects] objectAtIndex:0];
//        [libraryController writeElement:element toPasteboard:[NSPasteboard generalPasteboard]];
//        [experimentController paste:self];
//        [self dismiss];
//        [propertiesPanel makeKeyAndOrderFront:self];
//		result = YES;
//    }

    if(commandSelector == @selector(moveUp:)) {
		// up arrow pressed
        NSLog(@"up");
        [self previousResult:self];
		result = YES;
	}else if(commandSelector == @selector(moveDown:)) {
		// down arrow pressed
        NSLog(@"down");
		[self nextResult:self];
        result = YES;
	}
    return result;
}



@end
