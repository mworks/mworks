//
//  Utilities.h
//  PythonTools
//
//  Created by Christopher Stawarz on 10/2/19.
//  Copyright © 2019 MWorks Project. All rights reserved.
//

#ifndef Utilities_h
#define Utilities_h


BEGIN_NAMESPACE_MW_PYTHON


struct ErrorAlreadySet { };


class ScopedGILAcquire : boost::noncopyable {
    
private:
    PyGILState_STATE state;
    
public:
    ScopedGILAcquire() :
        state(PyGILState_Ensure())
    { }
    
    ~ScopedGILAcquire() {
        PyGILState_Release(state);
    }
    
};


class ScopedGILRelease : boost::noncopyable {
    
private:
    PyThreadState *state;
    
public:
    ScopedGILRelease() :
        state(PyEval_SaveThread())
    { }
    
    ~ScopedGILRelease() {
        PyEval_RestoreThread(state);
    }
    
};


class ScopedRecursionGuard : boost::noncopyable {
    
public:
    explicit ScopedRecursionGuard(const char *where) {
#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03090000
        if (Py_EnterRecursiveCall(where)) {
            throw ErrorAlreadySet();
        }
#endif
    }
    
    ~ScopedRecursionGuard() {
#if !defined(Py_LIMITED_API) || Py_LIMITED_API+0 >= 0x03090000
        Py_LeaveRecursiveCall();
#endif
    }
    
};


template<typename T>
struct SafeCallFailure {
    static constexpr PyObject * result() { return nullptr; }
};


template<>
struct SafeCallFailure<int (*/*initproc*/)(PyObject *, PyObject *, PyObject *)> {
    static constexpr int result() { return -1; }
};


template<>
struct SafeCallFailure<int (*/*converter*/)(PyObject *, void *) > {
    static constexpr int result() { return 0; }
};


template<typename T>
struct SafeCallFailure<void (*)(T *)> {
    static constexpr void result() { }
};


template<auto f, typename... Args>
auto safeCall(Args... args) noexcept {
    try {
        return f(args...);
    } catch (const ErrorAlreadySet &) {
        // Python error already set.  Do nothing.
    } catch (const std::exception &e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
    } catch (...) {
        PyErr_SetString(PyExc_RuntimeError, "An unknown error occurred");
    }
    return SafeCallFailure<decltype(f)>::result();
}


template<typename T>
using remove_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;


END_NAMESPACE_MW_PYTHON


#endif /* Utilities_h */
