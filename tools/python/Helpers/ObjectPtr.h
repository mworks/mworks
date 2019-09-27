//
//  ObjectPtr.h
//  PythonTools
//
//  Created by Christopher Stawarz on 10/1/19.
//  Copyright © 2019 MWorks Project. All rights reserved.
//

#ifndef ObjectPtr_h
#define ObjectPtr_h

#include "Utilities.h"


BEGIN_NAMESPACE_MW_PYTHON


class ObjectPtr {
    
public:
    static ObjectPtr created(PyObject *obj) {
        if (!obj) throw ErrorAlreadySet();
        return owned(obj);
    }
    
    static ObjectPtr owned(PyObject *obj) noexcept {
        return ObjectPtr(obj, false);  // Already owned, so don't incref
    }
    
    static ObjectPtr borrowed(PyObject *obj) noexcept {
        return ObjectPtr(obj, true);   // Borrowed, so incref to establish ownership
    }
    
    static ObjectPtr none() noexcept {
        return borrowed(Py_None);
    }
    
    ~ObjectPtr() {
        Py_XDECREF(obj);
    }
    
    constexpr ObjectPtr() noexcept :
        obj(nullptr)
    { }
    
    // Copy constructor
    ObjectPtr(const ObjectPtr &other) noexcept :
        obj(other.obj)
    {
        Py_XINCREF(obj);
    }
    
    // Move constructor
    ObjectPtr(ObjectPtr &&other) noexcept :
        obj(other.obj)
    {
        other.obj = nullptr;
    }
    
    // Copy assignment
    ObjectPtr& operator=(const ObjectPtr &other) noexcept {
        if (this != &other) {
            Py_XDECREF(obj);
            obj = other.obj;
            Py_XINCREF(obj);
        }
        return (*this);
    }
    
    // Move assignment
    ObjectPtr& operator=(ObjectPtr &&other) noexcept {
        std::swap(obj, other.obj);
        return (*this);
    }
    
    PyObject* get() const noexcept {
        return obj;
    }
    
    [[nodiscard]] PyObject* release() noexcept {
        PyObject* _obj = obj;
        obj = nullptr;
        return _obj;
    }
    
    void reset() noexcept {
        (*this) = ObjectPtr();
    }
    
    explicit operator bool() const noexcept {
        return obj;
    }
    
private:
    ObjectPtr(PyObject *obj, bool incref) noexcept :
        obj(obj)
    {
        if (incref) Py_XINCREF(obj);
    }
    
    PyObject *obj;
    
};


END_NAMESPACE_MW_PYTHON


#endif /* ObjectPtr_h */
