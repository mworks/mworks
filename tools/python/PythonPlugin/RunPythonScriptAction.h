//
//  RunPythonScriptAction.h
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#ifndef __PythonTools__RunPythonScriptAction__
#define __PythonTools__RunPythonScriptAction__


BEGIN_NAMESPACE_MW


class RunPythonScriptAction : public Action, boost::noncopyable {
    
public:
    static const std::string PATH;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit RunPythonScriptAction(const ParameterValueMap &parameters);
    ~RunPythonScriptAction();
    
    bool execute() override;
    
private:
    static void initializePython();
    
    static PyObject *globalsDict;
    
    const boost::filesystem::path filename;
    PyCodeObject *codeObject;
    
};


END_NAMESPACE_MW


#endif /* !defined(__PythonTools__RunPythonScriptAction__) */
