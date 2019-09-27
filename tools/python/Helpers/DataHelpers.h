//
//  DataHelpers.h
//
//  Created by Christopher Stawarz on 10/15/12.
//
//

#ifndef DataHelpers_h_
#define DataHelpers_h_

#include "ObjectPtr.h"


BEGIN_NAMESPACE_MW_PYTHON


void importNumpyTypes();


// Convert a Python object into a MWorks Datum
Datum convertObjectToDatum(const ObjectPtr &obj);

// Convert a MWorks Datum into a Python object
ObjectPtr convertDatumToObject(const Datum &datum);


template<typename T, std::enable_if_t<std::is_same_v<ObjectPtr, remove_cvref_t<T>>, int> = 0>
inline auto asObject(T &&val) {
    return std::forward<ObjectPtr>(val);
}

template<typename T>
inline ObjectPtr asObject(T *) {
    static_assert(!std::is_same_v<T, T>, "asObject cannot be used with pointer types");
}

inline ObjectPtr asObject(bool val) {
    return ObjectPtr::created(PyBool_FromLong(val));
}

inline ObjectPtr asObject(int val) {
    return ObjectPtr::created(PyLong_FromLong(val));
}

inline ObjectPtr asObject(std::size_t val) {
    return ObjectPtr::created(PyLong_FromSize_t(val));
}

inline ObjectPtr asObject(long long val) {
    return ObjectPtr::created(PyLong_FromLongLong(val));
}

inline ObjectPtr asObject(double val) {
    return ObjectPtr::created(PyFloat_FromDouble(val));
}

inline ObjectPtr asObject(const std::string &val) {
    return ObjectPtr::created(PyUnicode_FromStringAndSize(val.data(), val.size()));
}

template<typename Key, typename T>
inline ObjectPtr asObject(const std::map<Key, T> &val) {
    auto obj = ObjectPtr::created(PyDict_New());
    for (auto &item : val) {
        if (PyDict_SetItem(obj.get(), asObject(item.first).get(), asObject(item.second).get())) {
            throw ErrorAlreadySet();
        }
    }
    return obj;
}

inline ObjectPtr asObject(const Datum &val) {
    return convertDatumToObject(val);
}


END_NAMESPACE_MW_PYTHON


#endif /* !defined(DataHelpers_h_) */
