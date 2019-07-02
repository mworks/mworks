//
//  RunPythonStringAction.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#include "RunPythonStringAction.h"


BEGIN_NAMESPACE_MW_PYTHON


const std::string RunPythonStringAction::CODE("code");


void RunPythonStringAction::describeComponent(ComponentInfo &info) {
    RunPythonAction::describeComponent(info);
    info.setSignature("action/run_python_string");
    info.addParameter(CODE);
}


RunPythonStringAction::RunPythonStringAction(const ParameterValueMap &parameters) :
    RunPythonAction(parameters, parameters[CODE].str())
{ }


END_NAMESPACE_MW_PYTHON



























