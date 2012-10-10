/*
 *  CodecAwareConduitTest.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 12/9/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "CodecAwareConduitTest.h"

#include "SimpleConduitTest.h"
#include "IPCEventTransport.h"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( CodecAwareConduitTestFixture, "Unit Test" );


void CodecAwareConduitTestFixture::setUp(){
    FullCoreEnvironmentTestFixture::setUp();
    
    // Create Interprocess event transports. In principle, these could be
    // in other processes
    serverside_transport = shared_ptr<EventTransport>(new IPCEventTransport(EventTransport::server_event_transport,
                                                                          EventTransport::bidirectional_event_transport,
                                                                          "CodecAwareConduitTest"));
    
    clientside_transport = shared_ptr<EventTransport>(new IPCEventTransport(EventTransport::client_event_transport,
                                                                          EventTransport::bidirectional_event_transport,
                                                                          "CodecAwareConduitTest"));
    
    
    // Create the conduits to test
    server_conduit = shared_ptr<CodecAwareConduit>(new CodecAwareConduit(serverside_transport));
    client_conduit = shared_ptr<CodecAwareConduit> (new CodecAwareConduit(clientside_transport));
    
}

void CodecAwareConduitTestFixture::tearDown(){

    if(server_conduit != NULL){
        server_conduit->finalize();
    }
    
    if(client_conduit != NULL){
        client_conduit->finalize();
    }
    
    FullCoreEnvironmentTestFixture::tearDown();
    
}


void CodecAwareConduitTestFixture::testInOneThread(){
    
    shared_ptr<Clock> clock = Clock::instance();
    
    // Start the conduits running
    server_conduit->initialize();
    client_conduit->initialize();
    
    
    string test_variable_name("test");
    // Just to be devious, we associate different codes with "test" on either side of the conduit.
    server_conduit->registerLocalEventCode(4, test_variable_name);
    client_conduit->registerLocalEventCode(6, test_variable_name);
    
    

    clock->sleepMS(100);
    
    // Create some dummy event handlers.  The collector objects just catch
    // events and save them for inspection (we define these in SimpleConduitTest.h)
    shared_ptr<SimpleEventCollector> serverside_collector(new SimpleEventCollector());
    shared_ptr<SimpleEventCollector> clientside_collector(new SimpleEventCollector());
    
    // We register callbacks here by name, so both sides will callback to the
    // collector when an event "named" test_variable_name arrives
    server_conduit->registerCallbackByName(test_variable_name, bind(&SimpleEventCollector::handleEvent, serverside_collector, _1));
    client_conduit->registerCallbackByName(test_variable_name, bind(&SimpleEventCollector::handleEvent, clientside_collector, _1));
    
    // Give some time for behind-the-scenes read threads to do their work
    clock->sleepMS(100);
    
    
    // 4 means "test" on the server side
    server_conduit->sendData(4, Datum(4.0));
    //server_conduit->sendData(6, Datum(4.0));
    
    // 6 means "test" on the client side
    client_conduit->sendData(6, Datum(4.0));
    //client_conduit->sendData(4, Datum(4.0));
    
    // Give some time for behind-the-scenes read threads to do their work
    clock->sleepMS(100);
    
    // Interrogate what came out on the other side
    shared_ptr<Event> received_event = clientside_collector->getLastEvent();
    
    //std::cerr << "Attempting ASSERTS" << std::endl;
    
    CPPUNIT_ASSERT(received_event != NULL);
    //CPPUNIT_ASSERT(received_event->getEventCode() == 0);
    CPPUNIT_ASSERT((double)received_event->getData() == 4.0);
    
    
    received_event = serverside_collector->getLastEvent();
    
    CPPUNIT_ASSERT(received_event != NULL);
    //CPPUNIT_ASSERT(received_event->getEventCode() == 0);
    CPPUNIT_ASSERT((double)received_event->getData() == 4.0);
    
    // send another event with an unknown code
    client_conduit->sendData(7, Datum(6.0));
    clock->sleepMS(100);
    
    received_event = serverside_collector->getLastEvent();
    
    CPPUNIT_ASSERT(received_event != NULL);
    CPPUNIT_ASSERT(received_event->getEventCode() == 4);
    // still should be 4, because the event was sent on a different channel
    CPPUNIT_ASSERT((double)received_event->getData() == 4.0);
    
}


END_NAMESPACE_MW
