/*
 *  PythonDataBindingsHelpers.cpp
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "PythonDataBindingsHelpers.h"

#include <limits.h>

#include "PythonDataHelpers.h"

using boost::python::throw_error_already_set;


BEGIN_NAMESPACE_MW


PythonDataFile::PythonDataFile(std::string _file_name){
    file_name = _file_name;
}

std::string PythonDataFile::file(){
    return file_name;
}

std::string PythonDataFile::true_mwk_file(){
    if(indexer != NULL){
        return indexer->getFilePath();
    } else {
        return "";
    }
}

void PythonDataFile::open(){
    
    // convert the filename string to a boost::filesystem path
    // TODO: catch exception
    boost::filesystem::path file_path(file_name);
    
    // open the file
    indexer = shared_ptr<dfindex>(new dfindex(file_path));
}

void PythonDataFile::close(){
    // close it
    indexer = shared_ptr<dfindex>();
    eventsIterator = shared_ptr<DataFileIndexer::EventsIterator>();
}

bool PythonDataFile::exists(){
    return true;
}

bool PythonDataFile::loaded(){
    return (indexer != NULL);
}

bool PythonDataFile::valid(){
    return (indexer != NULL);
}

MWTime PythonDataFile::minimum_time() {
    if (indexer != NULL) {
        return indexer->getMinimumTime();
    }
    return MIN_MONKEY_WORKS_TIME();
}

MWTime PythonDataFile::maximum_time() {
    if (indexer != NULL) {
        return indexer->getMaximumTime();
    }
    return MAX_MONKEY_WORKS_TIME();
}


void PythonDataFile::select_events(bp::list codes, const MWTime lower_bound, const MWTime upper_bound)
{
    if (indexer == NULL) {
        throw std::runtime_error("data file is not open");
    }
    
    std::vector<unsigned int> event_codes;
    int n = len(codes);
        
    for(int i = 0; i < n; i++){
        event_codes.push_back(bp::extract<unsigned int>(codes[i]));
    }
    
    eventsIterator = shared_ptr<DataFileIndexer::EventsIterator>(new DataFileIndexer::EventsIterator(indexer->getEventsIterator(event_codes, lower_bound, upper_bound)));
}


shared_ptr<EventWrapper> PythonDataFile::get_next_event() {
    if (eventsIterator == NULL) {
        throw std::runtime_error("no event iterator available");
    }
    return shared_ptr<EventWrapper>(new EventWrapper(eventsIterator->getNextEvent()));
}


std::vector<EventWrapper> PythonDataFile::get_events() {
    if (eventsIterator == NULL) {
        throw std::runtime_error("no event iterator available");
    }
    
    std::vector<EventWrapper> events;
    EventWrapper evt;
    
    while ((evt = eventsIterator->getNextEvent())) {
        events.push_back(evt);
    }
    
    return events;
}


void PythonDataStream::createFile(const std::string &filename) {
    if (scarab_create_file(filename.c_str()) != 0) {
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
        PyErr_SetString(PyExc_ValueError, "Scarab session is already connected");
        throw_error_already_set();
    }
    
    session = scarab_session_connect(uri.c_str());
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


shared_ptr<EventWrapper> PythonDataStream::read_event(){
    Datum datum(readDatum());
    return shared_ptr<EventWrapper>(new EventWrapper(datum.getScarabDatum()));
}


void PythonDataStream::write_event(const shared_ptr<EventWrapper> &e) {
    Datum datum(e->getDatum());
    writeDatum(datum);
}


void PythonDataStream::requireValidSession() const {
    if (!session) {
        PyErr_SetString(PyExc_ValueError, "Scarab session is not connected");
        throw_error_already_set();
    }
}


Datum PythonDataStream::readDatum() {
    requireValidSession();
    
    ScarabDatum *rawDatum = scarab_read(session);
    
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
    
    int err = scarab_write(session, datum.getScarabDatum());
    
    if (err != 0) {
        PyErr_Format(PyExc_IOError, "Scarab write failed: %s", scarab_strerror(err));
        throw_error_already_set();
    }
}


boost::python::object extract_event_value(EventWrapper e){
    if(e.empty()){
        // TODO throw / complain
        return boost::python::object();
    }
    return convert_datum_to_python(Datum(e.getPayload()));
}


END_NAMESPACE_MW


























