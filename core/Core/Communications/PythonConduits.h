/*
 *  PythonConduits.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/20/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */


#include "PythonSimpleConduit.h"
#include "PythonAccumulatingConduit.h"

namespace mw {


BOOST_PYTHON_MODULE(_conduit)
{
    
    PyEval_InitThreads();
    
    class_<PythonIPCServerConduit>("_IPCServerConduit", init<std::string, bool>())
    .def("_initialize", &PythonIPCServerConduit::initialize)
    .def("finalize", &PythonIPCServerConduit::finalize)
    .def("send_float", &PythonIPCServerConduit::sendFloat)
    .def("send_integer", &PythonIPCServerConduit::sendInteger)
    .def("send_object", &PythonIPCServerConduit::sendPyObject)
    .def("register_callback_for_code", &PythonIPCServerConduit::registerCallbackForCode)
    .def("register_callback_for_name", &PythonIPCServerConduit::registerCallbackForName)
    .def("register_local_event_code", &PythonIPCServerConduit::registerLocalEventCode)
    .add_property("codec", &PythonIPCServerConduit::getCodec)
    .add_property("reverse_codec", &PythonIPCServerConduit::getReverseCodec)
    .add_property("initialized", &PythonIPCServerConduit::isInitialized)
    ;
    
    class_<PythonIPCClientConduit>("_IPCClientConduit", init<std::string, bool>())
    .def("_initialize", &PythonIPCClientConduit::initialize)
    .def("finalize", &PythonIPCClientConduit::finalize)
    .def("send_float", &PythonIPCClientConduit::sendFloat)
    .def("send_integer", &PythonIPCClientConduit::sendInteger)
    .def("send_object", &PythonIPCClientConduit::sendPyObject)
    .def("register_callback_for_code", &PythonIPCClientConduit::registerCallbackForCode)
    .def("register_callback_for_name", &PythonIPCClientConduit::registerCallbackForName)
    .def("register_local_event_code", &PythonIPCClientConduit::registerLocalEventCode)
    .add_property("codec", &PythonIPCClientConduit::getCodec)
    .add_property("reverse_codec", &PythonIPCClientConduit::getReverseCodec)
    .add_property("initialized", &PythonIPCClientConduit::isInitialized)
    ;
    
    class_<Event>("Event")
    .def("_convert_mw_datum_to_python", &convert_datum_to_python)
    .staticmethod("_convert_mw_datum_to_python")
    .add_property("code", &Event::getEventCode)
    .add_property("time", &Event::getTime)
    .add_property("_mw_datum", &Event::getData);
    ;
    
    register_ptr_to_python< shared_ptr<Event> >();
    
    class_<Datum>("Datum")
    .add_property("float", &Datum::getFloat)
    .add_property("integer", &Datum::getInteger);
    ;
    
    
    class_<PythonIPCAccumulatingServerConduit>("_IPCAccumulatingServerConduit", init< string,string,string,boost::python::list>())
    .def("_initialize", &PythonIPCAccumulatingServerConduit::initialize)
    .def("finalize", &PythonIPCAccumulatingServerConduit::finalize)
    .def("send_float", &PythonIPCAccumulatingServerConduit::sendFloat)
    .def("send_integer", &PythonIPCAccumulatingServerConduit::sendInteger)
    .def("send_object", &PythonIPCAccumulatingServerConduit::sendPyObject)
    .def("register_callback_for_code", &PythonIPCAccumulatingServerConduit::registerCallbackForCode)
    .def("register_callback_for_name", &PythonIPCAccumulatingServerConduit::registerCallbackForName)
    .def("register_local_event_code", &PythonIPCAccumulatingServerConduit::registerLocalEventCode)
    .def("register_bundle_callback", &PythonIPCAccumulatingServerConduit::registerBundleCallback)
    .add_property("initialized", &PythonIPCAccumulatingServerConduit::isInitialized)
    ;
    
    class_<PythonIPCAccumulatingClientConduit>("_IPCAccumulatingClientConduit", init< string,string,string,boost::python::list>())
    .def("_initialize", &PythonIPCAccumulatingClientConduit::initialize)
    .def("finalize", &PythonIPCAccumulatingClientConduit::finalize)
    .def("send_float", &PythonIPCAccumulatingClientConduit::sendFloat)
    .def("send_integer", &PythonIPCAccumulatingClientConduit::sendInteger)
    .def("send_object", &PythonIPCAccumulatingClientConduit::sendPyObject)
    .def("register_callback_for_code", &PythonIPCAccumulatingClientConduit::registerCallbackForCode)
    .def("register_callback_for_name", &PythonIPCAccumulatingClientConduit::registerCallbackForName)
    .def("register_local_event_code", &PythonIPCAccumulatingClientConduit::registerLocalEventCode)
    .def("register_bundle_callback", &PythonIPCAccumulatingClientConduit::registerBundleCallback)
    .add_property("initialized", &PythonIPCAccumulatingClientConduit::isInitialized)
    ;
    
    
}


}  // end namespace mw
