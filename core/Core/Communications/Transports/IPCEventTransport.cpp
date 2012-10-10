/*
 *  IPCEventTransport.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 9/25/08.
 *  Copyright 2008 Harvard University. All rights reserved.
 *
 */

#include "IPCEventTransport.h"

#include <iostream>
#include <sstream>

#include "Exceptions.h"
#include "Utilities.h"


BEGIN_NAMESPACE_MW


const std::string IPCEventTransport::OUTGOING_SUFFIX("_out");
const std::string IPCEventTransport::INCOMING_SUFFIX("_in");


IPCEventTransport::IPCEventTransport(event_transport_type _type, event_transport_directionality _dir, string _resource_name) :
    EventTransport(_type, _dir)
{

    resource_name = _resource_name;
    
    int incoming_queue_size = 0;
    int outgoing_queue_size = 0;
    
    if(directionality != write_only_event_transport){
        incoming_queue_size = IPCEventTransport::DEFAULT_QUEUE_SIZE;
    }
    
    if(directionality != read_only_event_transport){
        outgoing_queue_size = IPCEventTransport::DEFAULT_QUEUE_SIZE;
    }
    
    string resource_name_outgoing = resource_name + OUTGOING_SUFFIX;
    string resource_name_incoming = resource_name + INCOMING_SUFFIX;
    
    if (type == server_event_transport) {
        message_queue::remove(resource_name_outgoing.c_str());
        message_queue::remove(resource_name_incoming.c_str());
        outgoing_queue = shared_ptr<message_queue>(new message_queue(boost::interprocess::open_or_create, resource_name_outgoing.c_str(), outgoing_queue_size, MAX_MESSAGE_SIZE));
        incoming_queue = shared_ptr<message_queue>(new message_queue(boost::interprocess::open_or_create, resource_name_incoming.c_str(), incoming_queue_size, MAX_MESSAGE_SIZE));
    } else if (type == client_event_transport) {
        outgoing_queue = shared_ptr<message_queue>(new message_queue(boost::interprocess::open_only, resource_name_incoming.c_str()));
        incoming_queue = shared_ptr<message_queue>(new message_queue(boost::interprocess::open_only, resource_name_outgoing.c_str()));
    } else if (type == symmetric_event_transport) {
        outgoing_queue = shared_ptr<message_queue>(new message_queue(boost::interprocess::open_or_create, resource_name_outgoing.c_str(), outgoing_queue_size, MAX_MESSAGE_SIZE));
        incoming_queue = shared_ptr<message_queue>(new message_queue(boost::interprocess::open_or_create, resource_name_incoming.c_str(), incoming_queue_size, MAX_MESSAGE_SIZE));
    } else {
        throw SimpleException("Internal error: invalid event_transport_type in IPCEventTransport constructor");
    }
    
}


void IPCEventTransport::sendEvent(shared_ptr<Event> event){
        
    std::ostringstream output_stream;
    oarchive serialized_archive(output_stream);
    
    //if(outgoing_queue == NULL){
    //    cerr << "Error sending on outgoing queue: nonexistent queue" << endl;
    //    return;
    //}
    
    serialized_archive << event;
    
    string data = output_stream.str();
    try {
        //cerr << "data.size() = " << data.size() << endl;
        boost::posix_time::ptime timeout = (boost::posix_time::microsec_clock::local_time() +
                                            boost::posix_time::microseconds(10000));
        if (!(outgoing_queue->timed_send((void *)data.c_str(), data.size(), QUEUE_PRIORITY, timeout))) {
            merror(M_SYSTEM_MESSAGE_DOMAIN, "Send on outgoing queue timed out; event will not be sent");
        }
    } catch(std::exception& e){
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Error sending on outgoing queue: %s", e.what());
    }
}


shared_ptr<Event> IPCEventTransport::deserializeEvent(message_queue_size_type& received_size){
    
    string incoming_data(receive_buffer, received_size);
    std::istringstream input_stream(incoming_data);
    iarchive serialized_archive(input_stream);
    
    shared_ptr<Event> event;
    serialized_archive >> event;

    return event;
}

shared_ptr<Event> IPCEventTransport::receiveEvent(){
    
    message_queue_size_type received_size = 0;
    unsigned int priority = QUEUE_PRIORITY;
    
    //if(incoming_queue == NULL){
    //    return shared_ptr<Event>();
    //}
    
    try{
        incoming_queue->receive((void *)receive_buffer, MAX_MESSAGE_SIZE, received_size, priority);
    } catch(std::exception& e){
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Error receiving on incoming queue: %s", e.what());
        return shared_ptr<Event>();
    }
    
    shared_ptr<Event> event = deserializeEvent(received_size);
    
    return event;
}


shared_ptr<Event> IPCEventTransport::receiveEventAsynchronous(){
    
    message_queue_size_type received_size = 0;
    unsigned int priority = QUEUE_PRIORITY;
    
    //if(incoming_queue == NULL){
    //    return shared_ptr<Event>();
    //}
    
    bool okayp = true;
    try{
        okayp = incoming_queue->try_receive((void *)receive_buffer, MAX_MESSAGE_SIZE, received_size, priority);
    } catch(std::exception& e){
        merror(M_SYSTEM_MESSAGE_DOMAIN, "Error receiving on incoming queue: %s", e.what());
        okayp = false;
    }
    
    if(!okayp){
        return shared_ptr<Event>();
    }
    
    shared_ptr<Event> event = deserializeEvent(received_size);
    
    return event;
}


void IPCEventTransport::flush(){
    int num_msg = incoming_queue->get_num_msg();
    
    for(int i = 0; i < num_msg - 1; i++){
        receiveEvent();
    }
}


END_NAMESPACE_MW




