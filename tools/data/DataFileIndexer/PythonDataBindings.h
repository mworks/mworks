/*
 *  DataFile.h
 *  DataFileIndexer
 *
 *  Created by David Cox on 8/17/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef PYTHON_DATA_BINDINGS_H_
#define PYTHON_DATA_BINDINGS_H_


#include "PythonDataBindingsHelpers.h"

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/register_ptr_to_python.hpp>

using namespace boost;
using namespace boost::python;



// Create a module called "_data"; a pure python module ("data") will load this and add some
// more pythonic fixin's on top
BOOST_PYTHON_MODULE(_data)
{
    
    if (scarab_init(0) != 0) {
        PyErr_SetString(PyExc_RuntimeError, "Scarab initialization failed");
        throw_error_already_set();
    }
 
    class_<EventWrapper, boost::noncopyable>("Event", init<ScarabDatum *>())
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

#endif


























