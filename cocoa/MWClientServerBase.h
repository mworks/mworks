//
//  MWClientServerBase.h
//  MWorksCocoa
//
//  Created by Ben Kennedy on 9/19/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#import <MWorksSwift/MWorksSwift.h>


@protocol MWClientServerBase <NSObject>

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                             onMainThread:(BOOL)on_main;

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                          forVariableCode:(int)code
                             onMainThread:(BOOL)on_main;

- (void)unregisterCallbacksWithKey:(const char *)key;
- (NSNumber *)codeForTag:(NSString *)tag;

@end


@interface MWClientServerBase : NSObject <MWClientServerBase>

- (instancetype)initWithCore:(MWKCore *)core;

@property(nonatomic, readonly) MWKCore *core;

@end
