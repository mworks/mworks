//
//  RunPythonScriptAction.h
//  PythonTools
//
//  Created by Christopher Stawarz on 6/1/15.
//  Copyright (c) 2015 MWorks Project. All rights reserved.
//

#ifndef __PythonTools__RunPythonScriptAction__
#define __PythonTools__RunPythonScriptAction__

#include "RunPythonAction.h"


BEGIN_NAMESPACE_MW


class RunPythonScriptAction : public RunPythonAction {
    
public:
    static const std::string PATH;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit RunPythonScriptAction(const ParameterValueMap &parameters);
    
private:
    const boost::filesystem::path filename;
    
};


END_NAMESPACE_MW


#endif /* !defined(__PythonTools__RunPythonScriptAction__) */
