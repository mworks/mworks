//
//  MWorksPython.c
//  MWorksPython
//
//  Created by Christopher Stawarz on 10/31/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "MWorksPython.h"


static bool MWorksPythonInitFull(int argc, char **argv, bool initSignals) {
    PyConfig config;
    PyStatus status;
#if !TARGET_OS_OSX
    CFBundleRef bundle = NULL;  // Not owned
    CFURLRef bundleURL = NULL;
    CFURLRef pythonHomeURL = NULL;
    char pythonHomePath[1024];
#endif
    
    static bool didInitialize = false;
    
    if (didInitialize) {
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
        goto error;
    }
    status = PyConfig_SetBytesString(&config, &(config.home), pythonHomePath);
#endif
    if (PyStatus_Exception(status)) {
        goto error;
    }
    
    if (argv) {
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
    return didInitialize;
}


bool MWorksPythonInit(bool initSignals) {
    return MWorksPythonInitFull(0, NULL, initSignals);
}


int MWorksPythonMain(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file [args...]\n", argv[0]);
        return 2;
    }
    
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Can't open file: %s\n", strerror(errno));
        return 1;
    }
    
    int result = 1;
    
    if (!MWorksPythonInitFull(argc - 1, argv + 1, true)) {
        fprintf(stderr, "Python initialization failed\n");
        goto error;
    }
    
    result = PyRun_SimpleFile(fp, argv[1]);
    
    (void)Py_FinalizeEx();
    
error:
    (void)fclose(fp);
    
    return (result != 0);
}
