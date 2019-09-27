/*
 *  PythonDataBindingsHelpers.cpp
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "PythonDataBindingsHelpers.h"

#include "PythonEvent.hpp"


BEGIN_NAMESPACE_MW_PYTHON


PythonDataFile::~PythonDataFile() {
    if (reader) {
        ScopedGILRelease sgr;
        reader.reset();
    }
}


void PythonDataFile::init(const std::string &file) {
    file_name = file;
}


std::string PythonDataFile::file() const {
    return file_name;
}


void PythonDataFile::open(){
    if (reader) {
        PyErr_SetString(PyExc_IOError, "Data file is already open");
        throw ErrorAlreadySet();
    }
    
    // convert the filename string to a boost::filesystem path
    boost::filesystem::path file_path(file_name);
    
    // open the file
    ScopedGILRelease sgr;
    reader = DataFileReader::openDataFile(file_path);
}


void PythonDataFile::close(){
    // close it
    ScopedGILRelease sgr;
    reader.reset();
}


bool PythonDataFile::loaded() const {
    return bool(reader);
}


std::size_t PythonDataFile::num_events() const {
    requireValidReader();
    return reader->getNumEvents();
}


MWTime PythonDataFile::minimum_time() const {
    requireValidReader();
    return reader->getTimeMin();
}


MWTime PythonDataFile::maximum_time() const {
    requireValidReader();
    return reader->getTimeMax();
}


void PythonDataFile::select_events(const ObjectPtr &codes, MWTime lower_bound, MWTime upper_bound) {
    if (!PyList_Check(codes.get())) {
        PyErr_SetString(PyExc_TypeError, "first argument must be a list");
        throw ErrorAlreadySet();
    }
    
    requireValidReader();
    
    std::unordered_set<int> event_codes;
    auto numCodes = PyList_Size(codes.get());
    
    for (Py_ssize_t i = 0; i < numCodes; i++) {
        auto item = PyList_GetItem(codes.get(), i);
        if (!item) {
            throw ErrorAlreadySet();
        }
        auto codeObj = ObjectPtr::created(PyNumber_Long(item));
        auto code = PyLong_AsLong(codeObj.get());
        if (-1 == code && PyErr_Occurred()) {
            throw ErrorAlreadySet();
        }
        event_codes.insert(code);
    }
    
    ScopedGILRelease sgr;
    reader->selectEvents(event_codes, lower_bound, upper_bound);
}


ObjectPtr PythonDataFile::get_next_event() {
    requireValidReader();
    
    int code;
    MWTime time;
    Datum value;
    bool success = false;
    {
        ScopedGILRelease sgr;
        success = reader->nextEvent(code, time, value);
    }
    
    if (success) {
        return PythonEvent::create(code, time, value);
    }
    
    return ObjectPtr::none();
}


void PythonDataFile::requireValidReader() const {
    if (!reader) {
        PyErr_SetString(PyExc_IOError, "Data file is not open");
        throw ErrorAlreadySet();
    }
}


template<>
PyMethodDef ExtensionType<PythonDataFile>::methods[] = {
    MethodDef<&PythonDataFile::open>("open"),
    MethodDef<&PythonDataFile::close>("close"),
    MethodDef<&PythonDataFile::select_events>("_select_events"),
    MethodDef<&PythonDataFile::get_next_event>("_get_next_event"),
    { }  // Sentinel
};


template<>
PyGetSetDef ExtensionType<PythonDataFile>::getset[] = {
    GetSetDef<&PythonDataFile::file>("file"),
    GetSetDef<&PythonDataFile::loaded>("loaded"),
    GetSetDef<&PythonDataFile::loaded>("valid"),  // For backwards compatibility (I think)
    GetSetDef<&PythonDataFile::num_events>("num_events"),
    GetSetDef<&PythonDataFile::minimum_time>("minimum_time"),
    GetSetDef<&PythonDataFile::maximum_time>("maximum_time"),
    { }  // Sentinel
};


PythonMWKWriter::~PythonMWKWriter() {
    if (session) {
        ScopedGILRelease sgr;
        (void)scarab_session_close(session);  // Ignore return value
    }
}


void PythonMWKWriter::init(const std::string &filename) {
    int err;
    {
        ScopedGILRelease sgr;
        if (session) {
            (void)scarab_session_close(session);  // Ignore return value
            session = nullptr;
        }
        err = scarab_create_file(filename.c_str());
    }
    if (err != 0) {
        PyErr_Format(PyExc_IOError, "Cannot create file '%s'", filename.c_str());
        throw ErrorAlreadySet();
    }
    
    {
        ScopedGILRelease sgr;
        session = scarab_session_connect(("ldobinary:file://" + filename).c_str());
    }
    if (!session) {
        // The pointer will be NULL only if scarab_mem_malloc failed
        PyErr_NoMemory();
        throw ErrorAlreadySet();
    }
    
    err = scarab_session_geterr(session);
    if (err != 0) {
        scarab_mem_free(session);
        session = nullptr;
        PyErr_Format(PyExc_IOError, "Cannot open file '%s' for writing: %s", filename.c_str(), scarab_strerror(err));
        throw ErrorAlreadySet();
    }
}


void PythonMWKWriter::write_event(int code, MWTime time, const Datum &data) {
    if (!session) {
        PyErr_SetString(PyExc_IOError, "File is not open");
        throw ErrorAlreadySet();
    }
    
    Event event(code, time, data);
    int err;
    {
        ScopedGILRelease sgr;
        err = scarab_write(session, eventToScarabEventDatum(event).get());
    }
    if (err != 0) {
        PyErr_Format(PyExc_IOError, "Event write failed: %s", scarab_strerror(err));
        throw ErrorAlreadySet();
    }
}


template<>
PyMethodDef ExtensionType<PythonMWKWriter>::methods[] = {
    MethodDef<&PythonMWKWriter::write_event>("write_event"),
    { }  // Sentinel
};


PythonMWK2Writer::~PythonMWK2Writer() {
    if (writer) {
        ScopedGILRelease sgr;
        writer.reset();
    }
}


void PythonMWK2Writer::init(const std::string &filename, Py_ssize_t pageSize) {
    ScopedGILRelease sgr;
    if (pageSize > 0) {
        writer.reset(new MWK2Writer(filename, pageSize));
    } else {
        writer.reset(new MWK2Writer(filename));
    }
}


void PythonMWK2Writer::write_event(int code, MWTime time, const Datum &data) {
    requireValidWriter();
    ScopedGILRelease sgr;
    writer->writeEvent(code, time, data);
}


void PythonMWK2Writer::begin_transaction() {
    requireValidWriter();
    ScopedGILRelease sgr;
    writer->beginTransaction();
}


void PythonMWK2Writer::commit_transaction() {
    requireValidWriter();
    ScopedGILRelease sgr;
    writer->commitTransaction();
}


void PythonMWK2Writer::requireValidWriter() const {
    if (!writer) {
        PyErr_SetString(PyExc_IOError, "File is not open");
        throw ErrorAlreadySet();
    }
}


template<>
PyMethodDef ExtensionType<PythonMWK2Writer>::methods[] = {
    MethodDef<&PythonMWK2Writer::write_event>("write_event"),
    MethodDef<&PythonMWK2Writer::begin_transaction>("begin_transaction"),
    MethodDef<&PythonMWK2Writer::commit_transaction>("commit_transaction"),
    { }  // Sentinel
};


END_NAMESPACE_MW_PYTHON
