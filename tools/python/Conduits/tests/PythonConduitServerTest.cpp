/*
 *  ConduitServerTest.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "PythonConduitServerTest.h"


/*
 *  EventStreamConduitTest.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 11/12/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#include "IPCEventTransport.h"
#include "CodecAwareConduit.h"
#include "StandardSystemEventHandler.h"
#include "StandardVariables.h"
#include "EventStreamConduit.h"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( PythonConduitServerTestFixture, "Unit Test" );


void PythonConduitServerTestFixture::setUp(){
    FullCoreEnvironmentTestFixture::setUp();
    
    event_handler = shared_ptr<EventStreamInterface>(new StandardSystemEventHandler());
}



void PythonConduitServerTestFixture::testInOneThread(){
    // As noted below, we need to figure out where to store PythonConduitTest.py
    std::cout << " (DISABLED)";
    return;
   
    
    // Create Interprocess event transports. In principle, these could be
    // in other processes
    // NOTE: the resource name must agree with that in PythonConduitTest.py
    shared_ptr<EventTransport> serverside_transport(new IPCEventTransport(EventTransport::server_event_transport,
                                                                          EventTransport::bidirectional_event_transport,
                                                                          "python_test_conduit"));
    
    
    
    // Create the conduits to test
    shared_ptr<CodecAwareConduit> server_conduit(new CodecAwareConduit(serverside_transport));
    server_conduit->initialize();
    
    // Launch the python process
    // need to figure out what to do with the paths
    execl("/usr/bin/python", "../../PythonConduitTest.py", NULL);

    // the following values must agree with those in PythonConduitTest.py
    int code_to_use = 4;
    string var_to_use = ANNOUNCE_BLOCK_TAGNAME;
    
    
    // Create some dummy event handlers.  The collector objects just catch
    // events and save them for inspection
    shared_ptr<SimpleEventCollector> serverside_collector_a(new SimpleEventCollector());
    shared_ptr<SimpleEventCollector> serverside_collector_b(new SimpleEventCollector());
    
    
    server_conduit->registerCallbackByName(ANNOUNCE_BLOCK_TAGNAME, bind(&SimpleEventCollector::handleEvent, serverside_collector_a, _1));
    
    // Start the conduits runnings
    
    shared_ptr<Clock> clock = Clock::instance();
    
    clock->sleepMS(100);
    
    shared_ptr<Variable> a_standard_variable = global_variable_registry->getVariable(var_to_use);
    a_standard_variable->setValue(4.0);
    a_standard_variable->setValue(5.0);
    a_standard_variable->setValue(6.0);
    
    server_conduit->sendData(code_to_use, 6L);
    server_conduit->sendData(code_to_use, "blah");
    server_conduit->sendData(code_to_use, 8.0);
    
    clock->sleepMS(100);
    
    shared_ptr<Event> received_event_a = serverside_collector_a->getLastEvent();
    shared_ptr<Event> received_event_b = serverside_collector_b->getLastEvent();
    
    CPPUNIT_ASSERT(received_event_a != NULL);
    CPPUNIT_ASSERT(received_event_a->getEventCode() == 200);
    CPPUNIT_ASSERT((double)received_event_a->getData() == (double)3.0);

    CPPUNIT_ASSERT(received_event_b != NULL);
    CPPUNIT_ASSERT(received_event_b->getEventCode() == 201);
    CPPUNIT_ASSERT((double)received_event_b->getData() == (double)6.0);
    

}


END_NAMESPACE_MW
