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
    if (indexer) {
        PyErr_SetString(PyExc_IOError, "Data file is already open");
        throw_error_already_set();
    }
    
    // convert the filename string to a boost::filesystem path
    boost::filesystem::path file_path(file_name);
    
    // open the file
    ScopedGILRelease sgr;
    indexer.reset(new scarab::dfindex(file_path));
}


void PythonDataFile::close(){
    // close it
    ScopedGILRelease sgr;
    indexer.reset();
}


bool PythonDataFile::loaded() const {
    return (indexer != NULL);
}


unsigned int PythonDataFile::num_events() const {
    requireValidIndexer();
    return indexer->getNEvents();
}


MWTime PythonDataFile::minimum_time() const {
    requireValidIndexer();
    return indexer->getMinimumTime();
}


MWTime PythonDataFile::maximum_time() const {
    requireValidIndexer();
    return indexer->getMaximumTime();
}


void PythonDataFile::select_events(const boost::python::list &codes, MWTime lower_bound, MWTime upper_bound)
{
    requireValidIndexer();
    
    std::set<unsigned int> event_codes;
    int n = len(codes);
        
    for(int i = 0; i < n; i++){
        event_codes.insert(boost::python::extract<unsigned int>(codes[i]));
    }
    
    indexer->selectEvents(event_codes, lower_bound, upper_bound);
}


PythonEventWrapper PythonDataFile::get_next_event() {
    requireValidIndexer();
    ScopedGILRelease sgr;
    
    int code;
    MWTime time;
    Datum value;
    if (indexer->getNextEvent(code, time, value)) {
        return PythonEventWrapper(code, time, value);
    }
    
    return PythonEventWrapper();
}


std::vector<PythonEventWrapper> PythonDataFile::get_events() {
    requireValidIndexer();
    
    ScopedGILRelease sgr;
    std::vector<PythonEventWrapper> events;
    
    int code;
    MWTime time;
    Datum value;
    while (indexer->getNextEvent(code, time, value)) {
        events.emplace_back(code, time, value);
    }
    
    return events;
}


void PythonDataFile::requireValidIndexer() const {
    if (!indexer) {
        PyErr_SetString(PyExc_IOError, "Data file is not open");
        throw_error_already_set();
    }
}


void PythonDataStream::createFile(const std::string &filename) {
    int err;
    {
        ScopedGILRelease sgr;
        err = scarab_create_file(filename.c_str());
    }
    
    if (err != 0) {
        PyErr_Format(PyExc_IOError, "Cannot create Scarab file '%s'", filename.c_str());
        throw_error_already_set();
    }
}


PythonDataStream::PythonDataStream(const std::string &uri) :
    uri(uri),
    session(NULL)
{ }


PythonDataStream::~PythonDataStream() {
    close();
}


void PythonDataStream::open(){
    if (session) {
        PyErr_SetString(PyExc_IOError, "Scarab session is already connected");
        throw_error_already_set();
    }
    
    {
        ScopedGILRelease sgr;
        session = scarab_session_connect(uri.c_str());
    }
    
    if (!session) {
        // The pointer will be NULL only if scarab_mem_malloc failed
        PyErr_NoMemory();
        throw_error_already_set();
    }
    
    int err = scarab_session_geterr(session);
    if (err != 0) {
        scarab_mem_free(session);
        session = NULL;
        PyErr_Format(PyExc_IOError, "Cannot connect to Scarab session '%s': %s", uri.c_str(), scarab_strerror(err));
        throw_error_already_set();
    }
}


void PythonDataStream::close(){
    // close it
    if(session) {
        ScopedGILRelease sgr;
        (void)scarab_session_close(session);  // Ignore return value
        session = NULL;
    }
}


boost::python::object PythonDataStream::read() {
    return convert_datum_to_python(readDatum());
}


void PythonDataStream::write(const boost::python::object &obj) {
    writeDatum(convert_python_to_datum(obj));
}


PythonEventWrapper PythonDataStream::read_event() {
    auto scarabEvent = datumToScarabDatum(readDatum());
    if (!scarab::isScarabEvent(scarabEvent.get())) {
        PyErr_SetString(PyExc_IOError, "Read invalid event from Scarab session");
        throw_error_already_set();
    }
    return PythonEventWrapper(scarab::getScarabEventCode(scarabEvent.get()),
                              scarab::getScarabEventTime(scarabEvent.get()),
                              scarabDatumToDatum(scarab::getScarabEventPayload(scarabEvent.get())));
}


void PythonDataStream::write_event(const PythonEventWrapper &e) {
    writeDatum(e.asDatum());
}


void PythonDataStream::flush() {
    requireValidSession();
    
    int err;
    {
        ScopedGILRelease sgr;
        err = scarab_session_flush(session);
    }
    
    if (err != 0) {
        PyErr_Format(PyExc_IOError, "Scarab flush failed: %s", scarab_strerror(err));
        throw_error_already_set();
    }
}


void PythonDataStream::requireValidSession() const {
    if (!session) {
        PyErr_SetString(PyExc_IOError, "Scarab session is not connected");
        throw_error_already_set();
    }
}


Datum PythonDataStream::readDatum() {
    requireValidSession();
    
    ScarabDatum *rawDatum;
    {
        ScopedGILRelease sgr;
        rawDatum = scarab_read(session);
    }
    
    if (!rawDatum) {
        int err = scarab_session_geterr(session);
        if (err != 0) {
            PyErr_Format(PyExc_IOError, "Scarab read failed: %s", scarab_strerror(err));
        } else {
            PyErr_SetNone(PyExc_EOFError);
        }
        throw_error_already_set();
    }
    
    Datum datum = scarabDatumToDatum(rawDatum);
    scarab_free_datum(rawDatum);
    
    return datum;
}


void PythonDataStream::writeDatum(const Datum &datum) {
    requireValidSession();
    
    int err;
    {
        ScopedGILRelease sgr;
        err = scarab_write(session, datumToScarabDatum(datum).get());
    }
    
    if (err != 0) {
        PyErr_Format(PyExc_IOError, "Scarab write failed: %s", scarab_strerror(err));
        throw_error_already_set();
    }
}


END_NAMESPACE_MW


























