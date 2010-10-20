/*
 *  PythonAccumulatingConduit.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/18/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

/*
 *  PythonSimpleConduit.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/2/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include <boost/python.hpp>
#include "PythonSimpleConduit.h"


#include "AccumulatingConduit.h"
#include "IPCEventTransport.h"
#include "DummyEventTransport.h"
#include "Exceptions.h"


namespace mw {

    using namespace std;
    
    
    class PythonEventListCallback{
        
    protected:
        boost::python::object function_object;
        
    public:
        
        typedef vector<Event> EventList;
        
        PythonEventListCallback(boost::python::object _function_object){
            function_object = _function_object;
        }
        
        void callback(shared_ptr< vector<Event> > evts){
            PyGILState_STATE gstate = PyGILState_Ensure();
            EventList evts_copy(*evts);
            
            try {
                function_object(evts_copy);
            } catch (error_already_set &) {
                PyErr_Print();
            }
            
            PyGILState_Release(gstate);
        }
        
        
    };
    
    
    
    class PythonIPCAccumulatingConduit : public PythonIPCPseudoConduit{
     
    protected:
    
        string start_evt, end_evt;
        shared_ptr< vector<string> > events_to_watch;
        
    public:
        
        PythonIPCAccumulatingConduit(string _resource_name, EventTransport::event_transport_type type
                                     string _start_evt,
                                     string _end_evt,
                                     vector<std::string> _events_to_watch) :
                                     PythonIPCPseudoConduit(_resource_name, event_transport_type){
            start_evt = _start_evt;
            end_evt = _end_evt;
            events_to_watch = shared_ptr< vector<string> >(new vector<string>(_events_to_watch));
        }
        
        
        virtual bool initialize(){
            
            try{
                conduit = shared_ptr<CodecAwareConduit>(new AccumulatingConduit(start_evt, end_evt, events_to_watch));
                initialized = conduit->initialize();
            } catch(std::exception& e){
                fprintf(stderr, "%s\n", e.what()); fflush(stderr);
                initialized = false;
            }
            
            return initialized;
        }
        
        virtual void registerBundleCallback(boost::python::object function_object){
            
            shared_ptr<PythonEventListCallback> cb(new PythonEventListCallback(function_object));
            accumulating_conduit = dynamic_pointer_cast<AccumulatingConduit>(conduit);
            accumulating_conduit->registerBundleCallback(bind(&PythonEventListCallback::callback, cb, _1));
        }

    };
    
    class PythonIPCAccumulatingServerConduit : public PythonIPCAccumulatingConduit {
        
    public:
        PythonIPCAccumulatingServerConduit(std::string _resource_name) : PythonIPCAccumulatingConduit(_resource_name, EventTransport::server_event_transport){}
    };
    
    class PythonIPCAccumulatingClientConduit : public PythonIPCAccumulatingConduit {
    public:
        PythonIPCAccumulatingClientConduit(std::string _resource_name) : PythonIPCAccumulatingConduit(_resource_name, EventTransport::client_event_transport){}
    };
    
    
    
    extern PyObject *convert_scarab_to_python(ScarabDatum *datum);
    extern PyObject *convert_datum_to_python(Datum datum);
    
    
    BOOST_PYTHON_MODULE(_conduit)
    {
        
        PyEval_InitThreads();
        
        class_<PythonIPCAccumulatingServerConduit>("_IPCAccumulatingServerConduit", init<std::string>())
        .def("_initialize", &PythonIPCAccumulatingServerConduit::initialize)
        .def("finalize", &PythonIPCAccumulatingServerConduit::finalize)
        .def("send_float", &PythonIPCAccumulatingServerConduit::sendFloat)
        .def("send_integer", &PythonIPCAccumulatingServerConduit::sendInteger)
        .def("send_object", &PythonIPCAccumulatingServerConduit::sendPyObject)
        .def("register_callback_for_code", &PythonIPCAccumulatingServerConduit::registerCallbackForCode)
        .def("register_callback_for_name", &PythonIPCAccumulatingServerConduit::registerCallbackForName)
        .def("register_local_event_code", &PythonIPCAccumulatingServerConduit::registerLocalEventCode)
        .def("register_bundle_callback", &PythonIPCAccumulatingServerConduit::registerBundleCallback)
        .add_property("initialized", &PythonIPCAccumulatingServerConduit::isInitialized)
        ;
        
        class_<PythonIPCClientConduit>("_IPCAccumulatingClientConduit", init<std::string>())
        .def("_initialize", &PythonIPCAccumulatingClientConduit::initialize)
        .def("finalize", &PythonIPCAccumulatingClientConduit::finalize)
        .def("send_float", &PythonIPCAccumulatingClientConduit::sendFloat)
        .def("send_integer", &PythonIPCAccumulatingClientConduit::sendInteger)
        .def("send_object", &PythonIPCAccumulatingClientConduit::sendPyObject)
        .def("register_callback_for_code", &PythonIPCAccumulatingClientConduit::registerCallbackForCode)
        .def("register_callback_for_name", &PythonIPCAccumulatingClientConduit::registerCallbackForName)
        .def("register_local_event_code", &PythonIPCAccumulatingClientConduit::registerLocalEventCode)
        .def("register_bundle_callback", &PythonIPCAccumulatingClientConduit::registerBundleCallback)
        .add_property("initialized", &PythonIPCAccumulatingClientConduit::isInitialized)
        ;
        
    }
    
}
