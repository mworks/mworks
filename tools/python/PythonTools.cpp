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

#include <numpy/arrayobject.h>

using namespace boost::python;


BEGIN_NAMESPACE_MW


BOOST_PYTHON_MODULE(_mworks)
{
    
    // The following values for NPY_VERSION and NPY_FEATURE_VERSION (aka NPY_ABI_VERSION and
    // NPY_API_VERSION) are valid for macOS 10.11 to 10.14
    static_assert(NPY_VERSION == 0x01000009 && NPY_FEATURE_VERSION <= 0x00000009,
                  "Compiling against a NumPy version that is incompatible with the system NumPy "
                  "version on MACOSX_DEPLOYMENT_TARGET");
    
    if (_import_array() < 0) {
        PyErr_SetString(PyExc_ImportError, "Unable to import NumPy C API");
        throw_error_already_set();
    }
    
    if (scarab_init(0) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Scarab initialization failed");
        throw_error_already_set();
    }
    
    enum_<ReservedEventCode>("ReservedEventCode")
    .value("RESERVED_CODEC_CODE", RESERVED_CODEC_CODE)
    .value("RESERVED_SYSTEM_EVENT_CODE", RESERVED_SYSTEM_EVENT_CODE)
    .value("RESERVED_COMPONENT_CODEC_CODE", RESERVED_COMPONENT_CODEC_CODE)
    .value("RESERVED_TERMINATION_CODE", RESERVED_TERMINATION_CODE)
    ;
    
    class_<Event>("Event", no_init)
    .add_property("code", &Event::getEventCode)
    .add_property("time", extractEventTime)
    .add_property("data", extractEventData)
    .add_property("value", extractEventData)  // For compatibility with EventWrapper
    ;
    
    register_ptr_to_python< shared_ptr<Event> >();
    
    class_<PythonIPCConduit, boost::noncopyable>("_IPCConduit", no_init)
    .def("initialize", &PythonIPCConduit::initialize)
    .def("finalize", &PythonIPCConduit::finalize)
    .def("send_data", &PythonIPCConduit::sendPyObject)
    .def("register_callback_for_code", &PythonIPCConduit::registerCallbackForCode)
    .def("register_callback_for_name", &PythonIPCConduit::registerCallbackForName)
    .def("register_local_event_code", &PythonIPCConduit::registerLocalEventCode)
    .add_property("codec", &PythonIPCConduit::getCodec)
    .add_property("reverse_codec", &PythonIPCConduit::getReverseCodec)
    .add_property("initialized", &PythonIPCConduit::isInitialized)
    // The following three methods aren't needed but remain for backwards compatibility
    .def("send_float", &PythonIPCConduit::sendFloat)
    .def("send_integer", &PythonIPCConduit::sendInteger)
    .def("send_object", &PythonIPCConduit::sendPyObject)
    ;
    
    class_<PythonIPCServerConduit, boost::noncopyable, bases<PythonIPCConduit> >
    ("_IPCServerConduit", init<std::string, bool>())
    .def(init<std::string>())
    ;
    
    class_<PythonIPCClientConduit, boost::noncopyable, bases<PythonIPCConduit> >
    ("_IPCClientConduit", init<std::string, bool>())
    .def(init<std::string>())
    ;
    
    class_<PythonIPCAccumulatingConduit, boost::noncopyable, bases<PythonIPCConduit> >
    ("_IPCAccumulatingConduit", no_init)
    .def("initialize", &PythonIPCAccumulatingConduit::initialize)
    .def("register_bundle_callback", &PythonIPCAccumulatingConduit::registerBundleCallback)
    ;
    
    class_<PythonIPCAccumulatingServerConduit, boost::noncopyable, bases<PythonIPCAccumulatingConduit> >
    ("_IPCAccumulatingServerConduit", init<string, string, string, boost::python::list, bool>())
    .def(init<string, string, string, boost::python::list>())
    ;
    
    class_<PythonIPCAccumulatingClientConduit, boost::noncopyable, bases<PythonIPCAccumulatingConduit> >
    ("_IPCAccumulatingClientConduit", init<string, string, string, boost::python::list, bool>())
    .def(init<string, string, string, boost::python::list>())
    ;
    
    class_<PythonEventWrapper>("EventWrapper", no_init)
    .add_property("code", &PythonEventWrapper::getCode)
    .add_property("time", &PythonEventWrapper::getTime)
    .add_property("value", &PythonEventWrapper::getValue)
    .add_property("data", &PythonEventWrapper::getValue)  // For compatibility with Event
    .add_property("empty", &PythonEventWrapper::isEmpty)
    ;
    
    class_<std::vector<PythonEventWrapper>>("EventWrapperArray", no_init)
    .def(vector_indexing_suite<std::vector<PythonEventWrapper>>())
    ;
    
    class_<PythonDataFile, boost::noncopyable>("_MWKFile", init<std::string>())
    .def("open", &PythonDataFile::open)
    .def("close", &PythonDataFile::close)
    .def("_select_events", &PythonDataFile::select_events)
    .def("_get_next_event", &PythonDataFile::get_next_event)
    .def("_get_events", &PythonDataFile::get_events)
    .add_property("loaded", &PythonDataFile::loaded)
    .add_property("valid", &PythonDataFile::loaded)
    .add_property("num_events", &PythonDataFile::num_events)
    .add_property("minimum_time", &PythonDataFile::minimum_time)
    .add_property("maximum_time", &PythonDataFile::maximum_time)
    .add_property("file", &PythonDataFile::file)
    ;
    
    class_<PythonMWKWriter, boost::noncopyable>("_MWKWriter", init<std::string>())
    .def("write_event", &PythonMWKWriter::write_event)
    ;
    
    class_<PythonMWK2Writer, boost::noncopyable>("_MWK2Writer", init<std::string>())
    .def("write_event", &PythonMWK2Writer::write_event)
    .def("begin_transaction", &PythonMWK2Writer::begin_transaction)
    .def("commit_transaction", &PythonMWK2Writer::commit_transaction)
    ;
    
}


END_NAMESPACE_MW



























