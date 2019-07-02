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


BEGIN_NAMESPACE_MW_PYTHON


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


static Datum convert_bytes_to_datum(const boost::python::object &obj) {
    char *buffer;
    Py_ssize_t size;
    if (PyBytes_AsStringAndSize(obj.ptr(), &buffer, &size))
        throw_error_already_set();
    return Datum(buffer, int(size));
}


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
    
#if PY_MAJOR_VERSION < 3
    } else if (PyInt_Check(pObj)) {  // Must come *after* PyBool_Check
        
        long l_val = PyInt_AsLong(pObj);
        if ((l_val == -1) && PyErr_Occurred())
            throw_error_already_set();
        
        return Datum(l_val);
#endif
        
    } else if (PyArray_IsScalar(pObj, Integer)) {
        
#if PY_MAJOR_VERSION < 3
        return convert_python_to_datum(manageNewRef(PyNumber_Int(pObj)));
#else
        return convert_python_to_datum(manageNewRef(PyNumber_Long(pObj)));
#endif
        
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
        
    } else if (PyBytes_Check(pObj)) {
        
        return convert_bytes_to_datum(obj);
        
    } else if (PyUnicode_Check(pObj)) {
        
        return convert_bytes_to_datum(manageNewRef( PyUnicode_AsUTF8String(pObj) ));
        
    } else if (PyTuple_Check(pObj)) {
        
        Py_ssize_t size = PyTuple_Size(pObj);  // Doesn't fail
        Datum::list_value_type items;
        
        for (Py_ssize_t i = 0; i < size; i++) {
            items.emplace_back(convert_python_to_datum(manageBorrowedRef( PyTuple_GetItem(pObj, i) )));
        }
        
        return Datum(std::move(items));
        
    } else if (PyList_Check(pObj)) {
        
        Py_ssize_t size = PyList_Size(pObj);  // Doesn't fail
        Datum::list_value_type items;
        
        for (Py_ssize_t i = 0; i < size; i++) {
            items.emplace_back(convert_python_to_datum(manageBorrowedRef( PyList_GetItem(pObj, i) )));
        }
        
        return Datum(std::move(items));
        
    } else if (PyDict_Check(pObj)) {
        
        Py_ssize_t pos = 0;
        PyObject *key = nullptr;
        PyObject *value = nullptr;
        Datum::dict_value_type items;
        
        while (PyDict_Next(pObj, &pos, &key, &value)) {
            items.emplace(convert_python_to_datum(manageBorrowedRef( key )),
                          convert_python_to_datum(manageBorrowedRef( value )));
        }
        
        return Datum(std::move(items));
        
    } else if (PyMapping_Check(pObj) &&
               PyObject_HasAttrString(pObj, "keys") &&
               PyObject_HasAttrString(pObj, "values"))
    {
        
        Py_ssize_t size = PyMapping_Size(pObj);
        if (size == -1)
            throw_error_already_set();
        
        Datum::dict_value_type items;
        
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"
        boost::python::object keys = manageNewRef( PyMapping_Keys(pObj) );
        boost::python::object values = manageNewRef( PyMapping_Values(pObj) );
#pragma clang diagnostic pop
        
        for (Py_ssize_t i = 0; i < size; i++) {
            items.emplace(convert_python_to_datum(manageBorrowedRef( PyList_GetItem(keys.ptr(), i) )),
                          convert_python_to_datum(manageBorrowedRef( PyList_GetItem(values.ptr(), i) )));
        }
        
        return Datum(std::move(items));
        
    } else if (PySequence_Check(pObj)) {  // Must come *after* PyMapping_Check
        
        Py_ssize_t size = PySequence_Size(pObj);
        if (size == -1)
            throw_error_already_set();
        
        Datum::list_value_type items;
        
        for (Py_ssize_t i = 0; i < size; i++) {
            items.emplace_back(convert_python_to_datum(manageNewRef( PySequence_GetItem(pObj, i) )));
        }
        
        return Datum(std::move(items));
        
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
            auto &str = datum.getString();
#if PY_MAJOR_VERSION >= 3
            if (datum.stringIsCString()) {
                return manageNewRef( PyUnicode_FromStringAndSize(str.c_str(), str.size()) );
            }
#endif
            return manageNewRef( PyBytes_FromStringAndSize(str.c_str(), str.size()) );
        }
            
        case M_LIST: {
            auto &listValue = datum.getList();
            boost::python::object list = manageNewRef( PyList_New(listValue.size()) );
            
            for (Py_ssize_t i = 0; i < listValue.size(); i++) {
                boost::python::object item = convert_datum_to_python(listValue.at(i));
                // PyList_SetItem "steals" the item reference, so we need to INCREF it
                Py_INCREF(item.ptr());
                if (PyList_SetItem(list.ptr(), i, item.ptr()))
                    throw_error_already_set();
            }
            
            return list;
        }
            
        case M_DICTIONARY: {
            boost::python::object dict = manageNewRef( PyDict_New() );
            
            for (auto &item : datum.getDict()) {
                if (PyDict_SetItem(dict.ptr(),
                                   convert_datum_to_python(item.first).ptr(),
                                   convert_datum_to_python(item.second).ptr()))
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
#if PY_MAJOR_VERSION < 3
    if (ll_val >= LONG_MIN && ll_val <= LONG_MAX) {
        return manageNewRef( PyInt_FromLong(long(ll_val)) );
    }
#endif
    return manageNewRef( PyLong_FromLongLong(ll_val) );
}


END_NAMESPACE_MW_PYTHON
