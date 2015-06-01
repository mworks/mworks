/*
 *  PythonSimpleConduit.h
 *
 *  Created by David Cox on 10/2/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#ifndef PYTHON_SIMPLE_CONDUIT_H_
#define PYTHON_SIMPLE_CONDUIT_H_

#include "GILHelpers.h"
#include "PythonDataHelpers.h"

using namespace boost::python;


BEGIN_NAMESPACE_MW


class PythonEventCallback : boost::noncopyable {

protected:
    // Use a pointer so that we can ensure that the GIL is held when the object is destroyed
    boost::python::object *function_object;
    
public:

    explicit PythonEventCallback(const boost::python::object &_function_object) {
        // The caller should already hold the GIL, so we don't acquire it here
        function_object = new boost::python::object(_function_object);
    }
    
    ~PythonEventCallback() {
        ScopedGILAcquire sga;
        delete function_object;
    }
    
    void callback(shared_ptr<Event> evt){
        ScopedGILAcquire sga;
        try {
            (*function_object)(evt);
        } catch (const error_already_set &) {
            PyErr_Print();
        }
    }


};

// A helper class to wrap to make it easy to create a no-fuss interprocess
// conduit to send events from a python application into MW
class PythonIPCConduit : boost::noncopyable {

protected:

    shared_ptr<EventTransport> transport;
    shared_ptr<CodecAwareConduit> conduit;
    bool initialized;
    const bool correct_incoming_timestamps;
    
    void requireValidConduit() const {
        if (!conduit || !initialized) {
            throw SimpleException("Conduit is not initialized");
        }
    }
    
public:

    PythonIPCConduit(const std::string &resource_name,
                     bool _correct_incoming_timestamps,
                     EventTransport::event_transport_type type) :
        initialized(false),
        correct_incoming_timestamps(_correct_incoming_timestamps)
    {
        // We'll be calling Python code in non-Python background threads, so ensure that the
        // GIL is initialized
        PyEval_InitThreads();
        
        // Need to hold the GIL until *after* we call PyEval_InitThreads
        ScopedGILRelease sgr;
        
        // if there's no clock defined, create the core infrastructure
        if (!Clock::instance(false)) {
            StandardServerCoreBuilder core_builder;
            CoreBuilderForeman::constructCoreStandardOrder(&core_builder);
        }
        
        transport.reset(new IPCEventTransport(type, EventTransport::bidirectional_event_transport, resource_name));
    }
    
    bool isInitialized() const {
        return initialized;
    }
    
    bool initialize(){
        ScopedGILRelease sgr;
        
        conduit.reset(new CodecAwareConduit(transport, correct_incoming_timestamps));
        initialized = conduit->initialize();
        
        return initialized;
    }
    
    void registerCallbackForCode(int code, const boost::python::object &function_object) {
        requireValidConduit();
        
        shared_ptr<PythonEventCallback> callback(new PythonEventCallback(function_object));
        
        // Need to hold the GIL until *after* we create the PythonEventCallback, since doing so
        // involves an implicit Py_INCREF
        ScopedGILRelease sgr;
        
        conduit->registerCallback(code, bind(&PythonEventCallback::callback, callback, _1));
    }

    void registerCallbackForName(const std::string &event_name, const boost::python::object &function_object) {
        requireValidConduit();
        
        shared_ptr<PythonEventCallback> cb(new PythonEventCallback(function_object));
        
        // Need to hold the GIL until *after* we create the PythonEventCallback, since doing so
        // involves an implicit Py_INCREF
        ScopedGILRelease sgr;
        
        conduit->registerCallbackByName(event_name, bind(&PythonEventCallback::callback, cb, _1));
    }
    
    
    void registerLocalEventCode(int code, const std::string &event_name) {
        requireValidConduit();
        
        ScopedGILRelease sgr;
        conduit->registerLocalEventCode(code, event_name);
    } 
    
    boost::python::dict getAndConvertCodec(bool reverse) {
        requireValidConduit();
        
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
    
    boost::python::dict getCodec(){
        return getAndConvertCodec(false);
    }
    
    boost::python::dict getReverseCodec(){
        return getAndConvertCodec(true);
    }
    
    
    void finalize(){
        requireValidConduit();
        
        ScopedGILRelease sgr;
        conduit->finalize();
    }
    
    void sendPyObject(int code, const boost::python::object &pyobj){
        requireValidConduit();
        
        Datum data = convert_python_to_datum(pyobj);
        
        // Need to hold the GIL until *after* we convert the object
        ScopedGILRelease sgr;
        
        conduit->sendData(code, data);
    }
    
    void sendFloat(int code, double data){
        requireValidConduit();
        
        ScopedGILRelease sgr;
        conduit->sendData(code, Datum(data));
    }

    void sendInteger(int code, long data){
        requireValidConduit();
        
        ScopedGILRelease sgr;
        conduit->sendData(code, Datum(data));
    }

};

class PythonIPCServerConduit : public PythonIPCConduit {

public:
    explicit PythonIPCServerConduit(const std::string &resource_name,
                                    bool correct_incoming_timestamps=false) : 
                           PythonIPCConduit(resource_name, 
                                            correct_incoming_timestamps,
                                            EventTransport::server_event_transport)
    { }
};

class PythonIPCClientConduit : public PythonIPCConduit {
public:
    explicit PythonIPCClientConduit(const std::string &resource_name,
                                    bool correct_incoming_timestamps=false) :
                           PythonIPCConduit(resource_name, 
                                            correct_incoming_timestamps,
                                            EventTransport::client_event_transport)
    { }
};


inline boost::python::object extractEventTime(const Event &e) {
    return convert_longlong_to_python(e.getTime());
}


inline boost::python::object extractEventData(const Event &e) {
    return convert_datum_to_python(e.getData());
}


END_NAMESPACE_MW


#endif

























