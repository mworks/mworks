/*
 *  EventStreamConduitTest.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 11/12/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "EventHandlerConduitTest.h"

#include "IPCEventTransport.h"
#include "SimpleConduit.h"
#include "StandardSystemEventHandler.h"
#include "StandardVariables.h"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( EventStreamConduitTestFixture, "Unit Test" );


void EventStreamConduitTestFixture::setUp(){
    FullCoreEnvironmentTestFixture::setUp();
    
    event_handler = shared_ptr<EventStreamInterface>(new StandardSystemEventHandler());
}



void EventStreamConduitTestFixture::testInOneThread(){
    // CJS 6/10:  Dave disabled this test for some reason.  Rather than not running it at all, let's add a note
    // that it's disabled to the test runner output, so we don't forget about it.
    std::cout << " (DISABLED)";
    return;
    
    
    // Create Interprocess event transports. In principle, these could be
    // in other processes
    shared_ptr<EventTransport> serverside_transport(new IPCEventTransport(EventTransport::server_event_transport,
                                                                          EventTransport::bidirectional_event_transport,
                                                                          "TestCloud2"));
    
    shared_ptr<EventTransport> clientside_transport(new IPCEventTransport(EventTransport::client_event_transport,
                                                                          EventTransport::bidirectional_event_transport,
                                                                          "TestCloud2"));
    
    
    // Create the conduits to test
    shared_ptr<EventStreamConduit> server_conduit(new EventStreamConduit(serverside_transport, event_handler));
    server_conduit->initialize();
    
    shared_ptr<SimpleConduit> client_conduit(new SimpleConduit(clientside_transport));
    
    string var_to_use = ANNOUNCE_BLOCK_TAGNAME;
    shared_ptr<Variable> a_standard_variable = global_variable_registry->getVariable(var_to_use);
    
    // Create some dummy event handlers.  The collector objects just catch
    // events and save them for inspection
    shared_ptr<SimpleEventCollector> serverside_collector(new SimpleEventCollector());
    shared_ptr<SimpleEventCollector> clientside_collector(new SimpleEventCollector());
    
    //server_conduit.registerCallback(0, bind(&SimpleEventCollector::handleEvent, serverside_collector, _1));
    client_conduit->registerCallback(a_standard_variable->getCodecCode(), bind(&SimpleEventCollector::handleEvent, clientside_collector, _1));
    
    // Start the conduits runnings
    client_conduit->initialize();
    
    shared_ptr<Clock> clock = Clock::instance();
    
    clock->sleepMS(100);
    
    
    
    a_standard_variable->setValue(4.0);
    //server_conduit.sendData(, Datum(4.0));
    
    clock->sleepMS(100);
    
    shared_ptr<Event> received_event = clientside_collector->getLastEvent();
    
    CPPUNIT_ASSERT(received_event != NULL);
    CPPUNIT_ASSERT(received_event->getEventCode() == 0);
    CPPUNIT_ASSERT((double)received_event->getData() == (double)4.0);
    
    

    
}


END_NAMESPACE_MW
