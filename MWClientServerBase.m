//
//  MWClientServerBase.m
//  MonkeyWorksCocoa
//
//  Created by Ben Kennedy on 9/19/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import "MWClientServerBase.h"
#import "MonkeyWorksCore/Utilities.h"


@implementation MWClientServerBase

- (void)registerEventCallbackWithRecevier:(id)receiver 
							  andSelector:(SEL)selector
								   andKey:(NSString *)key {
	[NSException raise:NSInternalInconsistencyException
				format:@"registerEventCallbackWithRecevier:andSelector:andKey needs to be overridden"];
	
}

- (void)registerEventCallbackWithRecevier:(id)receiver 
							  andSelector:(SEL)selector
								   andKey:(NSString *)key
						  forVariableCode:(NSNumber *)code {
	[NSException raise:NSInternalInconsistencyException
				format:@"registerEventCallbackWithRecevier:andSelector:andKey:forVariable needs to be overridden"];	
}

- (NSNumber *)codeForTag:(NSString *)tag {
	[NSException raise:NSInternalInconsistencyException
				format:@"codeForTag: needs to be overridden"];	
	
	return nil;
}

- (void)set:(int)code to:(Data *)data {
	[NSException raise:NSInternalInconsistencyException
				format:@"set:to: needs to be overridden"];	
}

- (void)unregisterCallbacksWithKey:(NSString *)key {
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
