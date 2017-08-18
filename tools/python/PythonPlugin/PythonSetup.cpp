//
//  PythonSetup.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 8/17/17.
//  Copyright © 2017 MWorks Project. All rights reserved.
//

#include "PythonSetup.hpp"

#include <numpy/arrayobject.h>

#include "GILHelpers.h"
#include "PythonDataHelpers.h"
#include "PythonException.h"


BEGIN_NAMESPACE_MW
BEGIN_NAMESPACE(python)


BEGIN_NAMESPACE()


inline boost::shared_ptr<RegistryAwareEventStreamInterface> getCore() {
    return Server::instance();
}


boost::python::object getcodec() {
    auto core = getCore();
    boost::python::dict codec;
    for (auto &name : core->getVariableTagNames()) {
        auto code = core->lookupCodeForTag(name);
        if (code >= 0) {
            codec[code] = name;
        }
    }
    return codec;
}


boost::python::object getvar(const std::string &name) {
    auto var = getCore()->getVariable(name);
    if (!var) {
        throw UnknownVariableException(name);
    }
    return convert_datum_to_python(var->getValue());
}


void setvar(const std::string &name, const boost::python::object &value) {
    auto var = getCore()->getVariable(name);
    if (!var) {
        throw UnknownVariableException(name);
    }
    Datum val = convert_python_to_datum(value);
    {
        // setValue can perform an arbitrary number of notifications, some of which
        // may trigger blocking I/O operations, so temporarily release the GIL
        ScopedGILRelease sgr;
        var->setValue(val);
    }
}


void init_mworkscore() {
    auto module = manageBorrowedRef( Py_InitModule("mworkscore", nullptr) );
    boost::python::scope moduleScope(module);
    
    def("getcodec", getcodec);
    def("getvar", getvar);
    def("setvar", setvar);
}


END_NAMESPACE()


PyObject * getGlobalsDict() {
    static PyObject *globalsDict = nullptr;
    
    //
    // Perform one-time initialization of Python environment
    //
    static std::once_flag initFlag;
    std::call_once(initFlag, []() {
        //
        // Initialize Python interpreter
        //
        Py_InitializeEx(0);
        PyEval_InitThreads();
        BOOST_SCOPE_EXIT(void) {
            // PyEval_InitThreads acquires the GIL, so we must release it on exit
            PyEval_ReleaseThread(PyThreadState_Get());
        } BOOST_SCOPE_EXIT_END
        
        //
        // Import NumPy C API
        //
        if (_import_array() < 0) {
            throw PythonException("Unable to import NumPy C API");
        }
        
        //
        // Obtain __main__ module's dict and store a reference in globalsDict
        //
        PyObject *mainModule = PyImport_AddModule("__main__");  // Borrowed ref
        if (!mainModule) {
            throw PythonException("Unable to import Python __main__ module");
        }
        globalsDict = PyModule_GetDict(mainModule);  // Borrowed ref, never fails
        Py_INCREF(globalsDict);  // Upgrade to owned ref
        
        //
        // Create mworkscore module and import its functions into __main__
        //
        init_mworkscore();
        PyObject *result = PyRun_String("from mworkscore import *", Py_single_input, globalsDict, globalsDict);
        if (!result) {
            throw PythonException("Unable to import mworkscore methods into Python __main__ module");
        }
        Py_DECREF(result);
    });
    
    return globalsDict;
}


END_NAMESPACE(python)
END_NAMESPACE_MW


























