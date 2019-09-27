//
//  PythonEvent.cpp
//  PythonTools
//
//  Created by Christopher Stawarz on 10/15/19.
//  Copyright © 2019 MWorks Project. All rights reserved.
//

#include "PythonEvent.hpp"


BEGIN_NAMESPACE_MW_PYTHON


template<>
PyGetSetDef ExtensionType<PythonEvent>::getset[] = {
    GetSetDef<&PythonEvent::getCode>("code"),
    GetSetDef<&PythonEvent::getTime>("time"),
    GetSetDef<&PythonEvent::getData>("data"),
    GetSetDef<&PythonEvent::getData>("value"),  // For backwards compatibility
    { }  // Sentinel
};


END_NAMESPACE_MW_PYTHON
