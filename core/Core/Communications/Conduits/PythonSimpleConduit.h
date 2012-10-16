/*
 *  PythonSimpleConduit.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/2/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#ifndef PYTHON_SIMPLE_CONDUIT_H_
#define PYTHON_SIMPLE_CONDUIT_H_

#include <boost/python.hpp>
#include <boost/noncopyable.hpp>

#include "CodecAwareConduit.h"
#include "IPCEventTransport.h"
#include "Exceptions.h"

#include "Clock.h"
#include "CoreBuilderForeman.h"
#include "StandardServerCoreBuilder.h"

#include "PythonDataHelpers.h"

using namespace boost::python;


BEGIN_NAMESPACE_MW


class PythonEventCallback : boost::noncopyable {

protected:
    // Use a pointer so that we can ensure that the GIL is held when the object is destroyed
    boost::python::object *function_object;
    
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
    
public:

    PythonEventCallback(boost::python::object _function_object){
        // The caller should already hold the GIL, so we don't acquire it here
        function_object = new boost::python::object(_function_object);
    }
    
    ~PythonEventCallback() {
        ScopedGILAcquire sga;
        delete function_object;
    }
    
    void callback(shared_ptr<Event> evt){
        ScopedGILAcquire sga;
        Event evt_copy(*evt);
        
        try {
            (*function_object)(evt_copy);
        } catch (error_already_set &) {
            PyErr_Print();
        }
    }


};

// A helper class to wrap to make it easy to create a no-fuss interprocess
// conduit to send events from a python application into MW
class PythonIPCPseudoConduit {

protected:

    std::string resource_name;
    shared_ptr<EventTransport> transport;
    shared_ptr<CodecAwareConduit> conduit;
    bool initialized;
    bool correct_incoming_timestamps;
    
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
    
public:

    PythonIPCPseudoConduit(std::string _resource_name,
                           bool _correct_incoming_timestamps,
                           EventTransport::event_transport_type type) :
        correct_incoming_timestamps(_correct_incoming_timestamps)
    {
        ScopedGILRelease sgr;
        
        // if there's no clock defined, create the core infrastructure
        if(!Clock::instance(false)){
            shared_ptr<StandardServerCoreBuilder> core_builder(new StandardServerCoreBuilder());
            CoreBuilderForeman::constructCoreStandardOrder(core_builder.get());
        }
        
        resource_name = _resource_name;
        initialized = false;
        
        try {
            transport = shared_ptr<EventTransport>(new IPCEventTransport(type, 
                                                                          EventTransport::bidirectional_event_transport, 
                                                                          resource_name));
            if(transport == NULL){
                throw SimpleException("Failed to create valid transport");
            }
        } catch(std::exception& e){
            initialized = false;
            throw SimpleException("Failed to build conduit: ", e.what());
        }
    }
    
    virtual bool isInitialized(){
        return initialized;
    }
    
    virtual bool initialize(){
        ScopedGILRelease sgr;
        
        try{
            conduit = shared_ptr<CodecAwareConduit>(new CodecAwareConduit(transport, 
                                                                          correct_incoming_timestamps));

            initialized = conduit->initialize();
        } catch(std::exception& e){
            fprintf(stderr, "%s\n", e.what()); fflush(stderr);
            initialized = false;
        }
        
        if(conduit == NULL){
            initialized = false;
        }
        
        return initialized;
    }
    
    virtual void registerCallbackForCode(int code, boost::python::object function_object){
        
        shared_ptr<PythonEventCallback> callback(new PythonEventCallback(function_object));
        
        // Need to hold the GIL until *after* we create the PythonEventCallback, since doing so
        // involves an implicit Py_INCREF
        ScopedGILRelease sgr;
        
        conduit->registerCallback(code, bind(&PythonEventCallback::callback, callback, _1));
    }

    virtual void registerCallbackForName(string event_name, boost::python::object function_object){
        
        shared_ptr<PythonEventCallback> cb(new PythonEventCallback(function_object));
        
        // Need to hold the GIL until *after* we create the PythonEventCallback, since doing so
        // involves an implicit Py_INCREF
        ScopedGILRelease sgr;
        
        conduit->registerCallbackByName(event_name, bind(&PythonEventCallback::callback, cb, _1));
    }
    
    
    virtual void registerLocalEventCode(int code, string event_name){
        ScopedGILRelease sgr;
        conduit->registerLocalEventCode(code, event_name);
    } 
    
    virtual boost::python::dict getAndConvertCodec(bool reverse=false){
        boost::python::dict d;
        map<int, string> codec;
        
        {
            ScopedGILRelease sgr;
            codec = conduit->getRemoteCodec();
        }
        
        map<int, string>::iterator i;
        for(i = codec.begin(); i != codec.end(); i++){
            pair<int, string> map_pair = *i;
            if(reverse){
                d[map_pair.second] = map_pair.first;
            } else {
                d[map_pair.first] = map_pair.second;
            }
        }
        
        return d;
    }
    
    virtual boost::python::dict getCodec(){
        return getAndConvertCodec();
    }
    
    virtual boost::python::dict getReverseCodec(){
        return getAndConvertCodec(true);
    }
    
    
    virtual void finalize(){
        ScopedGILRelease sgr;
        if(conduit != NULL){
            conduit->finalize();
        } else {
            throw SimpleException("Invalid conduit");
        }
    }
    
    
    virtual mw::Datum packagePyObject(PyObject *pyobj){
        ScarabDatum *scarabDatum = convert_python_to_scarab(pyobj);
        mw::Datum datum(scarabDatum);
        scarab_free_datum(scarabDatum);
        return datum;
    }
    
    virtual void sendPyObject(int code, PyObject *pyobj){
        Datum data = packagePyObject(pyobj);
        
        // Need to hold the GIL until *after* we convert the object
        ScopedGILRelease sgr;
        
        if(conduit != NULL){
            conduit->sendData(code, data);
        } else {
            throw SimpleException("Invalid conduit");
        }
    }
    
    virtual void sendFloat(int code, double data){
        ScopedGILRelease sgr;
        
        if(conduit != NULL){
            conduit->sendData(code, Datum(data));
        }else {
            //fprintf(stderr, "Test"); fflush(stderr);
            throw SimpleException("Invalid conduit");
        }
    }

    virtual void sendInteger(int code, long data){
        ScopedGILRelease sgr;
        if(conduit != NULL){
            conduit->sendData(code, Datum(data));
        } else {
            throw SimpleException("Invalid conduit");
        }
    }

};

class PythonIPCServerConduit : public PythonIPCPseudoConduit {

public:
    PythonIPCServerConduit(std::string _resource_name, 
                           bool correct_incoming_timestamps=false) : 
                           PythonIPCPseudoConduit(_resource_name, 
                                                  correct_incoming_timestamps,
                                                  EventTransport::server_event_transport){}
};

class PythonIPCClientConduit : public PythonIPCPseudoConduit {
public:
    PythonIPCClientConduit(std::string _resource_name, 
                           bool correct_incoming_timestamps=false) :
                           PythonIPCPseudoConduit(_resource_name, 
                                                  correct_incoming_timestamps,
                                                  EventTransport::client_event_transport){}
};


    
inline PyObject *convert_datum_to_python(const Datum &datum) {
    return convert_scarab_to_python(datum.getScarabDatum());
}


END_NAMESPACE_MW


#endif
