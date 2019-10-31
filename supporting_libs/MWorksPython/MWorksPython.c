//
//  MWorksPython.c
//  MWorksPython
//
//  Created by Christopher Stawarz on 10/31/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "MWorksPython.h"


static CFBundleRef getBundle() {
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


static PyObject * init__mworkspython() {
    return PyModule_Create(&_mworkspythonModule);
}


bool MWorksPythonInit(bool initSignals) {
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
    
    if (!(bundle = getBundle()) ||
        !(zipURL = CFBundleCopyResourceURL(bundle, CFSTR("python"), CFSTR("zip"), NULL)) ||
        !CFURLGetFileSystemRepresentation(zipURL, true, (UInt8 *)zipPath, sizeof(zipPath)) ||
        !(decodedZipPath = Py_DecodeLocale(zipPath, NULL)))
    {
        goto error;
    }
    
    Py_SetPath(decodedZipPath);
    
    Py_NoSiteFlag = 1;  // Don't import the "site" module
    Py_InitializeEx(initSignals);  // Calls PyEval_InitThreads and thereby acquires the GIL
    
    if (!(mworkspythonModule = PyImport_ImportModule("mworkspython"))) {
        (void)Py_FinalizeEx();  // Finalize so that caller can try again
        goto error;
    }
    
    didInitialize = true;
    
error:
    Py_XDECREF(mworkspythonModule);
    PyMem_RawFree(decodedZipPath);
    if (zipURL) CFRelease(zipURL);
    
exit:
    return didInitialize;
}
