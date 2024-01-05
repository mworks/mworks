//
//  MWorksSwiftErrors.h
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/6/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <Foundation/Foundation.h>

#import <MWorksSwift/MWorksSwiftDefines.h>


NS_ASSUME_NONNULL_BEGIN


MWORKSSWIFT_PUBLIC_EXTERN NSErrorDomain const MWorksSwiftErrorDomain;

typedef NS_ERROR_ENUM(MWorksSwiftErrorDomain, MWorksSwiftError) {
    MWorksSwiftErrorUnspecified = 1,
    MWorksSwiftErrorComponentFactoryConflict = 2
};


NS_ASSUME_NONNULL_END
