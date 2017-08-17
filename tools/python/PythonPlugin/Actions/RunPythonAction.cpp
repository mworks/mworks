//
//  RunPythonAction.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#include "RunPythonAction.h"

#include "GILHelpers.h"
#include "PythonException.h"
#include "PythonSetup.hpp"


BEGIN_NAMESPACE_MW


RunPythonAction::RunPythonAction(const ParameterValueMap &parameters) :
    Action(parameters),
    codeObject(nullptr),
    globalsDict(python::getGlobalsDict())
{ }


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



























