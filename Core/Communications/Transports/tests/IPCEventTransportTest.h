/*
 *  IPCEventTransportTest.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/28/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#ifndef _IPC_EVENT_TRANSPORT_TEST_H_
#define _IPC_EVENT_TRANSPORT_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "FullCoreEnvironmentTest.h"
namespace mw {
class IPCEventTransportTestFixture : public CppUnit::TestFixture {
	
	
	CPPUNIT_TEST_SUITE( IPCEventTransportTestFixture );
	CPPUNIT_TEST( testOneThread );
	CPPUNIT_TEST_SUITE_END();
	
	
private:
public:

    void setUp();
	void tearDown();
	void testOneThread();
	
	
};

}
#endif
