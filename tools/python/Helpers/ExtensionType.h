//
//  ExtensionType.h
//  PythonTools
//
//  Created by Christopher Stawarz on 10/7/19.
//  Copyright © 2019 MWorks Project. All rights reserved.
//

#ifndef ExtensionType_h
#define ExtensionType_h

#include "ArgTuple.h"


BEGIN_NAMESPACE_MW_PYTHON


template<typename T>
struct ExtensionType;


BEGIN_NAMESPACE(method_traits)


template<typename T> struct types;


// static member function or non-member function
template<typename R, typename... Args>
struct types<R (*)(Args...)> {
    using result = R;
    using instance = void;
    using args = std::tuple<remove_cvref_t<Args>...>;
};


// non-const member function
template<typename R, typename T, typename... Args>
struct types<R (T::*)(Args...)> {
    using result = R;
    using instance = T;
    using args = std::tuple<remove_cvref_t<Args>...>;
};


// const member function
template<typename R, typename T, typename... Args>
struct types<R (T::*)(Args...) const> {
    using result = R;
    using instance = T;
    using args = std::tuple<remove_cvref_t<Args>...>;
};


template<auto f>
using result_type = typename types<decltype(f)>::result;

template<auto f>
using instance_type = typename types<decltype(f)>::instance;

template<auto f>
using args_type = typename types<decltype(f)>::args;

template<auto f>
constexpr auto args_size = std::tuple_size_v<args_type<f>>;

template<auto f>
constexpr auto is_void = std::is_same_v<result_type<f>, void>;

template<auto f>
constexpr auto is_static = std::is_same_v<instance_type<f>, void>;

template<auto f>
constexpr auto has_args = bool(args_size<f>);

template<auto f>
constexpr auto flags = (has_args<f> ? METH_VARARGS : METH_NOARGS);


template<auto f, std::size_t... I>
auto _call(const ArgTuple &args, std::index_sequence<I...>) {
    args_type<f> targets;
    args.parse(std::get<I>(targets)...);
    return f(std::get<I>(std::move(targets))...);
}


template<auto f>
auto call(const ArgTuple &args) {
    return _call<f>(args, std::make_index_sequence<args_size<f>>());
}


template<auto f, typename T, std::size_t... I>
auto _call(T &instance, const ArgTuple &args, std::index_sequence<I...>) {
    args_type<f> targets;
    args.parse(std::get<I>(targets)...);
    return (instance.*f)(std::get<I>(std::move(targets))...);
}


template<auto f, typename T>
auto call(T &instance, const ArgTuple &args) {
    return _call<f>(instance, args, std::make_index_sequence<args_size<f>>());
}


template<auto f, std::enable_if_t<!is_void<f> && is_static<f> && has_args<f>, int> = 0>
PyObject * callMethod(PyObject *self, PyObject *args) {
    return asObject(call<f>(ArgTuple(args))).release();
}


template<auto f, std::enable_if_t<is_void<f> && is_static<f> && has_args<f>, int> = 0>
PyObject * callMethod(PyObject *self, PyObject *args) {
    call<f>(ArgTuple(args));
    Py_RETURN_NONE;
}


template<auto f, std::enable_if_t<!is_void<f> && is_static<f> && !has_args<f>, int> = 0>
PyObject * callMethod(PyObject *self, PyObject *args) {
    return asObject(f()).release();
}


template<auto f, std::enable_if_t<is_void<f> && is_static<f> && !has_args<f>, int> = 0>
PyObject * callMethod(PyObject *self, PyObject *args) {
    f();
    Py_RETURN_NONE;
}


template<auto f, std::enable_if_t<std::is_base_of_v<ExtensionType<instance_type<f>>, instance_type<f>>, int> = 0>
auto & getInstance(PyObject *self) {
    return ExtensionType<instance_type<f>>::getInstance(self);
}


template<auto f, std::enable_if_t<!is_void<f> && !is_static<f> && has_args<f>, int> = 0>
PyObject * callMethod(PyObject *self, PyObject *args) {
    return asObject(call<f>(getInstance<f>(self), ArgTuple(args))).release();
}


template<auto f, std::enable_if_t<is_void<f> && !is_static<f> && has_args<f>, int> = 0>
PyObject * callMethod(PyObject *self, PyObject *args) {
    call<f>(getInstance<f>(self), ArgTuple(args));
    Py_RETURN_NONE;
}


template<auto f, std::enable_if_t<!is_void<f> && !is_static<f> && !has_args<f>, int> = 0>
PyObject * callMethod(PyObject *self, PyObject *args) {
    return asObject(((getInstance<f>(self)).*f)()).release();
}


