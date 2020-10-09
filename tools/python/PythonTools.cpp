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
    
    auto module = ObjectPtr::owned(PyModule_Create(&_mworksModule));
    if (!module) {
        return nullptr;
    }
    
    importNumpyTypes();
    
    if (PyModule_AddIntMacro(module.get(), RESERVED_CODEC_CODE) ||
        PyModule_AddIntMacro(module.get(), RESERVED_SYSTEM_EVENT_CODE) ||
        PyModule_AddIntMacro(module.get(), RESERVED_COMPONENT_CODEC_CODE) ||
        PyModule_AddIntMacro(module.get(), RESERVED_TERMINATION_CODE) ||
        PyModule_AddIntConstant(module.get(), "server_event_transport", EventTransport::server_event_transport) ||
        PyModule_AddIntConstant(module.get(), "client_event_transport", EventTransport::client_event_transport) ||
        PyModule_AddIntConstant(module.get(), "_Py_LIMITED_API", Py_LIMITED_API))
    {
        return nullptr;
    }
    
    if (!PythonEvent::createType("_mworks.Event", module) ||
        !PythonIPCConduit::createType("_mworks._IPCConduit", module) ||
        !PythonDataFile::createType("_mworks._MWKFile", module) ||
        !PythonMWKWriter::createType("_mworks._MWKWriter", module) ||
        !PythonMWK2Writer::createType("_mworks._MWK2Writer", module))
    {
        return nullptr;
    }
    
    return module.release();
}


END_NAMESPACE_MW_PYTHON
