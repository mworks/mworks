//
//  MWLibraryBrowserView.m
//  NewEditor
//
//  Created by David Cox on 11/17/07.
//  Copyright 2007 __MyCompanyName__. All rights reserved.
//

#import "MWLibraryBrowserView.h"
#import "MWLibraryBrowserMatrix.h"
#import "MWLibraryTreeController.h"


@implementation MWLibraryBrowserView

- (id)initWithCoder:(NSCoder *)decoder {
    if ((self = [super initWithCoder:decoder])) {
        [self setMatrixClass:[MWLibraryBrowserMatrix class]];
    }
    return self;
}

- (IBAction)copy:(id)sender{

	[(MWLibraryTreeController *)[self delegate] copy:sender];
}


@end
