/*
 *  PythonAccumulatingConduit.h
 *
 *  Created by David Cox on 10/18/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#include "PythonSimpleConduit.h"


namespace mw {
    
    
    class PythonEventListCallback : public PythonEventCallback {
        
    public:
        
        typedef vector< shared_ptr<Event> > EventList;
        
        explicit PythonEventListCallback(const boost::python::object &_function_object) :
            PythonEventCallback(_function_object)
        { }
        
        void callback(shared_ptr<EventList> evts){
            ScopedGILAcquire sga;
            
            boost::python::list evts_list;
            
            for (EventList::iterator i = evts->begin(); i != evts->end(); i++) {
                evts_list.append< shared_ptr<Event> >(*i);
            }
            
            try {
                (*function_object)(evts_list);
            } catch (const error_already_set &) {
                PyErr_Print();
            }
        }
        
        
    };
    
    
    
    class PythonIPCAccumulatingConduit : public PythonIPCConduit {
     
    protected:
    
        const std::string start_evt;
        const std::string end_evt;
        vector<string> events_to_watch;
        
    public:
        
        PythonIPCAccumulatingConduit(const std::string &resource_name,
                                     EventTransport::event_transport_type event_trans_type,
                                     const std::string &start_evt,
                                     const std::string &end_evt,
                                     const boost::python::list &_events_to_watch,
                                     bool correct_incoming_timestamps) :
            PythonIPCConduit(resource_name, correct_incoming_timestamps, event_trans_type),
            start_evt(start_evt),
            end_evt(end_evt)
        {
            ssize_t n = len(_events_to_watch);
            for (ssize_t i = 0; i < n; i++) {
                events_to_watch.push_back(boost::python::extract<string>(_events_to_watch[i]));
            }
        }
        
        
        bool initialize(){
            ScopedGILRelease sgr;
            
            conduit.reset(new AccumulatingConduit(transport, start_evt, end_evt, events_to_watch));
            initialized = conduit->initialize();
            
            return initialized;
        }
        
        void registerBundleCallback(const boost::python::object &function_object) {
            requireValidConduit();
            
            shared_ptr<PythonEventListCallback> cb(new PythonEventListCallback(function_object));
            
            // Need to hold the GIL until *after* we create the PythonEventListCallback, since doing so
            // involves an implicit Py_INCREF
            ScopedGILRelease sgr;
            
            shared_ptr<AccumulatingConduit> accumulating_conduit = boost::dynamic_pointer_cast<AccumulatingConduit>(conduit);
            accumulating_conduit->registerBundleCallback(bind(&PythonEventListCallback::callback, cb, _1));
        }

    };
    
    class PythonIPCAccumulatingServerConduit : public PythonIPCAccumulatingConduit {

    public:
        PythonIPCAccumulatingServerConduit(const std::string &resource_name,
                                           const std::string &start_evt,
                                           const std::string &end_evt,
                                           const boost::python::list &events_to_watch,
                                           bool correct_incoming_timestamps = false) :
            PythonIPCAccumulatingConduit(resource_name,
                                         EventTransport::server_event_transport,
                                         start_evt,
                                         end_evt,
                                         events_to_watch,
                                         correct_incoming_timestamps)
        { }
        
    };
    
    class PythonIPCAccumulatingClientConduit : public PythonIPCAccumulatingConduit {
        
    public:
        PythonIPCAccumulatingClientConduit(const std::string &resource_name,
                                           const std::string &start_evt,
                                           const std::string &end_evt,
                                           const boost::python::list &events_to_watch,
                                           bool correct_incoming_timestamps = false) :
            PythonIPCAccumulatingConduit(resource_name,
                                         EventTransport::client_event_transport,
                                         start_evt,
                                         end_evt,
                                         events_to_watch,
                                         correct_incoming_timestamps)
        { }
        
    };
    

    
}
