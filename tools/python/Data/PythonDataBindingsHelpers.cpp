/*
 *  PythonDataBindingsHelpers.cpp
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "PythonDataBindingsHelpers.h"

#include "GILHelpers.h"

#include <MWorksCore/DataFileUtilities.h>
#include <MWorksCore/ScarabServices.h>

using boost::python::throw_error_already_set;


BEGIN_NAMESPACE_MW


PythonDataFile::PythonDataFile(const std::string &_file_name) :
    file_name(_file_name)
{ }


std::string PythonDataFile::file() const {
    return file_name;
}


void PythonDataFile::open(){
    if (reader) {
        PyErr_SetString(PyExc_IOError, "Data file is already open");
        throw_error_already_set();
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
    return (reader != NULL);
}


unsigned int PythonDataFile::num_events() const {
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


void PythonDataFile::select_events(const boost::python::list &codes, MWTime lower_bound, MWTime upper_bound)
{
    requireValidReader();
    
    std::unordered_set<int> event_codes;
    int n = len(codes);
        
    for(int i = 0; i < n; i++){
        event_codes.insert(boost::python::extract<int>(codes[i]));
    }
    
    reader->selectEvents(event_codes, lower_bound, upper_bound);
}


PythonEventWrapper PythonDataFile::get_next_event() {
    requireValidReader();
    ScopedGILRelease sgr;
    
    int code;
    MWTime time;
    Datum value;
    if (reader->nextEvent(code, time, value)) {
        return PythonEventWrapper(code, time, value);
    }
    
    return PythonEventWrapper();
}


std::vector<PythonEventWrapper> PythonDataFile::get_events() {
    requireValidReader();
    
    ScopedGILRelease sgr;
    std::vector<PythonEventWrapper> events;
    
    int code;
    MWTime time;
    Datum value;
    while (reader->nextEvent(code, time, value)) {
        events.emplace_back(code, time, value);
    }
    
    return events;
}


void PythonDataFile::requireValidReader() const {
    if (!reader) {
        PyErr_SetString(PyExc_IOError, "Data file is not open");
        throw_error_already_set();
    }
}


PythonMWKWriter::PythonMWKWriter(const std::string &filename) :
    session(nullptr)
{
    int err;
    {
        ScopedGILRelease sgr;
        err = scarab_create_file(filename.c_str());
    }
    if (err != 0) {
        PyErr_Format(PyExc_IOError, "Cannot create file '%s'", filename.c_str());
        throw_error_already_set();
    }
    
    {
        ScopedGILRelease sgr;
        session = scarab_session_connect(("ldobinary:file://" + filename).c_str());
    }
    if (!session) {
        // The pointer will be NULL only if scarab_mem_malloc failed
        PyErr_NoMemory();
        throw_error_already_set();
    }
    
    err = scarab_session_geterr(session);
    if (err != 0) {
        scarab_mem_free(session);
        PyErr_Format(PyExc_IOError, "Cannot open file '%s' for writing: %s", filename.c_str(), scarab_strerror(err));
        throw_error_already_set();
    }
}


PythonMWKWriter::~PythonMWKWriter() {
    ScopedGILRelease sgr;
    (void)scarab_session_close(session);  // Ignore return value
}


void PythonMWKWriter::write_event(int code, MWTime time, const boost::python::object &data) {
    Event event(code, time, convert_python_to_datum(data));
    int err;
    {
        ScopedGILRelease sgr;
        err = scarab_write(session, eventToScarabEventDatum(event).get());
    }
    if (err != 0) {
        PyErr_Format(PyExc_IOError, "Event write failed: %s", scarab_strerror(err));
        throw_error_already_set();
    }
}


PythonMWK2Writer::PythonMWK2Writer(const std::string &filename) {
    ScopedGILRelease sgr;
    writer.reset(new MWK2Writer(filename));
}


PythonMWK2Writer::PythonMWK2Writer(const std::string &filename, std::size_t pageSize) {
    ScopedGILRelease sgr;
    writer.reset(new MWK2Writer(filename, pageSize));
}


PythonMWK2Writer::~PythonMWK2Writer() {
    ScopedGILRelease sgr;
    writer.reset();
}


void PythonMWK2Writer::write_event(int code, MWTime time, const boost::python::object &data) {
    auto eventData = convert_python_to_datum(data);
    ScopedGILRelease sgr;
    writer->writeEvent(code, time, eventData);
}


void PythonMWK2Writer::begin_transaction() {
    ScopedGILRelease sgr;
    writer->beginTransaction();
}


void PythonMWK2Writer::commit_transaction() {
    ScopedGILRelease sgr;
    writer->commitTransaction();
}


END_NAMESPACE_MW


























