//
//  MWKEvent.m
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/9/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWKEvent_Private.h"

#import "MWKDatum_Private.h"


static_assert(sizeof(MWKTime) >= sizeof(mw::MWTime), "MWKTime cannot store mw::MWTime");


@implementation MWKEvent {
    boost::shared_ptr<mw::Event> _event;
}


- (instancetype)initWithEvent:(const boost::shared_ptr<mw::Event> &)event {
    self = [super init];
    if (self) {
        _event = event;
    }
    return self;
}


- (const boost::shared_ptr<mw::Event> &)event {
    return _event;
}


- (NSInteger)code {
    return self.event->getEventCode();
}


- (MWKTime)time {
    return self.event->getTime();
}


- (MWKDatum *)data {
    return [MWKDatum datumWithDatum:self.event->getData()];
}


@end
