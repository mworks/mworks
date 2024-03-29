//
//  MWorksPython.c
//  MWorksPython
//
//  Created by Christopher Stawarz on 10/31/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "MWorksPython.h"


static CFBundleRef getBundle(void) {
    return CFBundleGetBundleWithIdentifier(CFSTR(PRODUCT_BUNDLE_IDENTIFIER));  // Not owned
}


static PyObject * get_executable_path(PyObject *self, PyObject *args) {
    CFBundleRef bundle = NULL;
    CFURLRef executableURL = NULL;
    char executablePath[1024];
    PyObject *result = NULL;
    
    if (!(bundle = getBundle()) ||
        !(executableURL = CFBundleCopyExecutableURL(bundle)) ||
        !CFURLGetFileSystemRepresentation(executableURL, true, (UInt8 *)executablePath, sizeof(executablePath)))
    {
        PyErr_SetString(PyExc_RuntimeError, "Cannot obtain path to MWorksPython framework executable");
        goto error;
    }
    
    result = PyUnicode_FromString(executablePath);
    
error:
    if (executableURL) CFRelease(executableURL);
    
    return result;
}


static PyObject * have_init_func(PyObject *self, PyObject *args) {
    char *name = NULL;
    CFBundleRef bundle = NULL;
    CFStringRef functionName = NULL;
    PyObject *result = NULL;
    
    if (!PyArg_ParseTuple(args, "s", &name)) {
        goto error;
    }
    
    if (!(bundle = getBundle()) ||
        !(functionName = CFStringCreateWithCString(kCFAllocatorDefault, name, kCFStringEncodingUTF8)) ||
        !CFBundleGetFunctionPointerForName(bundle, functionName))
    {
        result = Py_False;
    } else {
        result = Py_True;
    }
    
    Py_INCREF(result);
    
error:
    if (functionName) CFRelease(functionName);
    
    return result;
}


static PyMethodDef _mworkspythonMethods[] = {
    { "get_executable_path", get_executable_path, METH_NOARGS, NULL },
    { "have_init_func", have_init_func, METH_VARARGS, NULL },
    { }  // Sentinel
};


static PyModuleDef _mworkspythonModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_mworkspython",
    .m_size = -1,
    .m_methods = _mworkspythonMethods
};


static PyObject * init__mworkspython(void) {
    return PyModule_Create(&_mworkspythonModule);
}


static bool MWorksPythonInitFull(int argc, char **argv, bool initSignals) {
    PyPreConfig preConfig;
    PyStatus status;
    PyConfig config;
    CFBundleRef bundle = NULL;
    CFURLRef zipURL = NULL;
    char zipPath[1024];
    wchar_t *decodedZipPath = NULL;
    PyObject *mworkspythonModule = NULL;
    
    // Changes made by PyImport_AppendInittab aren't undone by Py_FinalizeEx, so we keep track of them
    // separately
    static bool didAppendInittab = false;
    static bool didInitialize = false;
    
    if (didInitialize) {
        goto exit;
    }
    
    if (!didAppendInittab) {
        if (-1 == PyImport_AppendInittab(_mworkspythonModule.m_name, &init__mworkspython)) {
            goto exit;
        }
        didAppendInittab = true;
    }
    
    PyPreConfig_InitIsolatedConfig(&preConfig);
    preConfig.utf8_mode = 1;
    status = Py_PreInitialize(&preConfig);
    if (PyStatus_Exception(status)) {
        goto exit;
    }
    
    PyConfig_InitIsolatedConfig(&config);
    
    if (!(bundle = getBundle()) ||
        !(zipURL = CFBundleCopyResourceURL(bundle, CFSTR("python"), CFSTR("zip"), NULL)) ||
        !CFURLGetFileSystemRepresentation(zipURL, true, (UInt8 *)zipPath, sizeof(zipPath)) ||
        !(decodedZipPath = Py_DecodeLocale(zipPath, NULL)))
    {
        goto error;
    }
    
    config.module_search_paths_set = 1;
    status = PyConfig_SetWideStringList(&config, &(config.module_search_paths), 1, &decodedZipPath);
    if (PyStatus_Exception(status)) {
        goto error;
    }
    
    config.site_import = 0;  // Don't import the "site" module
    config.install_signal_handlers = initSignals;
    
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
    
    if (!(mworkspythonModule = PyImport_ImportModule("mworkspython"))) {
        (void)Py_FinalizeEx();  // Finalize so that caller can try again
        goto error;
    }
    
    didInitialize = true;
    
error:
    Py_XDECREF(mworkspythonModule);
    PyMem_RawFree(decodedZipPath);
    if (zipURL) CFRelease(zipURL);
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
