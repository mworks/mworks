//
//  MWorksSwiftErrors_Private.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/6/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWorksSwiftErrors.h"


NS_ASSUME_NONNULL_BEGIN


NSError * MWorksSwiftConvertExceptionToNSError(std::exception_ptr eptr);
void MWorksSwiftLogException(std::exception_ptr eptr);


NS_ASSUME_NONNULL_END
