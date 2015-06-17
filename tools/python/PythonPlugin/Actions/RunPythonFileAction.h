//
//  RunPythonFileAction.h
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#ifndef __PythonTools__RunPythonFileAction__
#define __PythonTools__RunPythonFileAction__

#include "RunPythonAction.h"


BEGIN_NAMESPACE_MW


class RunPythonFileAction : public RunPythonAction {
    
public:
    static const std::string PATH;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit RunPythonFileAction(const ParameterValueMap &parameters);
    
};


END_NAMESPACE_MW


#endif /* !defined(__PythonTools__RunPythonFileAction__) */
