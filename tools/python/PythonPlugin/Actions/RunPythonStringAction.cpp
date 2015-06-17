//
//  RunPythonStringAction.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#include "RunPythonStringAction.h"

#include "GILHelpers.h"
#include "PythonException.h"


BEGIN_NAMESPACE_MW


const std::string RunPythonStringAction::CODE("code");


void RunPythonStringAction::describeComponent(ComponentInfo &info) {
    RunPythonAction::describeComponent(info);
    info.setSignature("action/run_python_string");
    info.addParameter(CODE);
}


RunPythonStringAction::RunPythonStringAction(const ParameterValueMap &parameters) :
    RunPythonAction(parameters)
{
    ScopedGILAcquire sga;
    
    struct _node *node = PyParser_SimpleParseString(parameters[CODE].str().c_str(), Py_file_input);
    BOOST_SCOPE_EXIT(node) {
        PyNode_Free(node);
    } BOOST_SCOPE_EXIT_END
    
    if (!node ||
        !(codeObject = PyNode_Compile(node, "<string>")))
    {
        throw PythonException("Python compilation failed");
    }
}


END_NAMESPACE_MW



























