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


@interface MWKMouseTracker : NSObject

@property (nonatomic, assign) BOOL shouldHideCursor;

- (id)initWithMouseInputDevice:(boost::shared_ptr<mw::MouseInputDevice>)mouseInputDevice;
- (void)hideCursor;
- (void)unhideCursor;

@end
