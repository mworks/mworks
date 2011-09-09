/*
 *  ConduitServerTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef _PYTHON_CONDUIT_SERVER_TEST_H_
#define _PYTHON_CONDUIT_SERVER_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "FullCoreEnvironmentTest.h"

#include "SimpleConduitTest.h"

#include "EventStreamConduit.h"

namespace mw {
    
    class PythonConduitServerTestFixture : public FullCoreEnvironmentTestFixture {
        
        
        CPPUNIT_TEST_SUITE( PythonConduitServerTestFixture );
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