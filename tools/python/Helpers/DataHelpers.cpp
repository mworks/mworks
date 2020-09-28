//
//  DataHelpers.cpp
//
//  Created by Christopher Stawarz on 10/15/12.
//
//

#include "DataHelpers.h"


BEGIN_NAMESPACE_MW_PYTHON


static PyObject *numpyBoolType = nullptr;
static PyObject *numpyIntegerType = nullptr;
static PyObject *numpyFloatingType = nullptr;


void importNumpyTypes() {
    Py_CLEAR(numpyBoolType);
    Py_CLEAR(numpyIntegerType);
    Py_CLEAR(numpyFloatingType);
    auto numpyModule = ObjectPtr::owned(PyImport_ImportModule("numpy"));
    if (!numpyModule ||
        !(numpyBoolType = PyObject_GetAttrString(numpyModule.get(), "bool_")) ||
        !(numpyIntegerType = PyObject_GetAttrString(numpyModule.get(), "integer")) ||
        !(numpyFloatingType = PyObject_GetAttrString(numpyModule.get(), "floating")))
    {
        PyErr_Clear();
    }
}


static auto isNumpyInstance(PyObject *obj, PyObject *type) {
    bool result = false;
    if (type) {
        switch (PyObject_IsInstance(obj, type)) {
            case 1:
                result = true;
                break;
            case -1:
                PyErr_Clear();
                break;
            default:
                break;
        }
    }
    return result;
}


static auto convertBytesToDatum(const ObjectPtr &obj) {
    char *buffer;
    Py_ssize_t size;
    if (PyBytes_AsStringAndSize(obj.get(), &buffer, &size))
        throw ErrorAlreadySet();
    return Datum(buffer, int(size));
}


static auto getTypeName(PyObject *obj) {
    auto name = ObjectPtr::owned(PyObject_GetAttrString(reinterpret_cast<PyObject *>(Py_TYPE(obj)), "__qualname__"));
    if (name) {
        auto nameUTF8 = ObjectPtr::owned(PyUnicode_AsUTF8String(name.get()));
        if (nameUTF8) {
            char *buffer;
            Py_ssize_t size;
            if (!PyBytes_AsStringAndSize(nameUTF8.get(), &buffer, &size)) {
                return std::string(buffer, size);
            }
        }
    }
    PyErr_Clear();
    return std::string("UNKNOWN");
}


