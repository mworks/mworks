//
//  main.m
//  PythonTests-iOS
//
//  Created by Christopher Stawarz on 8/8/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#import <UIKit/UIKit.h>

#import "AppDelegate.h"


int main(int argc, char * argv[]) {
    NSString *home = @(argv[0]).stringByDeletingLastPathComponent;
    wchar_t *decodedHome = Py_DecodeLocale(home.UTF8String, NULL);
    if (!decodedHome) {
        NSLog(@"Decoding failed");
        return 1;
    }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    Py_SetPythonHome(decodedHome);
#pragma clang diagnostic pop
    (void)Py_BytesMain(argc, argv);
    PyMem_RawFree(decodedHome);
    
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
