//
//  MWImageViewerController.h
//  ImageViewer
//
//  Created by Christopher Stawarz on 6/5/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

@import MWorksCocoa;

#import "MWImageViewerImageView.h"


NS_ASSUME_NONNULL_BEGIN


@interface MWImageViewerController : MWClientPluginViewController

// As of Xcode 11.5, Interface Builder doesn't see properties inherited from MWClientPluginViewController
// when MWorksCocoa is imported as a module.  The following declaration is copied from NSViewController.
@property(strong) IBOutlet NSView *view;

@property(nonatomic, weak) IBOutlet MWImageViewerImageView *imageView;

@property(nonatomic, copy) NSString *imageDataVariable;
@property(nonatomic, copy) NSString *regionOfInterestVariable;

- (IBAction)updateROI:(id)sender;
- (IBAction)clearROI:(id)sender;

@end


NS_ASSUME_NONNULL_END
