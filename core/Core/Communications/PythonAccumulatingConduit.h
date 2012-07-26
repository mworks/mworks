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
    
    
    class PythonEventListCallback : public PythonEventCallback {
        
    public:
        
        typedef vector< shared_ptr<Event> > EventList;
        
        PythonEventListCallback(boost::python::object _function_object) :
            PythonEventCallback(_function_object)
        { }
        
        void callback(shared_ptr<EventList> evts){
            ScopedGILAcquire sga;
            
            boost::python::list evts_list;
            
            EventList::iterator i;
            for(i = evts->begin(); i != evts->end(); ++i){
                evts_list.append< shared_ptr<Event> >(*i);
            }
            //EventList evts_copy(*evts);
            
            try {
                (*function_object)(evts_list);
            } catch (error_already_set &) {
                PyErr_Print();
            }
        }
        
        
    };
    
    
    
    class PythonIPCAccumulatingConduit : public PythonIPCPseudoConduit{
     
    protected:
    
        string start_evt, end_evt;
        vector<string> events_to_watch;
        
    public:
        
        PythonIPCAccumulatingConduit(string _resource_name, EventTransport::event_transport_type event_trans_type,
                                     string _start_evt,
                                     string _end_evt,
                                     boost::python::list _events_to_watch,
                                     bool correct_incoming_timestamps=false) :
                                     PythonIPCPseudoConduit(_resource_name,
                                                           correct_incoming_timestamps,
                                                           event_trans_type){
            start_evt = _start_evt;
            end_evt = _end_evt;
            
            int n = len(_events_to_watch);
            for(int i = 0; i < n; i++){
                events_to_watch.push_back(boost::python::extract<string>(_events_to_watch[i]));
            }
        }
        
        
        virtual bool initialize(){
            ScopedGILRelease sgr;
            
            try{
                conduit = shared_ptr<CodecAwareConduit>(new AccumulatingConduit(transport, start_evt, end_evt, events_to_watch));
                initialized = conduit->initialize();
            } catch(std::exception& e){
                fprintf(stderr, "%s\n", e.what()); fflush(stderr);
                initialized = false;
            }
            
            return initialized;
        }
        
        virtual void registerBundleCallback(boost::python::object function_object){
            
            shared_ptr<PythonEventListCallback> cb(new PythonEventListCallback(function_object));
            
            // Need to hold the GIL until *after* we create the PythonEventListCallback, since doing so
            // involves an implicit PyINCREF
            ScopedGILRelease sgr;
            
            shared_ptr<AccumulatingConduit> accumulating_conduit = dynamic_pointer_cast<AccumulatingConduit>(conduit);
            accumulating_conduit->registerBundleCallback(bind(&PythonEventListCallback::callback, cb, _1));
        }

    };
    
    class PythonIPCAccumulatingServerConduit : public PythonIPCAccumulatingConduit {
        
    public:
        PythonIPCAccumulatingServerConduit(std::string _resource_name,
                                           string _start_evt,
                                           string _end_evt,
                                           boost::python::list _events_to_watch) : PythonIPCAccumulatingConduit(_resource_name, 
                                                                                                                EventTransport::server_event_transport,
                                                                                                                _start_evt,
                                                                                                                _end_evt,
                                                                                                                _events_to_watch){}
    };
    
    class PythonIPCAccumulatingClientConduit : public PythonIPCAccumulatingConduit {
    public:
            PythonIPCAccumulatingClientConduit(std::string _resource_name,
                                           string _start_evt,
                                           string _end_evt,
                                           boost::python::list _events_to_watch) : PythonIPCAccumulatingConduit(_resource_name, 
                                                                                                                EventTransport::client_event_transport,
                                                                                                                _start_evt,
                                                                                                                _end_evt,
                                                                                                                _events_to_watch){}    
    };
    
    
    
    extern PyObject *convert_scarab_to_python(ScarabDatum *datum);
    extern PyObject *convert_datum_to_python(Datum datum);
    

    
}