Datum convertObjectToDatum(const ObjectPtr &obj) {
    ScopedRecursionGuard srg(" while converting Python object to MWorks datum");
    
    if (!obj) {
        throw ErrorAlreadySet();
    }
    
    if (obj.isNone()) {
        return Datum();
    }
    
    PyObject *pObj = obj.get();
    
    if (PyBool_Check(pObj)) {
        
        return Datum(bool(pObj == Py_True));
        
    } else if (isNumpyInstance(pObj, numpyBoolType)) {
        
        return Datum(bool(PyObject_IsTrue(pObj)));
        
    } else if (isNumpyInstance(pObj, numpyIntegerType)) {
        
        return convertObjectToDatum(ObjectPtr::owned(PyNumber_Long(pObj)));
        
    } else if (PyLong_Check(pObj)) {
        
        long long ll_val = PyLong_AsLongLong(pObj);
        if ((ll_val == -1) && PyErr_Occurred())
            throw ErrorAlreadySet();
        
        return Datum(ll_val);
        
    } else if (PyFloat_Check(pObj)) {
        
        double value = PyFloat_AsDouble(pObj);
        if ((value == -1.0) && PyErr_Occurred())
            throw ErrorAlreadySet();
        
        return Datum(value);
        
    } else if (isNumpyInstance(pObj, numpyFloatingType)) {
        
        return convertObjectToDatum(ObjectPtr::owned(PyNumber_Float(pObj)));
        
    } else if (PyBytes_Check(pObj)) {
        
        return convertBytesToDatum(obj);
        
    } else if (PyUnicode_Check(pObj)) {
        
        return convertBytesToDatum(ObjectPtr::created(PyUnicode_AsUTF8String(pObj)));
        
    } else if (PyTuple_Check(pObj)) {
        
        Py_ssize_t size = PyTuple_Size(pObj);  // Doesn't fail
        Datum::list_value_type items;
        
        for (Py_ssize_t i = 0; i < size; i++) {
            items.emplace_back(convertObjectToDatum(ObjectPtr::borrowed(PyTuple_GetItem(pObj, i))));
        }
        
        return Datum(std::move(items));
        
    } else if (PyList_Check(pObj)) {
        
        Py_ssize_t size = PyList_Size(pObj);  // Doesn't fail
        Datum::list_value_type items;
        
        for (Py_ssize_t i = 0; i < size; i++) {
            items.emplace_back(convertObjectToDatum(ObjectPtr::borrowed(PyList_GetItem(pObj, i))));
        }
        
        return Datum(std::move(items));
        
    } else if (PyDict_Check(pObj)) {
        
        Py_ssize_t pos = 0;
        PyObject *key = nullptr;
        PyObject *value = nullptr;
        Datum::dict_value_type items;
        
        while (PyDict_Next(pObj, &pos, &key, &value)) {
            items.emplace(convertObjectToDatum(ObjectPtr::borrowed(key)),
                          convertObjectToDatum(ObjectPtr::borrowed(value)));
        }
        
        return Datum(std::move(items));
        
    } else if (PyMapping_Check(pObj) &&
               PyObject_HasAttrString(pObj, "keys") &&
               PyObject_HasAttrString(pObj, "values"))
    {
        
        Py_ssize_t size = PyMapping_Size(pObj);
        if (size == -1)
            throw ErrorAlreadySet();
        
        Datum::dict_value_type items;
        auto keys = ObjectPtr::created(PyMapping_Keys(pObj));
        auto values = ObjectPtr::created(PyMapping_Values(pObj));
        
        for (Py_ssize_t i = 0; i < size; i++) {
            items.emplace(convertObjectToDatum(ObjectPtr::borrowed(PyList_GetItem(keys.get(), i))),
                          convertObjectToDatum(ObjectPtr::borrowed(PyList_GetItem(values.get(), i))));
        }
        
        return Datum(std::move(items));
        
    } else if (PySequence_Check(pObj)) {  // Must come *after* PyMapping_Check
        
        Py_ssize_t size = PySequence_Size(pObj);
        if (size == -1)
            throw ErrorAlreadySet();
        
        Datum::list_value_type items;
        
        for (Py_ssize_t i = 0; i < size; i++) {
            items.emplace_back(convertObjectToDatum(ObjectPtr::owned(PySequence_GetItem(pObj, i))));
        }
        
        return Datum(std::move(items));
        
    }
    
    PyErr_Format(PyExc_TypeError, "Cannot convert object of type %s", getTypeName(pObj).c_str());
    throw ErrorAlreadySet();
}


ObjectPtr convertDatumToObject(const Datum &datum) {
    ScopedRecursionGuard srg(" while converting MWorks datum to Python object");
    
    if (datum.isUndefined()) {
        return ObjectPtr::none();
    }
    
    switch (datum.getDataType()) {
        case M_INTEGER:
            return asObject(datum.getInteger());
            
        case M_FLOAT:
            return asObject(datum.getFloat());
            
        case M_BOOLEAN:
            return asObject(datum.getBool());
            
        case M_STRING: {
            auto &str = datum.getString();
            if (datum.stringIsCString()) {
                return asObject(str);
            }
            return ObjectPtr::created(PyBytes_FromStringAndSize(str.c_str(), str.size()));
        }
            
        case M_LIST: {
            auto &listValue = datum.getList();
            auto list = ObjectPtr::created(PyList_New(listValue.size()));
            
            for (Py_ssize_t i = 0; i < listValue.size(); i++) {
                auto item = convertDatumToObject(listValue.at(i));
                // PyList_SetItem "steals" the item reference, so we need to INCREF it
                Py_INCREF(item.get());
                if (PyList_SetItem(list.get(), i, item.get()))
                    throw ErrorAlreadySet();
            }
            
            return list;
        }
            
        case M_DICTIONARY: {
            auto dict = ObjectPtr::created(PyDict_New());
            
            for (auto &item : datum.getDict()) {
                if (PyDict_SetItem(dict.get(),
                                   convertDatumToObject(item.first).get(),
                                   convertDatumToObject(item.second).get()))
                {
                    throw ErrorAlreadySet();
                }
            }
            
            return dict;
        }
            
        default:
            PyErr_Format(PyExc_TypeError, "Cannot convert Datum of unknown type (%d)", datum.getDataType());
            throw ErrorAlreadySet();
    }
}


END_NAMESPACE_MW_PYTHON
