//
//  MWKTouchInputRecognizer.m
//  TouchInput
//
//  Created by Christopher Stawarz on 3/20/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#import "MWKTouchInputRecognizer.h"

#import <UIKit/UIGestureRecognizerSubclass.h>

#include "TouchInputDevice.hpp"


@interface MWKTouchInputRecognizerTarget : NSObject

- (void)handleGesture;

@end


@implementation MWKTouchInputRecognizerTarget


- (void)handleGesture {
    // Do nothing
}


@end


@implementation MWKTouchInputRecognizer {
    MWKTouchInputRecognizerTarget *touchInputRecognizerTarget;
    boost::weak_ptr<mw::TouchInputDevice> touchInputDeviceWeak;
}


- (instancetype)initWithTouchInputDevice:(const boost::shared_ptr<mw::TouchInputDevice> &)touchInputDevice {
    MWKTouchInputRecognizerTarget *target = [[MWKTouchInputRecognizerTarget alloc] init];
    self = [super initWithTarget:target action:@selector(handleGesture)];
    
    if (self) {
        touchInputRecognizerTarget = [target retain];
        touchInputDeviceWeak = touchInputDevice;
    }
    
    [target release];
    return self;
}


- (void)dealloc {
    [touchInputRecognizerTarget release];
    [super dealloc];
}


- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [super touchesBegan:touches withEvent:event];
    
    if (event.allTouches.count == 1) {
        if (auto touchInputDevice = touchInputDeviceWeak.lock()) {
            touchInputDevice->touchBegan([touches.anyObject locationInView:self.view]);
        }
    } else {
        // Ignore all touches after the first
        for (UITouch *touch in touches) {
            [self ignoreTouch:touch forEvent:event];
        }
    }
}


- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [super touchesMoved:touches withEvent:event];
    
    if (auto touchInputDevice = touchInputDeviceWeak.lock()) {
        touchInputDevice->touchMoved([touches.anyObject locationInView:self.view]);
    }
}


- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [super touchesEnded:touches withEvent:event];
    
    if (auto touchInputDevice = touchInputDeviceWeak.lock()) {
        touchInputDevice->touchEnded();
    }
}


- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [super touchesCancelled:touches withEvent:event];
    
    if (auto touchInputDevice = touchInputDeviceWeak.lock()) {
        touchInputDevice->touchEnded();
    }
}


@end



























