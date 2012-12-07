/*
 *  PythonDataBindingsHelpers.cpp
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "PythonDataBindingsHelpers.h"

#include "GILHelpers.h"
#include "PythonDataHelpers.h"

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
    indexer.reset(new dfindex(file_path));
}


void PythonDataFile::close(){
    // close it
    ScopedGILRelease sgr;
    indexer.reset();
    eventsIterator.reset();
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


void PythonDataFile::select_events(const bp::list &codes, MWTime lower_bound, MWTime upper_bound)
{
    requireValidIndexer();
    
    std::set<unsigned int> event_codes;
    int n = len(codes);
        
    for(int i = 0; i < n; i++){
        event_codes.insert(bp::extract<unsigned int>(codes[i]));
    }
    
    eventsIterator.reset(new DataFileIndexer::EventsIterator(indexer->getEventsIterator(event_codes, lower_bound, upper_bound)));
}


EventWrapper PythonDataFile::get_next_event() {
    requireValidEventsIterator();
    ScopedGILRelease sgr;
    return eventsIterator->getNextEvent();
}


std::vector<EventWrapper> PythonDataFile::get_events() {
    requireValidEventsIterator();
    
    ScopedGILRelease sgr;
    std::vector<EventWrapper> events;
    
    while (true) {
        EventWrapper evt = eventsIterator->getNextEvent();
        if (evt.empty())
            break;
        events.push_back(evt);
    }
    
    return events;
}


void PythonDataFile::requireValidIndexer() const {
    if (!indexer) {
        PyErr_SetString(PyExc_IOError, "Data file is not open");
        throw_error_already_set();
    }
}


void PythonDataFile::requireValidEventsIterator() const {
    if (!eventsIterator) {
        PyErr_SetString(PyExc_IOError, "No events have been selected");
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


EventWrapper PythonDataStream::read_event() {
    Datum datum(readDatum());
    if (!DataFileUtilities::isScarabEvent(datum.getScarabDatum())) {
        PyErr_SetString(PyExc_IOError, "Read invalid event from Scarab session");
        throw_error_already_set();
    }
    return EventWrapper(datum.getScarabDatum());
}


void PythonDataStream::write_event(const EventWrapper &e) {
    Datum datum(e.getDatum());
    writeDatum(datum);
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
    
    Datum datum(rawDatum);
    scarab_free_datum(rawDatum);
    
    return datum;
}


void PythonDataStream::writeDatum(const Datum &datum) {
    requireValidSession();
    
    int err;
    {
        ScopedGILRelease sgr;
        err = scarab_write(session, datum.getScarabDatum());
    }
    
    if (err != 0) {
        PyErr_Format(PyExc_IOError, "Scarab write failed: %s", scarab_strerror(err));
        throw_error_already_set();
    }
}


boost::python::object extract_event_time(const EventWrapper &e) {
    return convert_longlong_to_python(e.getTime());
}


boost::python::object extract_event_value(const EventWrapper &e) {
    return convert_datum_to_python(Datum(e.getPayload()));
}


END_NAMESPACE_MW


























