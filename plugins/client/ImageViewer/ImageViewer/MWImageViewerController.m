//
//  MWImageViewerController.m
//  ImageViewer
//
//  Created by Christopher Stawarz on 6/5/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWImageViewerController.h"


@implementation MWImageViewerController {
    id imageDataVariableProperty;
    id regionOfInterestVariableProperty;
    NSLayoutConstraint *imageViewAspectRatioConstraint;
}


// Silence warnings from compiler that view won't be synthesized automatically
@dynamic view;


- (instancetype)initWithClient:(id<MWKClient>)client {
    self = [super initWithClient:client];
    if (self) {
        self.title = @"Image Viewer";
        
        _imageDataVariable = @"";
        _regionOfInterestVariable = @"";
        
        imageDataVariableProperty =
            [self registerStoredPropertyWithName:NSStringFromSelector(@selector(imageDataVariable))
                                     defaultsKey:@"Image Viewer - image data variable"
                                    handleEvents:YES];
        regionOfInterestVariableProperty =
            [self registerStoredPropertyWithName:NSStringFromSelector(@selector(regionOfInterestVariable))
                                     defaultsKey:@"Image Viewer - region of interest variable"
                                    handleEvents:YES];
    }
    return self;
}


- (BOOL)validateWorkspaceValue:(id)value forStoredProperty:(id)property {
    if (property == imageDataVariableProperty || property == regionOfInterestVariableProperty) {
        return [value isKindOfClass:[NSString class]] && ((NSString *)value).length > 0;
    }
    return YES;
}


- (void)handleEvent:(MWKEvent *)event forStoredProperty:(id)property {
    if (property == imageDataVariableProperty) {
        [self imageDataReceived:event];
    } else if (property == regionOfInterestVariableProperty) {
        [self regionOfInterestReceived:event];
    }
}


- (void)imageDataReceived:(MWKEvent *)event {
    NSImage *image = nil;
    
    NSData *imageData = event.data.bytesValue;
    if (imageData && imageData.length > 0) {
        image = [[NSImage alloc] initWithData:imageData];
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        [self.imageView removeConstraint:self->imageViewAspectRatioConstraint];
        if (image) {
            NSSize imageSize = image.size;
            if (!NSEqualSizes(imageSize, NSZeroSize)) {
                self->imageViewAspectRatioConstraint = [self.imageView.heightAnchor constraintEqualToAnchor:self.imageView.widthAnchor
                                                                                                 multiplier:(imageSize.height / imageSize.width)];
                self->imageViewAspectRatioConstraint.active = YES;
            }
        }
        self.imageView.image = image;
    });
}


- (void)regionOfInterestReceived:(MWKEvent *)event {
    __block NSRect regionOfInterest = NSZeroRect;
    
    BOOL roiIsValid = ^{
        NSArray<MWKDatum *> *list = event.data.listValue;
        if (!list) {
            // Non-list is OK and means no region of intereset
            return YES;
        }
        
        if (list.count == 0) {
            // Empty list is OK and means no region of interest
            return YES;
        } else if (list.count != 4) {
            return NO;
        }
        
        NSMutableArray<NSNumber *> *values = [NSMutableArray arrayWithCapacity:4];
        for (MWKDatum *item in list) {
            NSNumber *number = item.numberValue;
            if (!number || number.doubleValue < 0.0 || number.doubleValue > 1.0) {
                return NO;
            }
            [values addObject:number];
        }
        
        double minX = values[0].doubleValue;
        double minY = values[1].doubleValue;
        double maxX = values[2].doubleValue;
        double maxY = values[3].doubleValue;
        
        if (minX >= maxX || minY >= maxY) {
            return NO;
        }
        
        regionOfInterest = NSMakeRect(minX, minY, maxX - minX, maxY - minY);
        return YES;
    }();
    if (!roiIsValid) {
        [self postError:@"Region of interest is invalid"];
    }
    
    dispatch_async(dispatch_get_main_queue(), ^{
        self.imageView.regionOfInterest = regionOfInterest;
    });
}


- (IBAction)updateROI:(id)sender {
    NSRect newROI = self.imageView.selectedRegion;
    if (!NSEqualRects(newROI, NSZeroRect)) {
        self.imageView.selectedRegion = NSZeroRect;
        MWKDatum *value = [MWKDatum datumWithList:@[
            [MWKDatum datumWithFloat:NSMinX(newROI)],
            [MWKDatum datumWithFloat:NSMinY(newROI)],
            [MWKDatum datumWithFloat:NSMaxX(newROI)],
            [MWKDatum datumWithFloat:NSMaxY(newROI)]
        ]];
        if ([self.client setValue:value forTag:self.regionOfInterestVariable]) {
            self.imageView.regionOfInterest = newROI;
        } else {
            self.imageView.regionOfInterest = NSZeroRect;
        }
    }
}


- (IBAction)clearROI:(id)sender {
    self.imageView.selectedRegion = NSZeroRect;
    if (!NSEqualRects(self.imageView.regionOfInterest, NSZeroRect)) {
        MWKDatum *value = [MWKDatum datumWithList:@[]];
        [self.client setValue:value forTag:self.regionOfInterestVariable];
        self.imageView.regionOfInterest = NSZeroRect;
    }
}


@end
