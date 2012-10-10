/*
 *  SimpleConduitTest.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 10/1/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "SimpleConduitTest.h"
#include "IPCEventTransport.h"
#include "SimpleConduit.h"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SimpleConduitTestFixture, "Unit Test" );

void SimpleEventCollector::handleEvent(shared_ptr<Event> event){
    boost::mutex::scoped_lock lock(last_event_lock);
    last_event = event;
    //std::cerr << "handleEvent on collector: " << this << std::endl;
}

shared_ptr<Event> SimpleEventCollector::getLastEvent(){ 
    boost::mutex::scoped_lock lock(last_event_lock);
    //std::cerr << "getLastEvent on collector: " << this << std::endl;
    return last_event; 
}

void SimpleConduitTestFixture::testInOneThread(){


    // Create Interprocess event transports. In principle, these could be
    // in other processes
    shared_ptr<EventTransport> serverside_transport(new IPCEventTransport(EventTransport::server_event_transport,
                                                                                EventTransport::bidirectional_event_transport,
                                                                                "TestCloud"));

    shared_ptr<EventTransport> clientside_transport(new IPCEventTransport(EventTransport::client_event_transport,
                                                                                EventTransport::bidirectional_event_transport,
                                                                                "TestCloud"));
                                                                                
    
    // Create the conduits to test
    SimpleConduit server_conduit(serverside_transport);
    SimpleConduit client_conduit(clientside_transport);
    
    
    // Create some dummy event handlers.  The collector objects just catch
    // events and save them for inspection
    shared_ptr<SimpleEventCollector> serverside_collector(new SimpleEventCollector());
    shared_ptr<SimpleEventCollector> clientside_collector(new SimpleEventCollector());
    server_conduit.registerCallback(0, bind(&SimpleEventCollector::handleEvent, serverside_collector, _1));
    client_conduit.registerCallback(0, bind(&SimpleEventCollector::handleEvent, clientside_collector, _1));
    
    // Start the conduits runnings
    server_conduit.initialize();
    client_conduit.initialize();

    server_conduit.sendData(0, Datum(4.0));
    
    shared_ptr<Clock> clock = Clock::instance();
    clock->sleepMS(100);
    
    shared_ptr<Event> received_event = clientside_collector->getLastEvent();
    
    CPPUNIT_ASSERT(received_event != NULL);
    CPPUNIT_ASSERT(received_event->getEventCode() == 0);
    CPPUNIT_ASSERT((double)received_event->getData() == 4.0);
    
    
    // Stop the conduits
    server_conduit.finalize();
    client_conduit.finalize();
    
}


END_NAMESPACE_MW
