//
//  MWorksSwiftErrors.m
//  MWorksSwift
//
//  Created by Christopher Stawarz on 9/6/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import "MWorksSwiftErrors_Private.h"


NSErrorDomain const MWorksSwiftErrorDomain = @"org.mworks-project.MWorksSwift.ErrorDomain";


NSError * MWorksSwiftConvertExceptionToNSError(std::exception_ptr eptr) {
    NSString *localizedDescription = nil;
    try {
        std::rethrow_exception(eptr);
    } catch (const std::exception &e) {
        localizedDescription = @(e.what());
    } catch (...) {
        localizedDescription = @"An unknown error occurred";
    }
    return [NSError errorWithDomain:MWorksSwiftErrorDomain
                               code:MWorksSwiftErrorUnspecified
                           userInfo:@{ NSLocalizedDescriptionKey: localizedDescription }];
}


void MWorksSwiftLogException(std::exception_ptr eptr) {
    mw::MessageDomain domain = mw::M_GENERIC_MESSAGE_DOMAIN;
    std::string message;
    try {
        std::rethrow_exception(eptr);
    } catch (mw::SimpleException &e) {
        domain = e.getDomain();
        message = e.getMessage();
    } catch (const std::exception &e) {
        message = e.what();
    } catch (...) {
        message = "An unknown error occurred";
    }
    mw::merror(domain, "%s", message.c_str());
}
