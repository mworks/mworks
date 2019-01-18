/*
 *  PythonDataBindingsHelpers.h
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef PYTHON_DATA_BINDINGS_HELPERS_H_
#define PYTHON_DATA_BINDINGS_HELPERS_H_

#include "PythonDataHelpers.h"


BEGIN_NAMESPACE_MW


class PythonEventWrapper {
    
public:
    PythonEventWrapper() :
        code(-1),
        time(0)
    { }
    
    PythonEventWrapper(int code, MWTime time, const Datum &value) :
        code(code),
        time(time),
        value(value)
    { }
    
    int getCode() const {
        return code;
    }
    
    boost::python::object getTime() const {
        return convert_longlong_to_python(time);
    }
    
    boost::python::object getValue() const {
        return convert_datum_to_python(value);
    }
    
    bool isEmpty() const {
        return (code < 0);
    }
    
    bool operator==(const PythonEventWrapper &other) const {
        return (code == other.code &&
                time == other.time &&
                value == other.value);
    }
    
    Datum asDatum() const {
        return Datum(Datum::list_value_type { Datum(code), Datum(time), value });
    }
    
private:
    int code;
    MWTime time;
    Datum value;
    
};


class PythonDataFile : boost::noncopyable {
    
public:
    explicit PythonDataFile(const std::string &_file_name);
    
    void open();    
    void close();    
    
    bool loaded() const;
    
    std::string file() const;
    
    unsigned int num_events() const;
    MWTime minimum_time() const;
    MWTime maximum_time() const;
    
    void select_events(const boost::python::list &codes, MWTime lower_bound, MWTime upper_bound);
    PythonEventWrapper get_next_event();
    std::vector<PythonEventWrapper> get_events();
    
private:
    void requireValidIndexer() const;
    
    const std::string file_name;
    boost::scoped_ptr<scarab::dfindex> indexer;
    
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
	
    PythonEventWrapper read_event();
    void write_event(const PythonEventWrapper &e);
    
    void flush();
    
private:
    void requireValidSession() const;
    
    Datum readDatum();
    void writeDatum(const Datum &datum);
    
    const std::string uri;
    ScarabSession *session;

};    


END_NAMESPACE_MW


#endif /* !defined(PYTHON_DATA_BINDINGS_HELPERS_H_) */



























