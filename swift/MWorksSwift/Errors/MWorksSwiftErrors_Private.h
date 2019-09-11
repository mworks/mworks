//
//  MWorksSwiftErrors_Private.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/6/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <MWorksSwift/MWorksSwiftErrors.h>

#include <exception>


NS_ASSUME_NONNULL_BEGIN


MWORKSSWIFT_PUBLIC_EXTERN NSError * MWorksSwiftConvertExceptionToNSError(std::exception_ptr eptr);
MWORKSSWIFT_PUBLIC_EXTERN void MWorksSwiftLogException(std::exception_ptr eptr);


NS_ASSUME_NONNULL_END
