//
//  MWLibraryTreeContoller.h
//  NewEditor
//
//  Created by David Cox on 11/3/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MWXMLDocument.h"


@interface MWLibraryTreeController : NSTreeController {
	IBOutlet NSBrowser *library_tree;
	IBOutlet MWXMLDocument *document;
}


- (BOOL) writeElement:(NSXMLElement *)element toPasteboard:(NSPasteboard *)pboard;

@end
