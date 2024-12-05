/*
 *  PythonTools.h
 *
 *  Created by David Cox on 10/20/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#include "PythonEvent.hpp"
#include "PythonSimpleConduit.h"
#include "PythonDataBindingsHelpers.h"


BEGIN_NAMESPACE_MW_PYTHON


void PythonEvent::Callback::handleCallbackError() {
    PyErr_Print();
}


static PyModuleDef _mworksModule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "_mworks",
    .m_size = -1,
    .m_methods = nullptr
};


PyMODINIT_FUNC
PyInit__mworks() {
    if (!initializeScarab()) {
        PyErr_SetString(PyExc_RuntimeError, "Scarab initialization failed");
        return nullptr;
    }
    
    auto mod = ObjectPtr::owned(PyModule_Create(&_mworksModule));
    if (!mod) {
        return nullptr;
    }
    
    importNumpyTypes();
    
    if (PyModule_AddIntMacro(mod.get(), RESERVED_CODEC_CODE) ||
        PyModule_AddIntMacro(mod.get(), RESERVED_SYSTEM_EVENT_CODE) ||
        PyModule_AddIntMacro(mod.get(), RESERVED_COMPONENT_CODEC_CODE) ||
        PyModule_AddIntMacro(mod.get(), RESERVED_TERMINATION_CODE) ||
        PyModule_AddIntConstant(mod.get(), "server_event_transport", EventTransport::server_event_transport) ||
        PyModule_AddIntConstant(mod.get(), "client_event_transport", EventTransport::client_event_transport))
    {
        return nullptr;
    }
    
    if (!PythonEvent::createType("_mworks.Event", mod) ||
        !PythonIPCConduit::createType("_mworks._IPCConduit", mod) ||
        !PythonDataFile::createType("_mworks._MWKFile", mod) ||
        !PythonMWKWriter::createType("_mworks._MWKWriter", mod) ||
        !PythonMWK2Writer::createType("_mworks._MWK2Writer", mod))
    {
        return nullptr;
    }
    
    return mod.release();
}


END_NAMESPACE_MW_PYTHON
