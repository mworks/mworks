//
//  PythonException.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 6/2/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#include "PythonException.h"


BEGIN_NAMESPACE_MW


static std::string formatException(const std::string &description) {
    std::string msg(description);
    
    PyObject *type = nullptr;
    PyObject *value = nullptr;
    PyObject *traceback = nullptr;
    BOOST_SCOPE_EXIT(&type, &value, &traceback) {
        Py_XDECREF(type);
        Py_XDECREF(value);
        Py_XDECREF(traceback);
    } BOOST_SCOPE_EXIT_END
    
    PyErr_Fetch(&type, &value, &traceback);
    PyErr_NormalizeException(&type, &value, &traceback);
    
    if (type && value) {
        if (!traceback) {
            Py_INCREF(Py_None);
            traceback = Py_None;
        }
        
        PyObject *tracebackModule = nullptr;
        PyObject *formattedExceptionLines = nullptr;
        BOOST_SCOPE_EXIT(&tracebackModule, &formattedExceptionLines) {
            Py_XDECREF(tracebackModule);
            Py_XDECREF(formattedExceptionLines);
        } BOOST_SCOPE_EXIT_END
        
        if ((tracebackModule = PyImport_ImportModule("traceback")) &&
            (formattedExceptionLines = PyObject_CallMethod(tracebackModule,
                                                           const_cast<char *>("format_exception"),
                                                           const_cast<char *>("OOO"),
                                                           type,
                                                           value,
                                                           traceback)) &&
            PyList_Check(formattedExceptionLines))
        {
            msg.append(": ");
            for (Py_ssize_t i = 0; i < PyList_GET_SIZE(formattedExceptionLines); i++) {
                const char *str;
                if ((str = PyString_AsString(PyList_GET_ITEM(formattedExceptionLines, i)))) {
                    msg.append(str);
                } else {
                    PyErr_Clear();
                    break;
                }
            }
        } else {
            // Ignore exceptions raised while attempting to format the exception
            PyErr_Clear();
        }
    }
    
    return std::move(msg);
}


void PythonException::logError(const std::string &message) {
    merror(M_PLUGIN_MESSAGE_DOMAIN, "%s", formatException(message).c_str());
}


PythonException::PythonException(const std::string &message) :
    SimpleException(M_PLUGIN_MESSAGE_DOMAIN, formatException(message))
{ }


END_NAMESPACE_MW




























