//
//  MWorksPython.c
//  MWorksPython
//
//  Created by Christopher Stawarz on 10/31/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "MWorksPython.h"


static PyStatus MWorksPythonInitFull(int argc, char **argv, bool initSignals) {
    PyStatus status;
    PyConfig config;
#if !TARGET_OS_OSX
    CFBundleRef bundle = NULL;  // Not owned
    CFURLRef bundleURL = NULL;
    CFURLRef pythonHomeURL = NULL;
    char pythonHomePath[1024];
#endif
    
    static bool didInitialize = false;
    if (didInitialize) {
        status = PyStatus_Ok();
        goto exit;
    }
    
    PyConfig_InitIsolatedConfig(&config);
    
    config.install_signal_handlers = initSignals;
#if TARGET_OS_OSX
    // If we leave isolated set to 1, Py_InitializeFromConfig will set
    // user_site_directory to 0
    config.isolated = 0;
    config.user_site_directory = 1;
#endif
    
#if TARGET_OS_OSX
    status = PyConfig_SetBytesString(&config, &(config.home), MW_PYTHON_3_PREFIX);
#else
    if (!(bundle = CFBundleGetMainBundle()) ||
        !(bundleURL = CFBundleCopyBundleURL(bundle)) ||
        !(pythonHomeURL = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, bundleURL, CFSTR("python"), true)) ||
        !CFURLGetFileSystemRepresentation(pythonHomeURL, true, (UInt8 *)pythonHomePath, sizeof(pythonHomePath)))
    {
        status = PyStatus_Error("Cannot determine path to Python home directory");
        goto error;
    }
    status = PyConfig_SetBytesString(&config, &(config.home), pythonHomePath);
#endif
    if (PyStatus_Exception(status)) {
        goto error;
    }
    
    if (argv) {
        config.parse_argv = 1;
        status = PyConfig_SetBytesArgv(&config, argc, argv);
        if (PyStatus_Exception(status)) {
            goto error;
        }
    }
    
    status = Py_InitializeFromConfig(&config);  // Creates and acquires the GIL
    if (PyStatus_Exception(status)) {
        goto error;
    }
    
    didInitialize = true;
    
error:
#if !TARGET_OS_OSX
    if (pythonHomeURL) CFRelease(pythonHomeURL);
    if (bundleURL) CFRelease(bundleURL);
#endif
    PyConfig_Clear(&config);
    
exit:
    return status;
}


bool MWorksPythonInit(bool initSignals) {
    return !PyStatus_Exception(MWorksPythonInitFull(0, NULL, initSignals));
}


int MWorksPythonMain(int argc, char **argv) {
    PyStatus status = MWorksPythonInitFull(argc, argv, true);
    if (PyStatus_Exception(status)) {
        Py_ExitStatusException(status);  // Calls exit()
    }
    return Py_RunMain();
}
