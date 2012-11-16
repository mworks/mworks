//
//  GILHelpers.h
//  PythonTools
//
//  Created by Christopher Stawarz on 11/15/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef PythonTools_GILHelpers_h
#define PythonTools_GILHelpers_h

#include <Python.h>

#include <boost/noncopyable.hpp>

#include <MWorksCore/MWorksMacros.h>


BEGIN_NAMESPACE_MW


class ScopedGILAcquire : boost::noncopyable {
    
private:
    PyGILState_STATE gstate;
    
public:
    ScopedGILAcquire() {
        gstate = PyGILState_Ensure();
    }
    
    ~ScopedGILAcquire() {
        PyGILState_Release(gstate);
    }
    
};


class ScopedGILRelease : boost::noncopyable {
    
private:
    PyThreadState *threadState;
    
public:
    ScopedGILRelease() {
        threadState = PyEval_SaveThread();
    }
    
    ~ScopedGILRelease() {
        PyEval_RestoreThread(threadState);
    }
    
};


END_NAMESPACE_MW


#endif /* !defined(PythonTools_GILHelpers_h) */


























