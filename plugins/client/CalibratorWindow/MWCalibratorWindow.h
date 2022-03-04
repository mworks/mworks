//
//  MWCalibratorWindow.h
//  MWorksCalibratorWindow
//
//  Created by Ben Kennedy on 5/3/08.
//  Copyright 2008 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWCalibratorWindow : NSWindow {
	IBOutlet id __unsafe_unretained controller;
}

@property (nonatomic, readwrite, unsafe_unretained) id controller;

@end
