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

MWorksTime PythonDataFile::minimum_time(){ return indexer->getMinimumTime(); }
MWorksTime PythonDataFile::maximum_time(){ return indexer->getMaximumTime(); }

std::vector<EventWrapper> PythonDataFile::test_function(int number){
    
    std::vector<EventWrapper> returnval;
    
    printf("Booyah");
    return returnval;
}


std::vector<EventWrapper> PythonDataFile::fetch_all_events(){
    std::vector<unsigned int> no_codes;
    if(indexer != NULL){
        return indexer->events(no_codes, MIN_MONKEY_WORKS_TIME(), MAX_MONKEY_WORKS_TIME());
    } else {
        std::vector<EventWrapper> empty;
        return empty;
    }
}

std::vector<EventWrapper> PythonDataFile::fetch_events1(bp::list codes){
    return fetch_events3(codes, MIN_MONKEY_WORKS_TIME(), MAX_MONKEY_WORKS_TIME());
}

std::vector<EventWrapper> PythonDataFile::fetch_events2(bp::list codes, const MWorksTime lower_bound){
    return fetch_events3(codes, lower_bound, MAX_MONKEY_WORKS_TIME());
}


std::vector<EventWrapper> PythonDataFile::fetch_events3(bp::list codes,
                                        const MWorksTime lower_bound, 
                                        const MWorksTime upper_bound){
    
    //   std::vector<EventWrapper> fetch_events(bp::list codes,
    //                                           long lower_bound = 0, 
    //                                           long upper_bound = 999999999L){
    //        
    std::vector<unsigned int> event_codes;
    
    int n = len(codes);
    
        
    for(int i = 0; i < n; i++){
        event_codes.push_back(bp::extract<unsigned int>(codes[i]));
    }
    
    //printf("fetching events from %lld to %lld\n", lower_bound, upper_bound); 
    
    if(indexer != NULL){
        return indexer->events(event_codes, lower_bound, upper_bound);
    } else {
        std::vector<EventWrapper> empty;
        return empty;
    }
}



PythonDataStream::PythonDataStream(std::string _uri){
    uri = _uri;
    session = NULL;
}

