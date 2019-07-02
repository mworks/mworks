//
//  RunPythonAction.h
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#ifndef __PythonTools__RunPythonAction__
#define __PythonTools__RunPythonAction__

#include "PythonEvaluator.hpp"


BEGIN_NAMESPACE_MW_PYTHON


class RunPythonAction : public Action {
    
public:
    static const std::string STOP_ON_FAILURE;
    
    static void describeComponent(ComponentInfo &info);
    
    RunPythonAction(const ParameterValueMap &parameters, const boost::filesystem::path &filePath);
    RunPythonAction(const ParameterValueMap &parameters, const std::string &code);
    
    bool execute() override;
    
private:
    PythonEvaluator evaluator;
    const bool stopOnFailure;
    
};


END_NAMESPACE_MW_PYTHON


#endif /* !defined(__PythonTools__RunPythonAction__) */



















