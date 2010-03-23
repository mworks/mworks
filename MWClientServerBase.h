//
//  MWClientServerBase.h
//  MWorksCocoa
//
//  Created by Ben Kennedy on 9/19/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MWorksCore/GenericData.h"
using namespace mw;

@interface MWClientServerBase : NSObject {
}

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key;

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                          forVariableCode:(int)code;

- (void)unregisterCallbacksWithKey:(const char *)key;
- (NSNumber *)codeForTag:(NSString *)tag;
- (void)set:(int)code to:(Datum *)data;
- (NSNumber *)isLocalMessage:(NSNumber *)messageOrigin;
- (NSNumber *)maxConsoleLength;
- (NSArray *)variableNames;


@end