template<auto f, std::enable_if_t<is_void<f> && !is_static<f> && !has_args<f>, int> = 0>
PyObject * callMethod(PyObject *self, PyObject *args) {
    ((getInstance<f>(self)).*f)();
    Py_RETURN_NONE;
}


template<auto f, std::enable_if_t<!is_void<f> && !has_args<f>, int> = 0>
PyObject * callGetter(PyObject *self, void *) {
    return callMethod<f>(self, nullptr);
}


END_NAMESPACE(method_traits)


template<typename T>
struct ExtensionType {
    
    static bool createType(const char *name, const ObjectPtr &module) noexcept {
        PyType_Spec spec = {
            .name = name,
            .basicsize = sizeof(Object),
            .flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_FINALIZE,
            .slots = slots
        };
        auto type = ObjectPtr::owned(PyType_FromSpec(&spec));
        if (!type) {
            return false;
        }
        
        const char *shortName = std::strchr(name, '.');
        if (shortName) {
            shortName++;
        } else {
            shortName = name;
        }
        
        Py_INCREF(type.get());
        if (PyModule_AddObject(module.get(), shortName, type.get())) {
            Py_DECREF(type.get());
            return false;
        }
        
        Py_CLEAR(typeObject);
        typeObject = type.release();
        return true;
    }
    
    static PyObject * getTypeObject() noexcept {
        return typeObject;
    }
    
    static T & getInstance(PyObject *o) noexcept {
        return reinterpret_cast<T &>(reinterpret_cast<Object *>(o)->instance);
    }
    
private:
    struct Object {
        PyObject_HEAD
        std::aligned_storage_t<sizeof(T), alignof(T)> instance;
    };
    
    static PyObject * tp_new(PyTypeObject *subtype, PyObject *args, PyObject *kwds) {
        auto obj = ObjectPtr::created(PyType_GenericNew(subtype, args, kwds));
        new (&getInstance(obj.get())) T;
        return obj.release();
    }
    
    static int tp_init(PyObject *self, PyObject *args, PyObject *kwds) {
        if (kwds && PyDict_Size(kwds)) {
            PyErr_Format(PyExc_TypeError, "__init__ takes no keyword arguments");
            return -1;
        }
        static_assert(method_traits::is_void<&T::init>);
        method_traits::call<&T::init>(getInstance(self), ArgTuple(args));
        return 0;
    }
    
    static void tp_finalize(PyObject *o) noexcept {
        PyObject *errorType, *errorValue, *errorTraceback;
        PyErr_Fetch(&errorType, &errorValue, &errorTraceback);
        
        // Destructors shouldn't throw, but just in case...
        safeCall<std::destroy_at<T>>(&getInstance(o));
        if (PyErr_Occurred()) {
            PyErr_WriteUnraisable(o);
        }
        
        PyErr_Restore(errorType, errorValue, errorTraceback);
    }
    
    static PyType_Slot slots[];
    static PyMethodDef methods[];
    static PyGetSetDef getset[];
    static PyObject *typeObject;
    
    void init() { }  // Default __init__ does nothing
    
};


template<typename T>
PyType_Slot ExtensionType<T>::slots[] = {
    { Py_tp_new, reinterpret_cast<void *>(static_cast<newfunc>(safeCall<tp_new>)) },
    { Py_tp_init, reinterpret_cast<void *>(static_cast<initproc>(safeCall<tp_init>)) },
    { Py_tp_finalize, reinterpret_cast<void *>(tp_finalize) },  // Doesn't throw
    { Py_tp_methods, methods },
    { Py_tp_getset, getset },
    { }  // Sentinel
};


template<typename T>
PyMethodDef ExtensionType<T>::methods[] = {
    { }  // Sentinel
};


template<typename T>
PyGetSetDef ExtensionType<T>::getset[] = {
    { }  // Sentinel
};


template<typename T>
PyObject * ExtensionType<T>::typeObject = nullptr;


struct MethodInfo {
    explicit MethodInfo(const char *name, const char *doc = nullptr) noexcept :
        name(name),
        doc(doc)
    { }
    
protected:
    const char * const name;
    const char * const doc;
};


template<auto f>
struct MethodDef : public MethodInfo {
    using MethodInfo::MethodInfo;
    
    operator PyMethodDef() const noexcept {
        return { name, safeCall<method_traits::callMethod<f>>, method_traits::flags<f>, doc };
    }
};


template<auto f>
struct GetSetDef : public MethodInfo {
    using MethodInfo::MethodInfo;
    
    operator PyGetSetDef() const noexcept {
        return { name, safeCall<method_traits::callGetter<f>>, nullptr, doc };
    }
};


END_NAMESPACE_MW_PYTHON


#endif /* ExtensionType_h */
