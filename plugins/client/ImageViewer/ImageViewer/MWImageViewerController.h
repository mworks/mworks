//
//  MWImageViewerController.h
//  ImageViewer
//
//  Created by Christopher Stawarz on 6/5/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWImageViewerImageView.h"


NS_ASSUME_NONNULL_BEGIN


@interface MWImageViewerController : NSWindowController <NSTextFieldDelegate, MWClientPluginWorkspaceState>

@property(nonatomic, weak) IBOutlet id<MWClientProtocol> client;

@property(nonatomic, weak) IBOutlet MWImageViewerImageView *imageView;
@property(nonatomic, weak) IBOutlet NSTextField *imageDataVariableTextField;
@property(nonatomic, weak) IBOutlet NSTextField *regionOfInterestVariableTextField;

- (IBAction)updateROI:(id)sender;
- (IBAction)clearROI:(id)sender;

@end


NS_ASSUME_NONNULL_END
