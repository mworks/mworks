//
//  MWLibraryBrowserView.m
//  NewEditor
//
//  Created by David Cox on 11/17/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWLibraryBrowserView.h"


@implementation MWLibraryBrowserView

- (IBAction)copy:(id)sender{

	[[self delegate] copy:sender];
}


@end
