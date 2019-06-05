//
//  MWImageViewerController.m
//  ImageViewer
//
//  Created by Christopher Stawarz on 6/5/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWImageViewerController.h"

#define DEFAULTS_IMAGE_DATA_VARIABLE_KEY @"Image Viewer - image data variable"
#define DEFAULTS_REGION_OF_INTEREST_VARIABLE_KEY @"Image Viewer - region of interest variable"

#define IMAGE_VIEWER_CALLBACK_KEY "MWImageViewerController callback key"

#define WORKSPACE_IMAGE_DATA_VARIABLE_KEY @"imageDataVariable"
#define WORKSPACE_REGION_OF_INTEREST_VARIABLE_KEY @"regionOfInterestVariable"


@implementation MWImageViewerController {
    int imageDataVariableCode;
    int regionOfInterestVariableCode;
    NSLayoutConstraint *imageViewAspectRatioConstraint;
}


- (void)awakeFromNib {
    [super awakeFromNib];
    
    imageDataVariableCode = -1;
    regionOfInterestVariableCode = -1;
    
    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
    NSString *imageDataVariableName = [defaults stringForKey:DEFAULTS_IMAGE_DATA_VARIABLE_KEY];
    if (imageDataVariableName) {
        self.imageDataVariableTextField.stringValue = imageDataVariableName;
    }
    NSString *regionOfInterestVariableName = [defaults stringForKey:DEFAULTS_REGION_OF_INTEREST_VARIABLE_KEY];
    if (regionOfInterestVariableName) {
        self.regionOfInterestVariableTextField.stringValue = regionOfInterestVariableName;
    }
}


- (void)controlTextDidEndEditing:(NSNotification *)notification {
    NSTextField *textField = notification.object;
    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
    if (textField == self.imageDataVariableTextField) {
        [defaults setObject:textField.stringValue forKey:DEFAULTS_IMAGE_DATA_VARIABLE_KEY];
    } else if (textField == self.regionOfInterestVariableTextField) {
        [defaults setObject:textField.stringValue forKey:DEFAULTS_REGION_OF_INTEREST_VARIABLE_KEY];
    }
    
    [self registerEventCallbacks];
}


- (void)setClient:(id<MWClientProtocol>)client {
    _client = client;
    [self registerEventCallbacks];
}


- (void)registerEventCallbacks {
    [self.client unregisterCallbacksWithKey:IMAGE_VIEWER_CALLBACK_KEY];
    
    [self.client registerEventCallbackWithReceiver:self
                                          selector:@selector(codecReceived:)
                                       callbackKey:IMAGE_VIEWER_CALLBACK_KEY
                                   forVariableCode:RESERVED_CODEC_CODE
                                      onMainThread:YES];
    
    {
        imageDataVariableCode = -1;
        NSString *imageDataVariableName = self.imageDataVariableTextField.stringValue;
        if (imageDataVariableName && imageDataVariableName.length > 0) {
            imageDataVariableCode = [self.client codeForTag:imageDataVariableName].intValue;
            if (imageDataVariableCode < 0) {
                mwarning(M_CLIENT_MESSAGE_DOMAIN,
                         "Image viewer can't find variable \"%s\"",
                         imageDataVariableName.UTF8String);
            } else {
                [self.client registerEventCallbackWithReceiver:self
                                                      selector:@selector(imageDataReceived:)
                                                   callbackKey:IMAGE_VIEWER_CALLBACK_KEY
                                               forVariableCode:imageDataVariableCode
                                                  onMainThread:YES];
            }
        }
    }
    
    {
        regionOfInterestVariableCode = -1;
        NSString *regionOfInterestVariableName = self.regionOfInterestVariableTextField.stringValue;
        if (regionOfInterestVariableName && regionOfInterestVariableName.length > 0) {
            regionOfInterestVariableCode = [self.client codeForTag:regionOfInterestVariableName].intValue;
            if (regionOfInterestVariableCode < 0) {
                mwarning(M_CLIENT_MESSAGE_DOMAIN,
                         "Image viewer can't find variable \"%s\"",
                         regionOfInterestVariableName.UTF8String);
            } else {
                [self.client registerEventCallbackWithReceiver:self
                                                      selector:@selector(regionOfInterestReceived:)
                                                   callbackKey:IMAGE_VIEWER_CALLBACK_KEY
                                               forVariableCode:regionOfInterestVariableCode
                                                  onMainThread:YES];
            }
        }
    }
}


- (void)codecReceived:(MWCocoaEvent *)event {
    [self registerEventCallbacks];
}


