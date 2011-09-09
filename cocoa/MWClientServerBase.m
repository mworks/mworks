//
//  MWClientServerBase.m
//  MWorksCocoa
//
//  Created by Ben Kennedy on 9/19/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWClientServerBase.h"
#import "MWorksCore/Utilities.h"


@implementation MWClientServerBase

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key {
	[NSException raise:NSInternalInconsistencyException
				format:@"registerEventCallbackWithRecevier:andSelector:andKey needs to be overridden"];
	
}

- (void)registerEventCallbackWithReceiver:(id)receiver 
                                 selector:(SEL)selector
                              callbackKey:(const char *)key
                          forVariableCode:(int)code {
	[NSException raise:NSInternalInconsistencyException
				format:@"registerEventCallbackWithRecevier:andSelector:andKey:forVariable needs to be overridden"];	
}

- (NSNumber *)codeForTag:(NSString *)tag {
	[NSException raise:NSInternalInconsistencyException
				format:@"codeForTag: needs to be overridden"];	
	
	return nil;
}

- (void)set:(int)code to:(Datum *)data {
	[NSException raise:NSInternalInconsistencyException
				format:@"set:to: needs to be overridden"];	
}

- (void)unregisterCallbacksWithKey:(const char *)key {
	[NSException raise:NSInternalInconsistencyException
				format:@"unregisterCallbacksWithKey: needs to be overridden"];	
}

- (NSNumber *)isLocalMessage:(NSNumber *)messageOrigin {
	return [NSNumber numberWithBool:(GlobalMessageOrigin == [messageOrigin intValue])];
}	

- (NSNumber *)maxConsoleLength {
	return [NSNumber numberWithInt:100000];
}

- (NSArray *)variableNames {
	[NSException raise:NSInternalInconsistencyException
				format:@"variableNames needs to be overridden"];	
	return nil;
}

@end
