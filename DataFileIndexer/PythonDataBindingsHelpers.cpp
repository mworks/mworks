/*
 *  PythonDataBindingsHelpers.cpp
 *  DataFileIndexer
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "PythonDataBindingsHelpers.h"

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
    }
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
 




double scarab_extract_float(ScarabDatum *datum){
#if __LITTLE_ENDIAN__
	return *((double *)(datum->data.opaque.data));
#else
	int i;
	unsigned char swap_buffer[sizeof(double)];
	unsigned char *datum_bytes = (unsigned char *)datum->data.opaque.data;
	for(i = 0; i < sizeof(double); i++){
		swap_buffer[i] = datum_bytes[sizeof(double) - i - 1];
	}
	
	return *((double *)swap_buffer);
#endif
}


// Convert a ScarabDatum into a corresponding Python object
PyObject *convert_scarab_to_python(ScarabDatum *datum){
    
    if(datum == NULL){
        return Py_None;
    }
    
    int n_items;
    
    PyObject *dict;
    ScarabDatum **keys, **values;
    PyObject *thelist, *key_py_obj, *value_py_obj;
    
    switch (datum->type){
        case(SCARAB_NULL):
            return Py_None;
        case(SCARAB_INTEGER):
            return PyInt_FromLong((long)datum->data.integer);
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
                key_py_obj = convert_scarab_to_python(keys[i]);
                value_py_obj = convert_scarab_to_python(values[i]);
                
                PyDict_SetItem(dict, key_py_obj, value_py_obj);
            }
            
            return dict;
            
        case(SCARAB_LIST):
            n_items = datum->data.list->size;
            thelist = PyList_New(n_items);
            
            for(int i=0; i < n_items; i++){
                PyList_SetItem(thelist, i, convert_scarab_to_python(datum->data.list->values[i]));
            }
            
            return thelist;
            
        case(SCARAB_OPAQUE):
            return PyString_FromString(scarab_extract_string(datum));
            
        default:
            return Py_None;
    }
}


PyObject *extract_event_value(EventWrapper e){
    if(e.empty()){
        // TODO throw / complain
        return Py_None;
    }
    return convert_scarab_to_python(e.getPayload());
}
