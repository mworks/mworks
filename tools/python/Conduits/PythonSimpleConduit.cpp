//
//  PythonSimpleConduit.cpp
//  Extension Module
//
//  Created by Christopher Stawarz on 10/16/19.
//  Copyright © 2019 MWorks Project. All rights reserved.
//

#include "PythonSimpleConduit.h"


BEGIN_NAMESPACE_MW_PYTHON


template<>
PyMethodDef ExtensionType<PythonIPCConduit>::methods[] = {
    MethodDef<&PythonIPCConduit::initialize>("initialize"),
    MethodDef<&PythonIPCConduit::registerCallbackForCode>("register_callback_for_code"),
    MethodDef<&PythonIPCConduit::registerCallbackForName>("register_callback_for_name"),
    MethodDef<&PythonIPCConduit::registerLocalEventCode>("register_local_event_code"),
    MethodDef<&PythonIPCConduit::finalize>("finalize"),
    MethodDef<&PythonIPCConduit::sendData>("send_data"),
    { }  // Sentinel
};


template<>
PyGetSetDef ExtensionType<PythonIPCConduit>::getset[] = {
    GetSetDef<&PythonIPCConduit::isInitialized>("initialized"),
    GetSetDef<&PythonIPCConduit::getCodec>("codec"),
    GetSetDef<&PythonIPCConduit::getReverseCodec>("reverse_codec"),
    { }  // Sentinel
};


END_NAMESPACE_MW_PYTHON
