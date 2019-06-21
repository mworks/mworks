//
//  MWKDatum_Private.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 6/14/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWKDatum.h"


NS_ASSUME_NONNULL_BEGIN


@interface MWKDatum ()

+ (instancetype)datumWithDatum:(const mw::Datum &)datum;
- (instancetype)initWithDatum:(const mw::Datum &)datum;

@property(nonatomic, readonly) const mw::Datum &datum;

@end


NS_ASSUME_NONNULL_END
