/*
 *  PythonDataBindingsHelpers.cpp
 *  DataFileIndexer
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "PythonDataBindingsHelpers.h"

#include <limits.h>


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


PythonDataStream::PythonDataStream(std::string _uri){
    uri = _uri;
    session = NULL;
}

void PythonDataStream::open(){
    //std::cerr << "Opening file: " << uri_temp << "..." << std::endl;
    session = scarab_session_connect(uri.c_str());
    
    int err;
    if(err = scarab_session_geterr(session)){
        std::cerr << "Error: " << scarab_strerror(err) << std::endl; 
        session = NULL;
    }
    
    if(err = scarab_session_getoserr(session)){
        std::cerr << "Error: " << scarab_strerror(err) << std::endl; 
        session = NULL;
    }
    
    if(!session){
        PyErr_Warn(NULL, "Unable to connect Scarab session");
    }
    
}

void PythonDataStream::close(){
    // close it
    if(session) {
        scarab_session_close(session);
        session = NULL;
    }
}

// Creates a new scarab file
int PythonDataStream::_scarab_create_file(std::string _fn) {
    return scarab_create_file(_fn.c_str());
}

// Reads at most `len` bytes from the session file.
std::string PythonDataStream::_scarab_session_read(int len) {
    std::string rtn_buf;
    if (!session) return rtn_buf;
    
    char *buf = new char[len];
    size_t actual_len = (size_t) scarab_session_read(session, buf, len);
    rtn_buf.assign(buf, actual_len);  // "copies" the buf
    delete [] buf;
    
    return rtn_buf;
}

int PythonDataStream::_scarab_session_write(std::string buf) {
    if (!session) return -1;
    return scarab_session_write(session, buf.data(), buf.size());
}

int PythonDataStream::_scarab_session_seek(long int offset, int origin) {
    if (!session) return -1;
    return scarab_session_seek(session, offset, origin);
}

long int PythonDataStream::_scarab_session_tell(void) {
    if (!session) return -1;
    return scarab_session_tell(session);
}

int PythonDataStream::_scarab_session_flush(void) {
    if (!session) return -1;
    return scarab_session_flush(session);
}

int PythonDataStream::_scarab_write(PyObject *obj) {
    if (!session) return -1;

    ScarabDatum *datum = convert_python_to_scarab(obj);
    int r_val = scarab_write(session, datum);
    scarab_free_datum(datum);
    return r_val;
}


shared_ptr<EventWrapper> PythonDataStream::read_event(){
    ScarabDatum *current_datum = NULL;
    if(session){
        current_datum = scarab_read(session);
    }
    
    int err = 0;
    if(err = scarab_session_geterr(session)){
        std::cerr << "Error: " << scarab_strerror(err) << std::endl; 
    }
    
    if(err = scarab_session_getoserr(session)){
        std::cerr << "Error: " << scarab_strerror(err) << std::endl; 
        session = NULL;
    }
    
    shared_ptr<EventWrapper> event(new EventWrapper(current_datum));
    return event;
}
    
int PythonDataStream::write_event(shared_ptr<EventWrapper> e) {
    if (!session || !e) return -1;
    ScarabDatum *datum = e->getDatum();
    if (!datum) return -1;
    
    return scarab_write(session, datum); 
}


PyObject *extract_event_value(EventWrapper e){
    if(e.empty()){
        // TODO throw / complain
        Py_RETURN_NONE;
    }
    return convert_scarab_to_python(e.getPayload());
}
