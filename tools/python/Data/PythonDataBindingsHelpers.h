/*
 *  PythonDataBindingsHelpers.h
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef PYTHON_DATA_BINDINGS_HELPERS_H_
#define PYTHON_DATA_BINDINGS_HELPERS_H_

using namespace boost;
namespace bp = boost::python;


BEGIN_NAMESPACE_MW


class PythonDataFile : boost::noncopyable {
    
public:
    static void translateDFIError(const DataFileIndexerError &e) {
        PyErr_SetString(PyExc_IOError, e.what());
    }
    
    explicit PythonDataFile(const std::string &_file_name);
    
    void open();    
    void close();    
    
    bool loaded() const;
    
    std::string file() const;
    
    unsigned int num_events() const;
    MWTime minimum_time() const;
    MWTime maximum_time() const;
    
    void select_events(const bp::list &codes, MWTime lower_bound, MWTime upper_bound);
    EventWrapper get_next_event();
    std::vector<EventWrapper> get_events();
    
private:
    void requireValidIndexer() const;
    void requireValidEventsIterator() const;
    
    const std::string file_name;
    boost::scoped_ptr<dfindex> indexer;
    boost::scoped_ptr<DataFileIndexer::EventsIterator> eventsIterator;
    
};



// A simple, non-indexing stream (e.g. file / socket) reader/writer for 
// python wrapping
class PythonDataStream : boost::noncopyable {
    
public:
    static void createFile(const std::string &filename);
    
    explicit PythonDataStream(const std::string &uri);
    ~PythonDataStream();
    
    void open();
    void close();
    
    boost::python::object read();
    void write(const boost::python::object &obj);
	
    EventWrapper read_event();
    void write_event(const EventWrapper &e);
    
    void flush();
    
private:
    void requireValidSession() const;
    
    Datum readDatum();
    void writeDatum(const Datum &datum);
    
    const std::string uri;
    ScarabSession *session;

};    


boost::python::object extract_event_time(const EventWrapper &e);
boost::python::object extract_event_value(const EventWrapper &e);


END_NAMESPACE_MW


#endif /* !defined(PYTHON_DATA_BINDINGS_HELPERS_H_) */



























