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
    explicit RunPythonAction(const ParameterValueMap &parameters);
    ~RunPythonAction();
    
    bool execute() override;
    
protected:
    PyCodeObject *codeObject;
    
private:
    static PyObject *globalsDict;
    
};


END_NAMESPACE_MW


#endif /* !defined(__PythonTools__RunPythonAction__) */
