//
//  MWKServer_Private.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/10/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <MWorksSwift/MWKServer.h>

#include <MWorksCore/Server.h>


NS_ASSUME_NONNULL_BEGIN


@interface MWKServer ()

- (instancetype)initWithServer:(const boost::shared_ptr<mw::Server> &)server;

@property(nonatomic, readonly) const boost::shared_ptr<mw::Server> &server;

@end


NS_ASSUME_NONNULL_END
