/*
 *  PythonDataBindingsHelpers.h
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef PYTHON_DATA_BINDINGS_HELPERS_H_
#define PYTHON_DATA_BINDINGS_HELPERS_H_

#include "ExtensionType.h"


BEGIN_NAMESPACE_MW_PYTHON


class PythonDataFile : public ExtensionType<PythonDataFile> {
    
public:
    ~PythonDataFile();
    
    void init(const std::string &file);
    
    std::string file() const;
    
    void open();
    void close();
    
    bool loaded() const;
    
    std::size_t num_events() const;
    MWTime minimum_time() const;
    MWTime maximum_time() const;
    
    void select_events(const ObjectPtr &codes, MWTime lower_bound, MWTime upper_bound);
    ObjectPtr get_next_event();
    
private:
    void requireValidReader() const;
    
    std::string file_name;
    std::unique_ptr<DataFileReader> reader;
    
};


class PythonMWKWriter : public ExtensionType<PythonMWKWriter>, boost::noncopyable {
    
public:
    ~PythonMWKWriter();
    
    void init(const std::string &filename);
    
    void write_event(int code, MWTime time, const Datum &data);
    
private:
    ScarabSession *session = nullptr;
    
};


class PythonMWK2Writer : public ExtensionType<PythonMWK2Writer> {
    
public:
    ~PythonMWK2Writer();
    
    void init(const std::string &filename, Py_ssize_t pageSize);
    
    void write_event(int code, MWTime time, const Datum &data);
    
    void begin_transaction();
    void commit_transaction();
    
private:
    void requireValidWriter() const;
    
    std::unique_ptr<MWK2Writer> writer;
    
};


END_NAMESPACE_MW_PYTHON


#endif /* !defined(PYTHON_DATA_BINDINGS_HELPERS_H_) */
