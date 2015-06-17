//
//  RunPythonStringAction.h
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#ifndef __PythonTools__RunPythonStringAction__
#define __PythonTools__RunPythonStringAction__

#include "RunPythonAction.h"


BEGIN_NAMESPACE_MW


class RunPythonStringAction : public RunPythonAction {
    
public:
    static const std::string CODE;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit RunPythonStringAction(const ParameterValueMap &parameters);
    
};


END_NAMESPACE_MW


#endif /* !defined(__PythonTools__RunPythonStringAction__) */
