/*
 *  IPCEventTransportTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/28/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "IPCEventTransportTest.h"
#include "IPCEventTransport.h"
using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( IPCEventTransportTestFixture, "Unit Test" );


void IPCEventTransportTestFixture::testOneThread(){

    IPCEventTransport server(EventTransport::server_event_transport, 
                              EventTransport::write_only_event_transport, 
                              "TestTransport");
    IPCEventTransport client(EventTransport::client_event_transport,
                              EventTransport::read_only_event_transport,
                              "TestTransport");
                
    Data data((long)4);
    shared_ptr<Event> event(new Event(1, data));
    
    server.sendEvent(event);
    shared_ptr<Event> event2 = client.receiveEvent();
    
    CPPUNIT_ASSERT( event2->getEventCode() == event->getEventCode() );
    CPPUNIT_ASSERT( (long)event2->getData() == (long)event->getData() );
    

}

void IPCEventTransportTestFixture::setUp(){
	shared_ptr <Clock> new_clock = shared_ptr<Clock>(new Clock(0));
	Clock::registerInstance(new_clock);
}

void IPCEventTransportTestFixture::tearDown(){
	Clock::destroy();
}