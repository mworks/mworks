/*
 *  CodecAwareConduitTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 12/9/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef _CODEC_AWARE_CONDUIT_TEST_H_
#define _CODEC_AWARE_CONDUIT_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "FullCoreEnvironmentTest.h"
#include "SimpleConduitTest.h"
#include "CodecAwareConduit.h"


BEGIN_NAMESPACE_MW

    
    class CodecAwareConduitTestFixture : public FullCoreEnvironmentTestFixture {
        
        
        CPPUNIT_TEST_SUITE( CodecAwareConduitTestFixture );
        CPPUNIT_TEST( testInOneThread );
        CPPUNIT_TEST_SUITE_END();
        
        
    private:
        
        shared_ptr<EventTransport> serverside_transport, clientside_transport;
        shared_ptr<CodecAwareConduit> server_conduit, client_conduit;
        
    public:
        
        void setUp();
        void tearDown();
        
        void testInOneThread();
        
        
        
    };
    
    
END_NAMESPACE_MW


#endif
