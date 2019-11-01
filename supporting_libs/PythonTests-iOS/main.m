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
    Py_SetPythonHome(decodedHome);
    (void)Py_BytesMain(argc, argv);
    PyMem_RawFree(decodedHome);
    
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
