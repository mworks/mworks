/*
 *  ClockTest.h
 *  MWorksCore
 *
 *  Created by bkennedy on 10/30/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */
#ifndef CLOCK_TEST_H_
#define CLOCK_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


class ClockTestFixture : public CppUnit::TestFixture {
	
	CPPUNIT_TEST_SUITE( ClockTestFixture );	
	CPPUNIT_TEST (testClock);
	
	CPPUNIT_TEST_SUITE_END();
	
	
public:
	void setUp();
	void tearDown();
	
	void testClock();
};


END_NAMESPACE_MW


#endif


