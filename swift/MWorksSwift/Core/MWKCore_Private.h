//
//  MWKCore_Private.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/10/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <MWorksSwift/MWKCore.h>

#include <MWorksCore/VariableRegistryInterface.h>


NS_ASSUME_NONNULL_BEGIN


@interface MWKCore ()

- (instancetype)initWithCore:(const boost::shared_ptr<mw::RegistryAwareEventStreamInterface> &)core;

@property(nonatomic, readonly) const boost::shared_ptr<mw::RegistryAwareEventStreamInterface> &core;

@end


NS_ASSUME_NONNULL_END
