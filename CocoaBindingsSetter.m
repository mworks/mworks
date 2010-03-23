//
//  CocoaBindingsSetter.m
//  MWorksCocoa
//
//  Created by David Cox on 12/30/09.
//  Copyright 2009 Harvard University. All rights reserved.
//

#import "CocoaBindingsSetter.h"


@implementation CocoaBindingsSetter

- (id) initWithReceiver:(id)_receiver bindingsKey:(NSString *)_bindings_key{
    receiver = _receiver;
    bindings_key = _bindings_key;
    return self;
}

- (void)setValue:(id)value{
    [receiver setValue:value forKey:bindings_key];
}

@end