void PythonDataStream::open(){
    
    // TODO: verify it is safe to remove Ben's kludge here
    char *uri_temp = new char[uri.length() + 1];
    strncpy(uri_temp, uri.c_str(), uri.length() + 1);
    
    //std::cerr << "Opening file: " << uri_temp << "..." << std::endl;
    session = scarab_session_connect(uri_temp);
    delete [] uri_temp;
    
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

// Convert a Python object into a ScarabDatum
ScarabDatum *convert_python_to_scarab(PyObject *pObj) { 
    ScarabDatum *datum;

    // None of NULL
    if (pObj == NULL) {
        datum = scarab_new_atomic();
        datum->type = SCARAB_NULL;
    }
    // Integer
    else if (PyInt_Check(pObj)) { 
        datum = scarab_new_integer(PyInt_AsLong(pObj));
    }
    // Long
    else if (PyLong_Check(pObj)) { 
        datum = scarab_new_integer((long long) PyLong_AsLongLong(pObj));
    }
    // Float
    else if (PyFloat_Check(pObj)) {
        double value = PyFloat_AsDouble(pObj);
        if (isinf(value)) {
            datum = scarab_new_atomic();
            datum->type = SCARAB_FLOAT_INF;
        }
        else if (isnan(value)) {
            datum = scarab_new_atomic();
            datum->type = SCARAB_FLOAT_NAN;
        }
        else datum = scarab_new_float(value);
    }
    // String
    else if (PyString_Check(pObj)) {
        char *buf; Py_ssize_t len;
        PyString_AsStringAndSize(pObj, &buf, &len);
        datum = scarab_new_opaque(buf, (int) len + 1 /* note: buf is NULL terminated*/);
    }
    // List/Tuple
    // TODO: Currently tuples are handled exactly same as lists. 
    // Might be better to use scarab->attributes to indicate tuples.
    else if (PyList_Check(pObj) || PyTuple_Check(pObj)) {
        int bList = PyList_Check(pObj);
        int len = (bList) ? PyList_Size(pObj) : PyTuple_Size(pObj);
        datum = scarab_list_new(len);

        for (int i = 0; i < len; i++) {
            ScarabDatum *list_element;
            if (bList) list_element = convert_python_to_scarab(PyList_GetItem(pObj, i));
            else list_element = convert_python_to_scarab(PyTuple_GetItem(pObj, i));
            scarab_list_put(datum, i, list_element);
            scarab_free_datum(list_element);
        }
    }
    // Dictionary
    else if (PyDict_Check(pObj)) {
        PyObject *pKey;
        PyObject *pValue;
        Py_ssize_t pos = 0;
        int len = (int) PyDict_Size(pObj);
        datum = scarab_dict_new(len, scarab_dict_times2);

        while (PyDict_Next(pObj, &pos, &pKey, &pValue)) {
            ScarabDatum *key  = convert_python_to_scarab(pKey);
            ScarabDatum *value = convert_python_to_scarab(pValue);
            scarab_dict_put(datum, key, value);
            scarab_free_datum(key);
            scarab_free_datum(value);
        }
    }
    // Every else
    else {
        datum = scarab_new_atomic();
        datum->type = SCARAB_NULL;
    }
    return datum;
}


// Convert a ScarabDatum into a corresponding Python object
PyObject *convert_scarab_to_python(ScarabDatum *datum, int prev_type /* = -1*/){
    
    if(datum == NULL){
        Py_RETURN_NONE;
    }
    
    int n_items;
    long long l_val;
    char *s_val;
    int s_size;
    
    PyObject *dict;
    ScarabDatum **keys, **values;
    PyObject *thelist, *key_py_obj, *value_py_obj, *string_py_obj;
    
    switch (datum->type){
        case(SCARAB_NULL):
            Py_RETURN_NONE;
        case(SCARAB_INTEGER):
            l_val = datum->data.integer;
            if (l_val > (long long) LONG_MAX)
                return PyLong_FromLongLong(l_val);
            else
	            return PyInt_FromLong((long) l_val);
        case(SCARAB_FLOAT):
            return PyFloat_FromDouble((double)datum->data.floatp);
        case(SCARAB_FLOAT_INF):
            return PyFloat_FromDouble(INFINITY);
        case(SCARAB_FLOAT_NAN):
            return PyFloat_FromDouble(NAN);
        case(SCARAB_FLOAT_OPAQUE):
            return PyFloat_FromDouble(scarab_extract_float(datum));
        case(SCARAB_DICT):
            dict = PyDict_New();
            n_items = scarab_dict_number_of_elements(datum);
            keys = scarab_dict_keys(datum);
            values = scarab_dict_values(datum);
            
            for(int i = 0; i < n_items; i++){
                // convert the key
                key_py_obj = convert_scarab_to_python(keys[i], SCARAB_DICT);
                value_py_obj = convert_scarab_to_python(values[i]);
                
                PyDict_SetItem(dict, key_py_obj, value_py_obj);
            }
            
            return dict;
            
        case(SCARAB_LIST):
            n_items = datum->data.list->size;
            // Python Dictionaries cannot have lists as keys. Should be tuple.
            if (prev_type == SCARAB_DICT) thelist = PyTuple_New(n_items);
            else thelist = PyList_New(n_items);
            
            for(int i=0; i < n_items; i++){
                if (prev_type == SCARAB_DICT)
                    PyTuple_SetItem(thelist, i, convert_scarab_to_python(datum->data.list->values[i]));
                else
                    PyList_SetItem(thelist, i, convert_scarab_to_python(datum->data.list->values[i]));
            }
            
            return thelist;
            
        case(SCARAB_OPAQUE):
            s_val = scarab_extract_opaque(datum, &s_size);
            if (s_val[s_size - 1] == '\0')
                s_size -= 1;  // PyString_FromStringAndSize doesn't expect a null-terminated string
            string_py_obj = PyString_FromStringAndSize(s_val, s_size);
            free(s_val);
            return string_py_obj;
            
        default:
            Py_RETURN_NONE;
    }
}


PyObject *extract_event_value(EventWrapper e){
    if(e.empty()){
        // TODO throw / complain
        Py_RETURN_NONE;
    }
    return convert_scarab_to_python(e.getPayload());
}
