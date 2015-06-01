//
//  GILHelpers.h
//  PythonTools
//
//  Created by Christopher Stawarz on 11/15/12.
//  Copyright (c) 2012 MWorks Project. All rights reserved.
//

#ifndef PythonTools_GILHelpers_h
#define PythonTools_GILHelpers_h


BEGIN_NAMESPACE_MW


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


END_NAMESPACE_MW


#endif /* !defined(PythonTools_GILHelpers_h) */


























