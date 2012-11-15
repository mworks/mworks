/*
 *  PythonTools.h
 *
 *  Created by David Cox on 10/20/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */


#include "PythonSimpleConduit.h"
#include "PythonAccumulatingConduit.h"
#include "PythonDataBindingsHelpers.h"

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>


BEGIN_NAMESPACE_MW


BOOST_PYTHON_MODULE(_mworks)
{
    
    if (scarab_init(0) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Scarab initialization failed");
        throw_error_already_set();
    }
    
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
    
    class_<EventWrapper, boost::noncopyable>("EventWrapper", init<ScarabDatum *>())
    .add_property("code", &EventWrapper::getEventCode)
    .add_property("time", &EventWrapper::getTime)
    .add_property("value", extract_event_value)
    .add_property("empty", &EventWrapper::empty)
    ;
    
    register_ptr_to_python< shared_ptr<EventWrapper> >();
    
    class_<std::vector<EventWrapper> >("EventWrapperArray")
    .def(vector_indexing_suite< std::vector<EventWrapper> >())
    ;
    
    class_<PythonDataFile>("_MWKFile", init<std::string>())
    .def("open", &PythonDataFile::open)
    .def("close", &PythonDataFile::close)
    .def("_select_events", &PythonDataFile::select_events)
    .def("_get_next_event", &PythonDataFile::get_next_event)
    .def("_get_events", &PythonDataFile::get_events)
    .add_property("exists", &PythonDataFile::exists)
    .add_property("loaded", &PythonDataFile::loaded)
    .add_property("valid", &PythonDataFile::valid)
    .add_property("minimum_time", &PythonDataFile::minimum_time)
    .add_property("maximum_time", &PythonDataFile::maximum_time)
    .add_property("file", &PythonDataFile::file)
    .add_property("true_mwk_file", &PythonDataFile::true_mwk_file)
    ;
    
    
    class_<PythonDataStream, boost::noncopyable>("_MWKStream", init<const std::string &>())
    .def("_create_file", &PythonDataStream::createFile)
    .staticmethod("_create_file")
    .def("open", &PythonDataStream::open)
    .def("close", &PythonDataStream::close)
    .def("_read", &PythonDataStream::read)
    .def("_write", &PythonDataStream::write)
    .def("_read_event", &PythonDataStream::read_event)
    .def("_write_event", &PythonDataStream::write_event)
    ;
    
}


END_NAMESPACE_MW
