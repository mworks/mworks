/*
 *  DataFile.h
 *  DataFileIndexer
 *
 *  Created by David Cox on 8/17/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include <Scarab/scarab.h>
#include <Scarab/scarab_utilities.h>
#include <boost/python.hpp>
#include "dfindex.h"
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <python.h>

#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

using namespace boost;
using namespace boost::python;


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


class PythonDataFile {
    std::string file_name;
    shared_ptr<dfindex> indexer;
    
    
public:
    
    PythonDataFile(std::string _file_name){
        file_name = _file_name;
    }
    
    void open(){
        
        // convert the filename string to a boost::filesystem path
        // TODO: catch exception
        boost::filesystem::path file_path(file_name);
        
        // open the file
        indexer = shared_ptr<dfindex>(new dfindex(file_path));
    }
    
    void close(){
        // close it
        indexer = shared_ptr<dfindex>();
    }
    
    bool exists(){
        return true;
    }
    
    bool loaded(){
        return (indexer != NULL);
    }
    
    bool valid(){
        return (indexer != NULL);
    }
    
    MonkeyWorksTime minimum_time(){ return indexer->getMinimumTime(); }
    MonkeyWorksTime maximum_time(){ return indexer->getMaximumTime(); }
    
    std::vector<EventWrapper> test_function(int number){
    
        std::vector<EventWrapper> returnval;
        
        printf("Booyah");
        return returnval;
    }
    
    
    std::vector<EventWrapper> fetch_events1(list codes){
        return fetch_events3(codes, MIN_MONKEY_WORKS_TIME(), MAX_MONKEY_WORKS_TIME());
    }
    
    std::vector<EventWrapper> fetch_events2(list codes, const MonkeyWorksTime lower_bound){
        return fetch_events3(codes, lower_bound, MAX_MONKEY_WORKS_TIME());
    }
    
    
    std::vector<EventWrapper> fetch_events3(list codes,
                                      const MonkeyWorksTime lower_bound, 
                                      const MonkeyWorksTime upper_bound){
    
 //   std::vector<EventWrapper> fetch_events(list codes,
//                                           long lower_bound = 0, 
//                                           long upper_bound = 999999999L){
//        
        std::vector<unsigned int> event_codes;
        
        int n = len(codes);
        for(int i = 0; i < n; i++){
            event_codes.push_back(extract<unsigned int>(codes[i]));
        }

        printf("fetching events from %lld to %lld\n", lower_bound, upper_bound); 
        return indexer->events(event_codes, lower_bound, upper_bound);
    }
};


// Convert a ScarabDatum into a corresponding Python object
PyObject *convert_scarab_to_python(ScarabDatum *datum){
    
    if(datum == NULL){
        return Py_None;
    }
    
    int n_items;
    
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
            PyObject *dict = PyDict_New();
            n_items = scarab_dict_number_of_elements(datum);
            ScarabDatum **keys = scarab_dict_keys(datum);
            ScarabDatum **values = scarab_dict_values(datum);
            
            for(int i = 0; i < n_items; i++){
                // convert the key
                PyObject *key_py_obj = convert_scarab_to_python(keys[i]);
                PyObject *value_py_obj = convert_scarab_to_python(values[i]);
                
                PyDict_SetItem(dict, key_py_obj, value_py_obj);
            }
            
            return dict;
        
        case(SCARAB_LIST):
            n_items = datum->data.list->size;
            PyObject *thelist = PyList_New(n_items);
            
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
    return convert_scarab_to_python(e.getPayload());
}




//struct eventvector2pysequence{
//    
//    typedef boost::tuples::tuple< float, float, float> colour_tuple_type;
//    
//    typedef bpl::from_py_sequence< colour_tuple_type > converter_type;
//    
//    static void* convertible(PyObject* obj){
//        return converter_type::convertible( obj );
//    }
//    
//    static void
//    construct( PyObject* obj, bpl::converter::rvalue_from_python_stage1_data* data){
//        typedef bpl::converter::rvalue_from_python_storage<colour_t> colour_storage_t;
//        colour_storage_t* the_storage = reinterpret_cast<colour_storage_t*>( data );
//        void* memory_chunk = the_storage->storage.bytes;
//        
//        float red(0.0), green(0.0), blue(0.0);
//        boost::tuples::tie(red, green, blue) = converter_type::to_c_tuple( obj );
//        
//        colour_t* colour = new (memory_chunk) colour_t(red, green, blue);
//        data->convertible = memory_chunk;
//    }
//};
//
//void register_pytuple2colour(){
//    converter::registry::push_back(  &pytuple2colour::convertible
//                                        , &pytuple2colour::construct
//                                        , bpl::type_id<colour_t>() );
//}


// Create a module called "_data"; a pure python module ("data") will load this and add some
// more pythonic fixin's on top
BOOST_PYTHON_MODULE(_data)
{
    
    PyEval_InitThreads();
 
    //def("convert_scarab_to_python", convert_scarab_to_python, "Convert a Scarab datum to a corresponding Python object, recursing as needed");

    
    class_<EventWrapper, boost::noncopyable>("Event", init<ScarabDatum *>())
    .add_property("code", &EventWrapper::getEventCode)
    .add_property("time", &EventWrapper::getTime)
    .add_property("value", extract_event_value)
    ;
    
    class_<std::vector<EventWrapper> >("EventWrapperArray")
        .def(vector_indexing_suite< std::vector<EventWrapper> >())
    ;
    
    class_< std::vector<unsigned int> >("unsigned_int_vector")
        .def(vector_indexing_suite< std::vector<unsigned int> >())
    ;
    
    class_<PythonDataFile>("MWKFile", init<std::string>())
        .def("open", &PythonDataFile::open)
        .def("close", &PythonDataFile::close)
        .def("__fetch_events", &PythonDataFile::fetch_events1)
        .def("__fetch_events", &PythonDataFile::fetch_events2)
        .def("__fetch_events", &PythonDataFile::fetch_events3)
        .def("__test_function", &PythonDataFile::test_function)
        .add_property("exists", &PythonDataFile::exists)
        .add_property("loaded", &PythonDataFile::loaded)
        .add_property("valid", &PythonDataFile::valid)
        .add_property("minimum_time", &PythonDataFile::minimum_time)
        .add_property("maximum_time", &PythonDataFile::maximum_time)
        ;
    

    
    
}
