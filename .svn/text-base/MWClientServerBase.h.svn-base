//
//  MWClientServerBase.h
//  MonkeyWorksCocoa
//
//  Created by Ben Kennedy on 9/19/07.
//  Copyright 2007 MIT. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MonkeyWorksCore/GenericData.h"
using namespace mw;

@interface MWClientServerBase : NSObject {
}

- (void)registerEventCallbackWithRecevier:(id)receiver 
							  andSelector:(SEL)selector
								   andKey:(NSString *)key;

- (void)registerEventCallbackWithRecevier:(id)receiver 
							  andSelector:(SEL)selector
								   andKey:(NSString *)key
						  forVariableCode:(NSNumber *)code;

- (void)unregisterCallbacksWithKey:(NSString *)key;
- (NSNumber *)codeForTag:(NSString *)tag;
- (void)set:(int)code to:(Data *)data;
- (NSNumber *)isLocalMessage:(NSNumber *)messageOrigin;
- (NSNumber *)maxConsoleLength;
- (NSArray *)variableNames;


@end
