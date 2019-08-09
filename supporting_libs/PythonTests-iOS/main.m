//
//  main.m
//  PythonTests-iOS
//
//  Created by Christopher Stawarz on 8/8/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#import <UIKit/UIKit.h>

#define Py_BUILD_CORE
#include <Python.h>

#import "AppDelegate.h"


int main(int argc, char * argv[]) {
    NSString *home = @(argv[0]).stringByDeletingLastPathComponent;
    wchar_t *decodedHome = Py_DecodeLocale(home.UTF8String, NULL);
    if (!decodedHome) {
        NSLog(@"Decoding failed");
        return 1;
    }
    Py_SetPythonHome(decodedHome);
    (void)_Py_UnixMain(argc, argv);
    PyMem_RawFree(decodedHome);
    
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
