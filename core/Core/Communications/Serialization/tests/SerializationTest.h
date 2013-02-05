/*
 *  SerializationTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 9/28/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */
/*
 *  IPCEventTransportTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 9/28/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#ifndef _SERIALIZATION_TEST_H_
#define _SERIALIZATION_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "FullCoreEnvironmentTest.h"


BEGIN_NAMESPACE_MW


class SerializationTestFixture : public CppUnit::TestFixture {
	
	
	CPPUNIT_TEST_SUITE( SerializationTestFixture );
	CPPUNIT_TEST( testEventSerialization );
	CPPUNIT_TEST( testScarabSerialization );
    CPPUNIT_TEST( testDictSerialization );
    CPPUNIT_TEST( testStringDatumSerialization );
    CPPUNIT_TEST( testListSerialization );
    CPPUNIT_TEST_SUITE_END();
	
	
private:
public:

	void setUp();
	void tearDown();

	void testScarabSerialization();
    void testEventSerialization();
	void testDictSerialization();
    void testStringDatumSerialization();
    void testListSerialization();
	
};


END_NAMESPACE_MW


#endif

