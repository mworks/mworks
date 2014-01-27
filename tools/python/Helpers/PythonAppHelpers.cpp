//
//  PythonAppHelpers.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 1/24/14.
//  Copyright (c) 2014 MWorks Project. All rights reserved.
//

#include "PythonAppHelpers.h"

#import <AppKit/AppKit.h>

#include "GILHelpers.h"


BEGIN_NAMESPACE_MW


void python_start_main_loop() {
    ScopedGILRelease sgr;
    
    @autoreleasepool {
        for (NSWindow *window in [[NSApplication sharedApplication] windows]) {
            [window orderFront:nil];
        }
    }
    
    [[NSApplication sharedApplication] run];
}


void python_stop_main_loop() {
    dispatch_async(dispatch_get_main_queue(), ^{
        [[NSApplication sharedApplication] stop:nil];
        // Per the docs, "stop:" "sets a flag that the application checks only after it finishes
        // dispatching an actual event object", so we send a dummy event to ensure that the main
        // loop stops ASAP
        [[NSApplication sharedApplication] postEvent:[NSEvent otherEventWithType:NSApplicationDefined
                                                                        location:NSZeroPoint
                                                                   modifierFlags:0
                                                                       timestamp:0.0
                                                                    windowNumber:0
                                                                         context:nil
                                                                         subtype:0
                                                                           data1:0
                                                                           data2:0]
                                             atStart:YES];
    });
}


END_NAMESPACE_MW


























