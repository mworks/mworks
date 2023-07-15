//
//  MWImageViewerImageView.h
//  ImageViewer
//
//  Created by Christopher Stawarz on 6/6/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

@import AppKit;


NS_ASSUME_NONNULL_BEGIN


@interface MWImageViewerImageView : NSImageView

@property(nonatomic) NSRect regionOfInterest;
@property(nonatomic) NSRect selectedRegion;

- (void)receivedImageData:(nullable NSData *)imageData;

@end


NS_ASSUME_NONNULL_END
