/*
 *  PythonSimpleConduit.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/2/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "CodecAwareConduit.h"
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
    
    void callback(shared_ptr<Event> evt){
        PyGILState_STATE gstate = PyGILState_Ensure();
        Event evt_copy(*evt);
        
        try {
            function_object(evt_copy);
        } catch (error_already_set &) {
            PyErr_Print();
        }
        
        PyGILState_Release(gstate);
    }


};

// A helper class to wrap to make it easy to create a no-fuss interprocess
// conduit to send events from a python application into MW
class PythonIPCPseudoConduit {

protected:

    std::string resource_name;
    shared_ptr<CodecAwareConduit> conduit;
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
            conduit = shared_ptr<CodecAwareConduit>(new CodecAwareConduit(_transport));
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
            fprintf(stderr, "%s\n", e.what()); fflush(stderr);
            initialized = false;
        }
        
        return initialized;
    }
    
    virtual void registerCallbackForCode(int code, boost::python::object function_object){
    
        shared_ptr<PythonEventCallback> callback(new PythonEventCallback(function_object));
        conduit->registerCallback(code, bind(&PythonEventCallback::callback, callback, _1));
    }

    virtual void registerCallbackForName(string event_name, boost::python::object function_object){
        
        shared_ptr<PythonEventCallback> cb(new PythonEventCallback(function_object));
        conduit->registerCallbackByName(event_name, bind(&PythonEventCallback::callback, cb, _1));
    }
    
    
    virtual void registerLocalEventCode(int code, string event_name){
        conduit->registerLocalEventCode(code, event_name);
    }
    
    virtual void finalize(){
        if(conduit != NULL){
            conduit->finalize();
        } else {
            throw SimpleException("Invalid conduit");
        }
    }
    
    
    virtual mw::Datum packagePyObject(PyObject *pyobj){
    
        if(PyFloat_Check(pyobj)){
            return mw::Datum(PyFloat_AsDouble(pyobj));
        } else if(PyInt_Check(pyobj)){
            return mw::Datum(PyInt_AsLong(pyobj));
        } else if(PyString_Check(pyobj)){
            return mw::Datum(PyString_AsString(pyobj));
        } else if(PyMapping_Check(pyobj)){
            return packagePyMapping(pyobj);
        } else if(PySequence_Check(pyobj)){
            return packagePySequence(pyobj);
        }

        return mw::Datum();
    }
           
    virtual mw::Datum packagePySequence(PyObject *pyobj){
        
        int size = PySequence_Size(pyobj);
        mw::Datum list_datum(M_LIST, size);
        
        for(int i = 0; i < size; i++){
            list_datum.addElement(packagePyObject(PySequence_GetItem(pyobj, i)));
        }
        
        return list_datum;
    }
     
    virtual mw::Datum packagePyMapping(PyObject *pyobj){
        
        int size = PyMapping_Size(pyobj);
        
        mw::Datum dict_datum(M_DICTIONARY, size);
        
        PyObject *keys = PyMapping_Keys(pyobj);
        PyObject *items = PyMapping_Items(pyobj);
        
        for(int i = 0; i < size; i++){
            PyObject *key = PySequence_GetItem(keys,i);
            PyObject *item = PySequence_GetItem(items, i);
            
            dict_datum.setElement(packagePyObject(key), packagePyObject(item));
        }
        
        return dict_datum;
    }
    
    virtual void sendPyObject(int code, PyObject *pyobj){
        if(conduit != NULL){
            conduit->sendData(code, packagePyObject(pyobj));
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


    
extern double scarab_extract_float(ScarabDatum *datum);
extern PyObject *convert_scarab_to_python(ScarabDatum *datum);
extern PyObject *convert_datum_to_python(Datum datum);
    
    
BOOST_PYTHON_MODULE(_conduit)
{
    
    PyEval_InitThreads();
        
    class_<PythonIPCServerConduit>("IPCServerConduit", init<std::string>())
        .def("__initialize", &PythonIPCServerConduit::initialize)
        .def("finalize", &PythonIPCServerConduit::finalize)
        .def("send_float", &PythonIPCServerConduit::sendFloat)
        .def("send_integer", &PythonIPCServerConduit::sendInteger)
        .def("send_object", &PythonIPCServerConduit::sendPyObject)
        .def("register_callback_for_code", &PythonIPCServerConduit::registerCallbackForCode)
        .def("register_callback_for_name", &PythonIPCServerConduit::registerCallbackForName)
        .def("register_local_event_code", &PythonIPCServerConduit::registerLocalEventCode)
        .add_property("initialized", &PythonIPCServerConduit::isInitialized)
    ;

    class_<PythonIPCClientConduit>("IPCClientConduit", init<std::string>())
        .def("__initialize", &PythonIPCClientConduit::initialize)
        .def("finalize", &PythonIPCClientConduit::finalize)
        .def("send_float", &PythonIPCClientConduit::sendFloat)
        .def("send_integer", &PythonIPCClientConduit::sendInteger)
        .def("send_object", &PythonIPCClientConduit::sendPyObject)
        .def("register_callback_for_code", &PythonIPCClientConduit::registerCallbackForCode)
        .def("register_callback_for_name", &PythonIPCClientConduit::registerCallbackForName)
        .def("register_local_event_code", &PythonIPCClientConduit::registerLocalEventCode)
        .add_property("initialized", &PythonIPCClientConduit::isInitialized)
    ;

    class_<Event>("Event")
        .def("__convert_mw_datum_to_python", &convert_datum_to_python)
        .staticmethod("__convert_mw_datum_to_python")
        .add_property("code", &Event::getEventCode)
        .add_property("mw_datum", &Event::getData);
    ;
    
    class_<Datum>("Datum")
        .add_property("float", &Datum::getFloat)
        .add_property("integer", &Datum::getInteger);
    ;
}

}
