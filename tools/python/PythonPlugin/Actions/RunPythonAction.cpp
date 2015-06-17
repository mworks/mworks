//
//  RunPythonAction.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#include "RunPythonAction.h"

#include <numpy/arrayobject.h>

#include "GILHelpers.h"
#include "PythonDataHelpers.h"
#include "PythonException.h"


BEGIN_NAMESPACE_MW


static PyObject * getvar(PyObject *self, PyObject *args) {
    const char *name;
    if (!PyArg_ParseTuple(args, "s:getvar", &name)) {
        return nullptr;
    }
    
    try {
        
        boost::shared_ptr<Variable> var = global_variable_registry->getVariable(name);
        if (!var) {
            throw UnknownVariableException(name);
        }
        
        boost::python::object value = convert_datum_to_python(var->getValue());
        
        PyObject *result = value.ptr();
        Py_INCREF(result);
        return result;
        
    } catch (const boost::python::error_already_set &) {
        // Python error is already set
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "An unknown error occurred");
    }
    
    return nullptr;
}


static PyObject * setvar(PyObject *self, PyObject *args) {
    const char *name;
    PyObject *value;
    if (!PyArg_ParseTuple(args, "sO:setvar", &name, &value)) {
        return nullptr;
    }
    
    try {
        
        boost::shared_ptr<Variable> var = global_variable_registry->getVariable(name);
        if (!var) {
            throw UnknownVariableException(name);
        }
        
        Datum val = convert_python_to_datum(manageBorrowedRef(value));
        {
            // setValue can perform an arbitrary number of notifications, some of which
            // may trigger blocking I/O operations, so temporarily release the GIL
            ScopedGILRelease sgr;
            var->setValue(val);
        }
        
        Py_RETURN_NONE;
        
    } catch (const boost::python::error_already_set &) {
        // Python error is already set
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "An unknown error occurred");
    }
    
    return nullptr;
}


static PyMethodDef methods[] = {
    { "getvar", getvar, METH_VARARGS, nullptr },
    { "setvar", setvar, METH_VARARGS, nullptr },
    {NULL, NULL, 0, NULL}
};


PyObject * RunPythonAction::globalsDict = nullptr;


RunPythonAction::RunPythonAction(const ParameterValueMap &parameters) :
    Action(parameters),
    codeObject(nullptr)
{
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
        import_array();
        if (PyErr_Occurred()) {
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
        Py_InitModule("mworkscore", methods);
        PyObject *result = PyRun_String("from mworkscore import *", Py_single_input, globalsDict, globalsDict);
        if (!result) {
            throw PythonException("Unable to import mworkscore methods into Python __main__ module");
        }
        Py_DECREF(result);
    });
}


RunPythonAction::~RunPythonAction() {
    ScopedGILAcquire sga;
    Py_XDECREF(codeObject);
}


bool RunPythonAction::execute() {
    ScopedGILAcquire sga;
    
    assert(codeObject);  // codeObject must be created by subclasses
    
    PyObject *result = PyEval_EvalCode(codeObject, globalsDict, globalsDict);
    if (!result) {
        PythonException::logError("Python execution failed");
        return false;
    }
    Py_DECREF(result);
    
    return true;
}


END_NAMESPACE_MW



























