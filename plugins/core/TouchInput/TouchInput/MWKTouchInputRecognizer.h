//
//  MWKTouchInputRecognizer.h
//  TouchInput
//
//  Created by Christopher Stawarz on 3/20/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//


namespace mw {
    class TouchInputDevice;  // Forward declaration
}


@interface MWKTouchInputRecognizer : UIGestureRecognizer

- (instancetype)initWithTouchInputDevice:(const boost::shared_ptr<mw::TouchInputDevice> &)touchInputDevice;

@end
