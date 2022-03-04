//
//  MWEyeWindowOptionController.h
//  MWorksEyeWindow
//
//  Created by labuser on 11/1/06.
//  Copyright 2006 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface MWEyeWindowOptionController : NSObject

@property(nonatomic, strong) NSNumber *timeOfTail;

@property(nonatomic, copy) NSString *h;
@property(nonatomic, copy) NSString *v;
@property(nonatomic, copy) NSString *eyeState;

@property(nonatomic, copy) NSString *auxH;
@property(nonatomic, copy) NSString *auxV;

@property(nonatomic, copy) NSString *a;
@property(nonatomic, copy) NSString *b;

@end
