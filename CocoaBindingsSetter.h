//
//  CocoaBindingsSetter.h
//  MonkeyWorksCocoa
//
//  Created by David Cox on 12/30/09.
//  Copyright 2009 Harvard University. All rights reserved.
//
//  Had to create this because Cocoa sucks donkey balls sometimes.  
//  Basically, the problem is this: Cocoa is not thread safe; neither are bindings
//  There is performSelectorOnMainThread, but this only takes single arguments, and they
//  must be Cocoa objects.  This all adds up to a whole lot of ugly bending over backwards
//  WTF?

#import <Cocoa/Cocoa.h>


@interface CocoaBindingsSetter : NSObject {

    id receiver;
    NSString *bindings_key;
}

- (id) initWithReceiver:(id)receiver bindingsKey:(NSString *)bindings_key;
- (void)setValue:(id)value;

@end
