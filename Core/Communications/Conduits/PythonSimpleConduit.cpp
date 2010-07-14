/*
 *  PythonSimpleConduit.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 10/2/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "PythonSimpleConduit.h"

namespace mw{

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

PyObject *convert_datum_to_python(Datum datum){
    return convert_scarab_to_python(datum.getScarabDatum());
}
    
}