- (void)imageDataReceived:(MWCocoaEvent *)event {
    NSImage *image = nil;
    
    Datum &data = *(event.data);
    if (data.isString()) {
        auto &imageData = data.getString();
        auto imageDataSize = imageData.size();
        if (imageDataSize > 0) {
            image = [[NSImage alloc] initWithData:[NSData dataWithBytes:imageData.c_str() length:imageDataSize]];
        }
    }
    
    [self.imageView removeConstraint:imageViewAspectRatioConstraint];
    if (image) {
        NSSize imageSize = image.size;
        if (!NSEqualSizes(imageSize, NSZeroSize)) {
            imageViewAspectRatioConstraint = [self.imageView.heightAnchor constraintEqualToAnchor:self.imageView.widthAnchor
                                                                                       multiplier:(imageSize.height / imageSize.width)];
            imageViewAspectRatioConstraint.active = YES;
        }
    }
    
    self.imageView.image = image;
}


static bool extractRegionOfInterest(const Datum &datum, NSRect &regionOfInterest) {
    if (!datum.isList()) {
        // Non-list is OK and means no region of intereset
        return true;
    }
    
    auto &list = datum.getList();
    if (list.empty()) {
        // Empty list is OK and means no region of interest
        return true;
    } else if (list.size() != 4) {
        return false;
    }
    
    std::vector<double> values;
    for (auto &item : list) {
        if (!item.isNumber()) {
            return false;
        }
        values.push_back(item.getFloat());
        if (values.back() < 0.0 || values.back() > 1.0) {
            return false;
        }
    }
    
    auto &minX = values.at(0);
    auto &minY = values.at(1);
    auto &maxX = values.at(2);
    auto &maxY = values.at(3);
    
    if (minX >= maxX || minY >= maxY) {
        return false;
    }
    
    regionOfInterest = NSMakeRect(minX, minY, maxX - minX, maxY - minY);
    return true;
}


- (void)regionOfInterestReceived:(MWCocoaEvent *)event {
    NSRect regionOfInterest = NSZeroRect;
    if (!extractRegionOfInterest(*(event.data), regionOfInterest)) {
        merror(M_CLIENT_MESSAGE_DOMAIN, "Image viewer: region of interest is invalid");
    }
    self.imageView.regionOfInterest = regionOfInterest;
}


- (IBAction)updateROI:(id)sender {
    NSRect newROI = self.imageView.selectedRegion;
    if (!NSEqualRects(newROI, NSZeroRect)) {
        self.imageView.selectedRegion = NSZeroRect;
        if (regionOfInterestVariableCode < 0) {
            self.imageView.regionOfInterest = NSZeroRect;
        } else {
            self.imageView.regionOfInterest = newROI;
            Datum value { Datum::list_value_type {
                Datum { NSMinX(newROI) },
                Datum { NSMinY(newROI) },
                Datum { NSMaxX(newROI) },
                Datum { NSMaxY(newROI) }
            } };
            [self.client updateVariableWithCode:regionOfInterestVariableCode withData:&value];
        }
    }
}


- (IBAction)clearROI:(id)sender {
    self.imageView.selectedRegion = NSZeroRect;
    if (!NSEqualRects(self.imageView.regionOfInterest, NSZeroRect)) {
        self.imageView.regionOfInterest = NSZeroRect;
        if (regionOfInterestVariableCode >= 0) {
            Datum value { Datum::list_value_type { } };
            [self.client updateVariableWithCode:regionOfInterestVariableCode withData:&value];
        }
    }
}


- (NSDictionary *)workspaceState {
    NSMutableDictionary *workspaceState = [NSMutableDictionary dictionary];
    
    NSString *imageDataVariableName = self.imageDataVariableTextField.stringValue;
    if (imageDataVariableName && imageDataVariableName.length > 0) {
        workspaceState[WORKSPACE_IMAGE_DATA_VARIABLE_KEY] = imageDataVariableName;
    }
    
    NSString *regionOfInterestVariableName = self.regionOfInterestVariableTextField.stringValue;
    if (regionOfInterestVariableName && regionOfInterestVariableName.length > 0) {
        workspaceState[WORKSPACE_REGION_OF_INTEREST_VARIABLE_KEY] = regionOfInterestVariableName;
    }
    
    return workspaceState;
}


- (void)setWorkspaceState:(NSDictionary *)workspaceState {
    NSUserDefaults *defaults = NSUserDefaults.standardUserDefaults;
    
    NSString *imageDataVariableName = workspaceState[WORKSPACE_IMAGE_DATA_VARIABLE_KEY];
    if (imageDataVariableName && [imageDataVariableName isKindOfClass:[NSString class]]) {
        self.imageDataVariableTextField.stringValue = imageDataVariableName;
        [defaults setObject:imageDataVariableName forKey:DEFAULTS_IMAGE_DATA_VARIABLE_KEY];
    }
    
    NSString *regionOfInterestVariableName = workspaceState[WORKSPACE_REGION_OF_INTEREST_VARIABLE_KEY];
    if (regionOfInterestVariableName && [regionOfInterestVariableName isKindOfClass:[NSString class]]) {
        self.regionOfInterestVariableTextField.stringValue = regionOfInterestVariableName;
        [defaults setObject:regionOfInterestVariableName forKey:DEFAULTS_REGION_OF_INTEREST_VARIABLE_KEY];
    }
    
    [self registerEventCallbacks];
}


@end
