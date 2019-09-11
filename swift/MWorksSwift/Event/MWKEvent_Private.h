//
//  MWKEvent_Private.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/9/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWKEvent.h"


NS_ASSUME_NONNULL_BEGIN


@interface MWKEvent ()

- (instancetype)initWithEvent:(const boost::shared_ptr<mw::Event> &)event;

@property(nonatomic, readonly) const boost::shared_ptr<mw::Event> &event;

@end


NS_ASSUME_NONNULL_END
