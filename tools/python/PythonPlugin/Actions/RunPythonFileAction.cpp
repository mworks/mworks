//
//  RunPythonFileAction.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#include "RunPythonFileAction.h"


BEGIN_NAMESPACE_MW_PYTHON


const std::string RunPythonFileAction::PATH("path");


void RunPythonFileAction::describeComponent(ComponentInfo &info) {
    RunPythonAction::describeComponent(info);
    info.setSignature("action/run_python_file");
    info.addParameter(PATH);
}


RunPythonFileAction::RunPythonFileAction(const ParameterValueMap &parameters) :
    // Converting to boost::filesystem::path buys us some useful path expansion and validation
    RunPythonAction(parameters, std::make_unique<PythonFileEvaluator>(pathFromParameterValue(variableOrText(parameters[PATH]))))
{ }


END_NAMESPACE_MW_PYTHON
