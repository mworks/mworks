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


const std::string RunPythonAction::STOP_ON_FAILURE("stop_on_failure");


void RunPythonAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.addParameter(STOP_ON_FAILURE, "NO");
}


RunPythonAction::RunPythonAction(const ParameterValueMap &parameters) :
    Action(parameters),
    codeObject(nullptr),
    globalsDict(python::getGlobalsDict()),
    stopOnFailure(parameters[STOP_ON_FAILURE])
{ }


RunPythonAction::~RunPythonAction() {
    ScopedGILAcquire sga;
    Py_XDECREF(codeObject);
}


bool RunPythonAction::execute() {
    ScopedGILAcquire sga;
    
    assert(codeObject);  // codeObject must be created by subclasses
    
    PyObject *result = PyEval_EvalCode(codeObject, globalsDict, globalsDict);
    if (result) {
        Py_DECREF(result);
    } else {
        PythonException::logError("Python execution failed");
        if (stopOnFailure) {
            merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Stopping experiment due to failed Python execution");
            StateSystem::instance()->stop();
        }
    }
    
    return true;
}


END_NAMESPACE_MW



























