//
//  PythonDataHelpers.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 10/15/12.
//
//

#include "PythonDataHelpers.h"

#include <climits>
#include <cmath>


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
        if (std::isinf(value)) {
            datum = scarab_new_atomic();
            datum->type = SCARAB_FLOAT_INF;
        }
        else if (std::isnan(value)) {
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
    // Sequence (list, tuple, etc.)
    else if (PySequence_Check(pObj)) {
        int len = PySequence_Size(pObj);
        datum = scarab_list_new(len);

        for (int i = 0; i < len; i++) {
            PyObject *item = PySequence_GetItem(pObj, i);  // Returns a new reference
            ScarabDatum *list_element = convert_python_to_scarab(item);
            scarab_list_put(datum, i, list_element);
            scarab_free_datum(list_element);
            Py_DECREF(item);
        }
    }
    // Mapping (dict, etc.)
    else if (PyMapping_Check(pObj)) {
        int len = (int) PyMapping_Size(pObj);
        datum = scarab_dict_new(len, scarab_dict_times2);
        
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-writable-strings"
        PyObject *keys = PyMapping_Keys(pObj);
        PyObject *values = PyMapping_Values(pObj);
#pragma clang diagnostic pop

        for (int i = 0; i < len; i++) {
            ScarabDatum *key = convert_python_to_scarab(PyList_GetItem(keys, i));
            ScarabDatum *value = convert_python_to_scarab(PyList_GetItem(values, i));
            scarab_dict_put(datum, key, value);
            scarab_free_datum(key);
            scarab_free_datum(value);
        }
        
        Py_DECREF(keys);
        Py_DECREF(values);
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
    PyObject *thelist, *key_py_obj, *value_py_obj;
    
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
            keys = datum->data.dict->keys;
            values = datum->data.dict->values;
            
            for (int i = 0; i < datum->data.dict->tablesize; i++) {
                if (keys[i]) {
                    // convert the key and value
                    key_py_obj = convert_scarab_to_python(keys[i], SCARAB_DICT);
                    value_py_obj = convert_scarab_to_python(values[i]);
                    
                    PyDict_SetItem(dict, key_py_obj, value_py_obj);
                    
                    // PyDict_SetItem does *not* steal the key and value references, so we need to DECREF them
                    Py_DECREF(key_py_obj);
                    Py_DECREF(value_py_obj);
                }
            }
            
            return dict;
            
        case(SCARAB_LIST):
            n_items = datum->data.list->size;
            // Python Dictionaries cannot have lists as keys. Should be tuple.
            if (prev_type == SCARAB_DICT) thelist = PyTuple_New(n_items);
            else thelist = PyList_New(n_items);
            
            for(int i=0; i < n_items; i++){
                // PyTuple_SetItem and PyList_SetItem steal the item reference, so we don't need to DECREF it
                if (prev_type == SCARAB_DICT)
                    PyTuple_SetItem(thelist, i, convert_scarab_to_python(datum->data.list->values[i]));
                else
                    PyList_SetItem(thelist, i, convert_scarab_to_python(datum->data.list->values[i]));
            }
            
            return thelist;
            
        case(SCARAB_OPAQUE):
            s_val = (char *)(datum->data.opaque.data);
            s_size = datum->data.opaque.size;
            if (scarab_opaque_is_string(datum))
                s_size -= 1;  // PyString_FromStringAndSize doesn't expect a null-terminated string
            return PyString_FromStringAndSize(s_val, s_size);
            
        default:
            Py_RETURN_NONE;
    }
}

























