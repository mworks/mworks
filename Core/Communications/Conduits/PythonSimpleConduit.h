/*
 *  PythonSimpleConduit.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 10/2/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "SimpleConduit.h"
#include "IPCEventTransport.h"
#include "DummyEventTransport.h"
#include "Exceptions.h"

#include "CoreBuilderForeman.h"
#include "StandardServerCoreBuilder.h"

#include <boost/python.hpp>
namespace mw {
using namespace boost::python;


class PythonEventCallback{

protected:
    boost::python::object function_object;
    
public:

    PythonEventCallback(boost::python::object _function_object){
        function_object = _function_object;
    }
    
    void callback(shared_ptr<Event> event){
        PyGILState_STATE gstate = PyGILState_Ensure();
        function_object(boost::reference_wrapper<Event>(*(event.get())));
        PyGILState_Release(gstate);
    }


};

// A helper class to wrap to make it easy to create a no-fuss interprocess
// conduit to send events from a python application into MW
class PythonIPCPseudoConduit {

protected:

    std::string resource_name;
    shared_ptr<SimpleConduit> conduit;
    bool initialized;
    
public:

    PythonIPCPseudoConduit(std::string _resource_name, EventTransport::event_transport_type type){
        
        // if there's no clock defined, create the core infrastructure
        if(!Clock::instance(false)){
            shared_ptr<StandardServerCoreBuilder> core_builder(new StandardServerCoreBuilder());
            CoreBuilderForeman::constructCoreStandardOrder(core_builder.get());
        }
        
        resource_name = _resource_name;
        initialized = false;
        
        try {
            shared_ptr<EventTransport> _transport(new IPCEventTransport(type, 
                                                                          EventTransport::bidirectional_event_transport, 
                                                                          resource_name));
            if(_transport == NULL){
                throw SimpleException("Failed to create valid transport");
            }
            conduit = shared_ptr<SimpleConduit>(new SimpleConduit(_transport));
        } catch(std::exception& e){
            initialized = false;
            throw SimpleException("Failed to build conduit: ", e.what());
        }
        
        if(conduit == NULL){
            throw SimpleException("Failed to build valid conduit");
        }
        
        cerr << "Created bidirectional conduit: " << resource_name << endl;
    }
    
    virtual bool isInitialized(){
        return initialized;
    }
    
    virtual bool initialize(){
        
        try{
            initialized = conduit->initialize();
        } catch(std::exception& e){
            fprintf(stderr, e.what()); fflush(stderr);
            initialized = false;
        }
        
        return initialized;
    }
    
    virtual void registerCallbackForCode(int code, boost::python::object function_object){
    
        shared_ptr<PythonEventCallback> callback(new PythonEventCallback(function_object));
        conduit->registerCallback(code, bind(&PythonEventCallback::callback, callback, _1));
    }

    virtual void registerCallbackForName(string event_name, boost::python::object function_object){
        
        shared_ptr<PythonEventCallback> callback(new PythonEventCallback(function_object));
        conduit->registerCallback(event_name, bind(&PythonEventCallback::callback, callback, _1));
    }
    
    virtual void finalize(){
        if(conduit != NULL){
            conduit->finalize();
        } else {
            throw SimpleException("Invalid conduit");
        }
    }
    
    virtual void sendFloat(int code, double data){
        //if(!isInitialized()){
//            throw SimpleException("Not initialized");
//        }
//        
        if(conduit != NULL){
            conduit->sendData(code, Datum(data));
        }else {
            //fprintf(stderr, "Test"); fflush(stderr);
            throw SimpleException("Invalid conduit");
        }
    }

    virtual void sendInteger(int code, long data){
        if(conduit != NULL){
            conduit->sendData(code, Datum(data));
        } else {
            throw SimpleException("Invalid conduit");
        }
    }

};

class PythonIPCServerConduit : public PythonIPCPseudoConduit {

public:
    PythonIPCServerConduit(std::string _resource_name) : PythonIPCPseudoConduit(_resource_name, EventTransport::server_event_transport){}
};

class PythonIPCClientConduit : public PythonIPCPseudoConduit {
public:
    PythonIPCClientConduit(std::string _resource_name) : PythonIPCPseudoConduit(_resource_name, EventTransport::client_event_transport){}
};


BOOST_PYTHON_MODULE(_conduit)
{
    
    PyEval_InitThreads();
    
    class_<PythonIPCServerConduit>("IPCServerConduit", init<std::string>())
        .def("initialize", &PythonIPCServerConduit::initialize)
        .def("finalize", &PythonIPCServerConduit::finalize)
        .def("send_float", &PythonIPCServerConduit::sendFloat)
        .def("send_integer", &PythonIPCServerConduit::sendInteger)
        .def("register_callback_for_code", &PythonIPCServerConduit::registerCallbackForCode)
        .def("register_callback_for_name", &PythonIPCServerConduit::registerCallbackForName)
        .add_property("initialized", &PythonIPCServerConduit::isInitialized)
    ;

    class_<PythonIPCClientConduit>("IPCClientConduit", init<std::string>())
        .def("initialize", &PythonIPCClientConduit::initialize)
        .def("finalize", &PythonIPCClientConduit::finalize)
        .def("send_float", &PythonIPCClientConduit::sendFloat)
        .def("send_integer", &PythonIPCClientConduit::sendInteger)
        .def("register_callback_for_code", &PythonIPCClientConduit::registerCallbackForCode)
        .def("register_callback_for_name", &PythonIPCClientConduit::registerCallbackForName)
        .add_property("initialized", &PythonIPCClientConduit::isInitialized)
    ;

    class_<Event, boost::noncopyable>("Event")
        .add_property("code", &Event::getEventCode)
        .add_property("data", &Event::getData);
    ;
    
    class_<Data, boost::noncopyable>("Data")
        .add_property("float", &Data::getFloat)
        .add_property("integer", &Data::getInteger);
    ;
}

}
