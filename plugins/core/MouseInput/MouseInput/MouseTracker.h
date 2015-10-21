//
//  MouseTracker.h
//  MouseInput
//
//  Created by Christopher Stawarz on 5/29/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//


namespace mw {
    class MouseInputDevice;  // Forward declaration
}


@interface MWKMouseTracker : NSObject {
    boost::weak_ptr<mw::MouseInputDevice> mouseInputDeviceWeak;
    BOOL shouldHideCursor;
    id upDownEventMonitor;
    id dragEventMonitor;
}

@property (nonatomic, assign) BOOL shouldHideCursor;

- (id)initWithMouseInputDevice:(boost::shared_ptr<mw::MouseInputDevice>)mouseInputDevice;

@end
