//
//  PythonDataHelpers.cpp
//
//  Created by Christopher Stawarz on 10/15/12.
//
//

#include "PythonDataHelpers.h"

#define NO_IMPORT_ARRAY
#include <numpy/arrayobject.h>

using boost::python::throw_error_already_set;


BEGIN_NAMESPACE_MW


class ScopedRecursionGuard : boost::noncopyable {
    
public:
    explicit ScopedRecursionGuard(const std::string &where) {
        if (Py_EnterRecursiveCall(const_cast<char *>(where.c_str()))) {
            throw_error_already_set();
        }
    }
    
    ~ScopedRecursionGuard() {
        Py_LeaveRecursiveCall();
    }
    
};


Datum convert_python_to_datum(const boost::python::object &obj) {
    ScopedRecursionGuard srg(" while converting Python object to MWorks datum");
    
    if (obj.is_none()) {
        return Datum();
    }
    
    PyObject *pObj = obj.ptr();
    
    if (PyBool_Check(pObj)) {
        
        return Datum(bool(pObj == Py_True));
        
    } else if (PyArray_IsScalar(pObj, Bool)) {
        
        return Datum(bool(PyObject_IsTrue(pObj)));
    
    } else if (PyInt_Check(pObj)) {  // Must come *after* PyBool_Check
        
        long l_val = PyInt_AsLong(pObj);
        if ((l_val == -1) && PyErr_Occurred())
            throw_error_already_set();
        
        return Datum(l_val);
        
    } else if (PyArray_IsScalar(pObj, Integer)) {
        
        return convert_python_to_datum(manageNewRef(PyNumber_Int(pObj)));
        
    } else if (PyLong_Check(pObj)) {
        
        long long ll_val = PyLong_AsLongLong(pObj);
        if ((ll_val == -1) && PyErr_Occurred())
            throw_error_already_set();
        
        return Datum(ll_val);
        
    } else if (PyFloat_Check(pObj)) {
        
        double value = PyFloat_AsDouble(pObj);
        if ((value == -1.0) && PyErr_Occurred())
            throw_error_already_set();
        
        return Datum(value);
        
    } else if (PyArray_IsScalar(pObj, Floating)) {
        
        return convert_python_to_datum(manageNewRef(PyNumber_Float(pObj)));
        
    } else if (PyString_Check(pObj) || PyUnicode_Check(pObj)) {
        
        boost::python::object string;
        if (!PyUnicode_Check(pObj)) {
            string = obj;
        } else {
            string = manageNewRef( PyUnicode_AsUTF8String(pObj) );
        }
        
        char *buffer;
        Py_ssize_t size;
        if (PyString_AsStringAndSize(string.ptr(), &buffer, &size))
            throw_error_already_set();
        
        // If the buffer contains no NUL's (meaning it's probably text, not binary data), include
        // the NUL terminator in the resulting Datum
        if (std::memchr(buffer, 0, size) == NULL)
            size++;
        
        return Datum(buffer, int(size));
        
    } else if (PyMapping_Check(pObj)) {
        
        int size = int(PyMapping_Size(pObj));
        if (size == -1)
            throw_error_already_set();
        
        Datum dict(M_DICTIONARY, size);
        
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"
        boost::python::object keys = manageNewRef( PyMapping_Keys(pObj) );
        boost::python::object values = manageNewRef( PyMapping_Values(pObj) );
#pragma clang diagnostic pop

        for (int i = 0; i < size; i++) {
            dict.addElement(convert_python_to_datum(manageBorrowedRef( PyList_GetItem(keys.ptr(), i) )),
                            convert_python_to_datum(manageBorrowedRef( PyList_GetItem(values.ptr(), i) )));
        }
        
        return dict;
        
    } else if (PySequence_Check(pObj)) {  // Must come *after* PyString_Check, PyUnicode_Check, and PyMapping_Check
        
        int size = int(PySequence_Size(pObj));
        if (size == -1)
            throw_error_already_set();
        
        Datum list(M_LIST, size);
        
        for (int i = 0; i < size; i++) {
            list.setElement(i, convert_python_to_datum(manageNewRef(PySequence_GetItem(pObj, i))));
        }
        
        return list;
    }
    
    PyErr_Format(PyExc_TypeError, "Cannot convert object of type %s", pObj->ob_type->tp_name);
    throw_error_already_set();
    return Datum();  // Never reached
}


boost::python::object convert_datum_to_python(const Datum &datum) {
    ScopedRecursionGuard srg(" while converting MWorks datum to Python object");
    
    if (datum.isUndefined()) {
        return boost::python::object();
    }
    
    switch (datum.getDataType()) {
        case M_INTEGER:
            return convert_longlong_to_python(datum.getInteger());
            
        case M_FLOAT:
            return manageNewRef( PyFloat_FromDouble(datum.getFloat()) );
            
        case M_BOOLEAN:
            return manageNewRef( PyBool_FromLong(datum.getBool()) );
            
        case M_STRING: {
            const char *data = datum.getString();
            int size = datum.getStringLength();
            if (datum.stringIsCString())
                size -= 1;  // PyString_FromStringAndSize doesn't expect a NUL-terminated string
            return manageNewRef( PyString_FromStringAndSize(data, size) );
        }
            
        case M_LIST: {
            int size = datum.getNElements();
            
            boost::python::object list = manageNewRef( PyList_New(size) );
            
            for (int i = 0; i < size; i++) {
                boost::python::object item = convert_datum_to_python(datum.getElement(i));
                // PyList_SetItem "steals" the item reference, so we need to INCREF it
                Py_INCREF(item.ptr());
                if (PyList_SetItem(list.ptr(), i, item.ptr()))
                    throw_error_already_set();
            }
            
            return list;
        }
            
        case M_DICTIONARY: {
            boost::python::object dict = manageNewRef( PyDict_New() );
            
            std::vector<Datum> keys = datum.getKeys();
            
            BOOST_FOREACH( const Datum &key, keys ) {
                if (PyDict_SetItem(dict.ptr(),
                                   convert_datum_to_python(key).ptr(),
                                   convert_datum_to_python(datum.getElement(key)).ptr()))
                {
                    throw_error_already_set();
                }
            }
            
            return dict;
        }
            
        default:
            PyErr_Format(PyExc_TypeError, "Cannot convert Datum of unknown type (%d)", datum.getDataType());
            throw_error_already_set();
            return boost::python::object();  // Never reached
    }
}


boost::python::object convert_longlong_to_python(long long ll_val) {
    if (ll_val >= LONG_MIN && ll_val <= LONG_MAX) {
        return manageNewRef( PyInt_FromLong(long(ll_val)) );
    }
    return manageNewRef( PyLong_FromLongLong(ll_val) );
}


END_NAMESPACE_MW


























