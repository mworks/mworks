/*
 *  AccumulatingConduit.h
 *  MWorksCore
 *
 *  Created by David Cox on 10/18/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#ifndef ACCUMULATING_CONDUIT_H_
#define ACCUMULATING_CONDUIT_H_

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "CodecAwareConduit.h"

namespace mw{

class AccumulatingConduit : public CodecAwareConduit{

protected:

    string start_event_name;
    string end_event_name;
    
    vector<string> event_names_to_accumulate;
    
    typedef vector< shared_ptr<Event> > EventList;
    shared_ptr<EventList> event_buffer;
    
    bool accumulating;

    typedef boost::function< void(shared_ptr<EventList>) > EventListCallback;    
    EventListCallback bundle_callback;

public:

    AccumulatingConduit(shared_ptr<EventTransport> _transport,
                        string _start_event_name,
                        string _end_event_name,
                        vector<string> _event_names) : CodecAwareConduit(_transport){
    
        accumulating = false;
        
        start_event_name = _start_event_name;
        end_event_name = _end_event_name;
        event_names_to_accumulate = _event_names;
        
        event_buffer = shared_ptr<EventList>(new EventList());
    }

    virtual bool initialize(){
        registerCallbackByName(start_event_name, boost::bind(&AccumulatingConduit::startAccumulating, this, _1));
        registerCallbackByName(end_event_name, boost::bind(&AccumulatingConduit::stopAccumulating, this, _1));
        
        vector<string>::iterator i;
        for(i = event_names_to_accumulate.begin(); i != event_names_to_accumulate.end(); ++i){
            registerCallbackByName(*i, boost::bind(&AccumulatingConduit::accumulate, this, _1));
        }
        
        return CodecAwareConduit::initialize();
    }

    void startAccumulating(shared_ptr<Event> evt){
        accumulating = true;
    }
    
    void stopAccumulating(shared_ptr<Event> evt){
        accumulating = false;
        bundle_callback(event_buffer);
        event_buffer = shared_ptr<EventList>(new EventList());
    }
    
    void accumulate(shared_ptr<Event> evt){
        if(accumulating){
            event_buffer->push_back(evt);
        }
    }

    void registerBundleCallback(EventListCallback _bundle_callback){
        bundle_callback = _bundle_callback;
    }

};
}

#endif