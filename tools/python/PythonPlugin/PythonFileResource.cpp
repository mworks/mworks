//
//  PythonFileResource.cpp
//  PythonPlugin
//
//  Created by Christopher Stawarz on 10/18/17.
//  Copyright © 2017 MWorks Project. All rights reserved.
//

#include "PythonFileResource.hpp"

#include "PythonEvaluator.hpp"


BEGIN_NAMESPACE_MW


const std::string PythonFileResource::PATH("path");


void PythonFileResource::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    info.setSignature("resource/python_file");
    info.addParameter(PATH);
}


PythonFileResource::PythonFileResource(const ParameterValueMap &parameters) :
    Component(parameters),
    path(parameters[PATH])
{
    PythonEvaluator evaluator(path);
    if (!(evaluator.eval())) {
        throw SimpleException(M_PLUGIN_MESSAGE_DOMAIN, "Python file execution failed");
    }
}


END_NAMESPACE_MW



















