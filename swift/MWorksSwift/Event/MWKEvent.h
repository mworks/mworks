//
//  MWKEvent.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/9/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <MWorksSwift/MWorksSwiftDefines.h>


NS_ASSUME_NONNULL_BEGIN


typedef NSInteger MWKTime;
@class MWKDatum;


MWORKSSWIFT_PUBLIC_CLASS
@interface MWKEvent : NSObject

@property(nonatomic, readonly) NSInteger code;
@property(nonatomic, readonly) MWKTime time;
@property(nonatomic, readonly) MWKDatum *data;

@end


NS_ASSUME_NONNULL_END
