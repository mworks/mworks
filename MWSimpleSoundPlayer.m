//
//  MWSimpleSoundPlayer.m
//  NewEditor
//
//  Created by David Cox on 9/11/08.
//  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
//

#import "MWSimpleSoundPlayer.h"


@implementation MWSimpleSoundPlayer

- (IBAction) playSound:(id)sender{

    NSString *path = [control_containing_path stringValue];
    NSLog(@"PATH: %@", path);
    NSSound *sound = [[NSSound alloc] initWithContentsOfURL:[NSURL URLWithString:path] byReference:NO];
    [sound play];
}

@end
