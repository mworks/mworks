/*
 *  EventStreamInterfaceConduitTest.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 11/12/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

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

#ifndef _EVENT_HANDLER_CONDUIT_TEST_H_
#define _EVENT_HANDLER_CONDUIT_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "FullCoreEnvironmentTest.h"

#include "SimpleConduitTest.h"

#include "EventStreamInterfaceConduit.h"

namespace mw {
    
    class EventStreamInterfaceConduitTestFixture : public FullCoreEnvironmentTestFixture {
        
        
        CPPUNIT_TEST_SUITE( EventStreamInterfaceConduitTestFixture );
        CPPUNIT_TEST( testInOneThread );
        CPPUNIT_TEST_SUITE_END();
        
        
    private:
        
        shared_ptr<EventStreamInterface> event_handler;
    public:
        
        void setUp();
        void testInOneThread();
    };
    

}

#endif

