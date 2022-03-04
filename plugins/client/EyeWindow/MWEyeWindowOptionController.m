//
//  MWEyeWindowOptionController.m
//  MWorksEyeWindow
//
//  Created by labuser on 11/1/06.
//  Copyright 2006 __MyCompanyName__. All rights reserved.
//

#import "MWEyeWindowOptionController.h"
#import "MWEyeWindowController.h"

#define MW_EYE_WINDOW_PREF_PREFIX               @"MWorksClient - Eye Window - "

#define MW_EYE_WINDOW_TIME_OF_TAIL              MW_EYE_WINDOW_PREF_PREFIX "time_of_tail"

#define MW_EYE_WINDOW_H_NAME                    MW_EYE_WINDOW_PREF_PREFIX "h"
#define MW_EYE_WINDOW_V_NAME                    MW_EYE_WINDOW_PREF_PREFIX "v"
#define MW_EYE_WINDOW_EYE_STATE_NAME            MW_EYE_WINDOW_PREF_PREFIX "eye_state"

#define MW_EYE_WINDOW_AUX_H_NAME                MW_EYE_WINDOW_PREF_PREFIX "aux_h"
#define MW_EYE_WINDOW_AUX_V_NAME                MW_EYE_WINDOW_PREF_PREFIX "aux_v"

#define MW_EYE_WINDOW_A_NAME                    MW_EYE_WINDOW_PREF_PREFIX "a"
#define MW_EYE_WINDOW_B_NAME                    MW_EYE_WINDOW_PREF_PREFIX "b"


@implementation MWEyeWindowOptionController


- (instancetype)init {
    self = [super init];
    
    if (self) {
        NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
        
        _timeOfTail = [NSNumber numberWithFloat:[ud floatForKey:MW_EYE_WINDOW_TIME_OF_TAIL]];
        
        if (nil == (_h = [ud stringForKey:MW_EYE_WINDOW_H_NAME])) {
            _h = @"";
        }
        if (nil == (_v = [ud stringForKey:MW_EYE_WINDOW_V_NAME])) {
            _v = @"";
        }
        if (nil == (_eyeState = [ud stringForKey:MW_EYE_WINDOW_EYE_STATE_NAME])) {
            _eyeState = @"";
        }
        
        if (nil == (_auxH = [ud stringForKey:MW_EYE_WINDOW_AUX_H_NAME])) {
            _auxH = @"";
        }
        if (nil == (_auxV = [ud stringForKey:MW_EYE_WINDOW_AUX_V_NAME])) {
            _auxV = @"";
        }
        
        if (nil == (_a = [ud stringForKey:MW_EYE_WINDOW_A_NAME])) {
            _a = @"";
        }
        if (nil == (_b = [ud stringForKey:MW_EYE_WINDOW_B_NAME])) {
            _b = @"";
        }
    }
    
    return self;
}	


- (void)setTimeOfTail:(NSNumber *)timeOfTail {
	_timeOfTail = timeOfTail;
    [self updateVariables];
}


- (void)setH:(NSString *)h {
    _h = (h ? [h copy] : @"");
    [self updateVariables];
}


- (void)setV:(NSString *)v {
    _v = (v ? [v copy] : @"");
    [self updateVariables];
}


- (void)setEyeState:(NSString *)eyeState {
    _eyeState = (eyeState ? [eyeState copy] : @"");
    [self updateVariables];
}


- (void)setAuxH:(NSString *)auxH {
    _auxH = (auxH ? [auxH copy] : @"");
    [self updateVariables];
}


- (void)setAuxV:(NSString *)auxV {
    _auxV = (auxV ? [auxV copy] : @"");
    [self updateVariables];
}


- (void)setA:(NSString *)a {
    _a = (a ? [a copy] : @"");
    [self updateVariables];
}


- (void)setB:(NSString *)b {
    _b = (b ? [b copy] : @"");
    [self updateVariables];
}


- (void)updateVariables {
    [[NSNotificationCenter defaultCenter] postNotificationName:MWEyeWindowVariableUpdateNotification
                                                        object:nil
                                                      userInfo:nil];
    
    NSUserDefaults *ud = [NSUserDefaults standardUserDefaults];
    
    [ud setFloat:[self.timeOfTail floatValue] forKey:MW_EYE_WINDOW_TIME_OF_TAIL];
    
    [ud setObject:self.h forKey:MW_EYE_WINDOW_H_NAME];
    [ud setObject:self.v forKey:MW_EYE_WINDOW_V_NAME];
    [ud setObject:self.eyeState forKey:MW_EYE_WINDOW_EYE_STATE_NAME];
    
    [ud setObject:self.auxH forKey:MW_EYE_WINDOW_AUX_H_NAME];
    [ud setObject:self.auxV forKey:MW_EYE_WINDOW_AUX_V_NAME];
    
    [ud setObject:self.a forKey:MW_EYE_WINDOW_A_NAME];
    [ud setObject:self.b forKey:MW_EYE_WINDOW_B_NAME];
}


//
// TODO: Force-end editing when options drawer closes
//
/*
- (void)closeSheet {
    // Finish editing in all fields
    if (![[self window] makeFirstResponder:[self window]]) {
        [[self window] endEditingFor:nil];
    }
    [[self window] orderOut:self];
    [NSApp endSheet:[self window]];
}
 */


@end





















