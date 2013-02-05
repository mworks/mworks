/*
 *  EventStreamConduitTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 11/12/09.
 *  Copyright 2009 Harvard University. All rights reserved.
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

#include "EventStreamConduit.h"


BEGIN_NAMESPACE_MW

    
    class EventStreamConduitTestFixture : public FullCoreEnvironmentTestFixture {
        
        
        CPPUNIT_TEST_SUITE( EventStreamConduitTestFixture );
        CPPUNIT_TEST( testInOneThread );
        CPPUNIT_TEST_SUITE_END();
        
        
    private:
        
        shared_ptr<EventStreamInterface> event_handler;
    public:
        
        void setUp();
        void testInOneThread();
    };
    

END_NAMESPACE_MW


#endif

