//
//  PythonEvent.hpp
//  PythonTools
//
//  Created by Christopher Stawarz on 10/15/19.
//  Copyright © 2019 MWorks Project. All rights reserved.
//

#ifndef PythonEvent_hpp
#define PythonEvent_hpp

#include "ExtensionType.h"


BEGIN_NAMESPACE_MW_PYTHON


class PythonEvent : public ExtensionType<PythonEvent> {
    
private:
    int code = -1;
    MWTime time = 0;
    Datum data;
    
public:
    static auto create(int code, MWTime time, const Datum &data) {
        auto obj = ObjectPtr::created(PyObject_CallObject(getTypeObject(), nullptr));
        auto &evt = getInstance(obj.get());
        evt.code = code;
        evt.time = time;
        evt.data = data;
        return obj;
    }
    
    auto getCode() const {
        return code;
    }
    
    auto getTime() const {
        return time;
    }
    
    auto getData() const {
        return data;
    }
    
    class Callback {
    public:
        ~Callback() {
            // Ensure that the GIL is held when the callback is decref'd
            ScopedGILAcquire sga;
            callback.reset();
        }
        
        explicit Callback(const ObjectPtr &callback) :
            // The caller already holds the GIL, so we don't need to acquire it
            callback(callback)
        { }
        
        // Copy constructor
        Callback(const Callback &other) {
            ScopedGILAcquire sga;
            callback = other.callback;
        }
        
        // Move constructor
        Callback(Callback &&other) :
            // No need to acquire the GIL, since no reference counts will change
            callback(std::move(other.callback))
        { }
        
        // No copy assignment or move assignment
        Callback& operator=(const Callback &other) = delete;
        Callback& operator=(Callback &&other) = delete;
        
        void operator()(boost::shared_ptr<Event> evt) {
            ScopedGILAcquire sga;
            try {
                auto event = create(evt->getEventCode(), evt->getTime(), evt->getData());
                ObjectPtr::created(PyObject_CallFunctionObjArgs(callback.get(), event.get(), nullptr));  // Discard result
            } catch (const ErrorAlreadySet &) {
                handleCallbackError();
            }
        }
        
    private:
        static void handleCallbackError();  // Must be implemented by any module that uses this class
        
        ObjectPtr callback;
    };
    
};


END_NAMESPACE_MW_PYTHON


#endif /* PythonEvent_hpp */
