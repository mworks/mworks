//
//  RunPythonFileAction.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#include "RunPythonFileAction.h"

#include "GILHelpers.h"
#include "PythonException.h"


BEGIN_NAMESPACE_MW


const std::string RunPythonFileAction::PATH("path");


void RunPythonFileAction::describeComponent(ComponentInfo &info) {
    RunPythonAction::describeComponent(info);
    info.setSignature("action/run_python_file");
    info.addParameter(PATH);
}


RunPythonFileAction::RunPythonFileAction(const ParameterValueMap &parameters) :
    RunPythonAction(parameters)
{
    // Converting to boost::filesystem::path first buys us some useful path expansion and validation
    const std::string filename(boost::filesystem::path(parameters[PATH]).string());
    
    std::FILE *fp = std::fopen(filename.c_str(), "r");
    if (!fp) {
        throw SimpleException(M_FILE_MESSAGE_DOMAIN, "Unable to open Python file", strerror(errno));
    }
    BOOST_SCOPE_EXIT(fp) {
        std::fclose(fp);
    } BOOST_SCOPE_EXIT_END
    
    ScopedGILAcquire sga;
    
    struct _node *node = PyParser_SimpleParseFile(fp, filename.c_str(), Py_file_input);
    BOOST_SCOPE_EXIT(node) {
        PyNode_Free(node);
    } BOOST_SCOPE_EXIT_END
    
    if (!node ||
        !(codeObject = PyNode_Compile(node, filename.c_str())))
    {
        throw PythonException("Python compilation failed");
    }
}


END_NAMESPACE_MW



























