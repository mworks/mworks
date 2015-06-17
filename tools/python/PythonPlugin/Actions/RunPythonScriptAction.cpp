//
//  RunPythonScriptAction.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#include "RunPythonScriptAction.h"

#include "GILHelpers.h"
#include "PythonException.h"


BEGIN_NAMESPACE_MW


const std::string RunPythonScriptAction::PATH("path");


void RunPythonScriptAction::describeComponent(ComponentInfo &info) {
    RunPythonAction::describeComponent(info);
    info.setSignature("action/run_python_script");
    info.addParameter(PATH);
}


RunPythonScriptAction::RunPythonScriptAction(const ParameterValueMap &parameters) :
    RunPythonAction(parameters),
    filename(parameters[PATH])
{
    std::FILE *fp = std::fopen(filename.string().c_str(), "r");
    if (!fp) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Unable to open Python script", strerror(errno));
    }
    BOOST_SCOPE_EXIT(fp) {
        std::fclose(fp);
    } BOOST_SCOPE_EXIT_END
    
    ScopedGILAcquire sga;
    
    struct _node *node = PyParser_SimpleParseFile(fp, filename.string().c_str(), Py_file_input);
    BOOST_SCOPE_EXIT(node) {
        PyNode_Free(node);
    } BOOST_SCOPE_EXIT_END
    
    if (!node ||
        !(codeObject = PyNode_Compile(node, filename.string().c_str())))
    {
        throw PythonException("Python compilation failed");
    }
}


END_NAMESPACE_MW



























