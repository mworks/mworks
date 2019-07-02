//
//  RunPythonAction.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#include "RunPythonAction.h"


BEGIN_NAMESPACE_MW_PYTHON


const std::string RunPythonAction::STOP_ON_FAILURE("stop_on_failure");


void RunPythonAction::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.addParameter(STOP_ON_FAILURE, "NO");
}


RunPythonAction::RunPythonAction(const ParameterValueMap &parameters, const boost::filesystem::path &filePath) :
    Action(parameters),
    evaluator(filePath),
    stopOnFailure(parameters[STOP_ON_FAILURE])
{ }


RunPythonAction::RunPythonAction(const ParameterValueMap &parameters, const std::string &code) :
    Action(parameters),
    evaluator(code),
    stopOnFailure(parameters[STOP_ON_FAILURE])
{ }


bool RunPythonAction::execute() {
    if (!(evaluator.exec())) {
        if (stopOnFailure) {
            merror(M_STATE_SYSTEM_MESSAGE_DOMAIN, "Stopping experiment due to failed Python execution");
            StateSystem::instance()->stop();
        }
    }
    
    return true;
}


END_NAMESPACE_MW_PYTHON



























