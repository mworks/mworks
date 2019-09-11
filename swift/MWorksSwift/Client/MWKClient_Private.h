//
//  MWKClient.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/11/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <MWorksSwift/MWKClient.h>

#include <MWorksCore/Client.h>


NS_ASSUME_NONNULL_BEGIN


@interface MWKClient ()

- (instancetype)initWithClient:(const boost::shared_ptr<mw::Client> &)client;

@property(nonatomic, readonly) const boost::shared_ptr<mw::Client> &client;

@end


NS_ASSUME_NONNULL_END
