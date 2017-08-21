//
//  RunPythonAction.h
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#ifndef __PythonTools__RunPythonAction__
#define __PythonTools__RunPythonAction__


BEGIN_NAMESPACE_MW


class RunPythonAction : public Action, boost::noncopyable {
    
public:
    static const std::string STOP_ON_FAILURE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit RunPythonAction(const ParameterValueMap &parameters);
    ~RunPythonAction();
    
    bool execute() override;
    
protected:
    PyCodeObject *codeObject;
    
private:
    PyObject * const globalsDict;
    const bool stopOnFailure;
    
};


END_NAMESPACE_MW


#endif /* !defined(__PythonTools__RunPythonAction__) */
