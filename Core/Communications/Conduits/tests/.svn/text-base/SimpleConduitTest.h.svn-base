/*
 *  SimpleConduitTest.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/28/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */
/*
 *  IPCEventTransportTest.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/28/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#ifndef _SIMPLE_CONDUIT_TEST_H_
#define _SIMPLE_CONDUIT_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "FullCoreEnvironmentTest.h"

class SimpleConduitTestFixture : public FullCoreEnvironmentTestFixture {
	
	
	CPPUNIT_TEST_SUITE( SimpleConduitTestFixture );
	CPPUNIT_TEST( testInOneThread );
	CPPUNIT_TEST_SUITE_END();
	
	
private:
public:

//    void setUp(){ }
//	void tearDown(){ }

	void testInOneThread();
};


class SimpleEventCollector {

protected:

    shared_ptr<Event> last_event;
    
public:

    void handleEvent(shared_ptr<Event> event);
    
    shared_ptr<Event> getLastEvent(){ return last_event; }
};


#endif

