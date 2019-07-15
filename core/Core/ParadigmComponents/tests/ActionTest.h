/*
 *  ActionTest.h
 *  MWorksCore
 *
 *  Created by labuser on 10/2/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#ifndef _ACTION_TEST_H_
#define _ACTION_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "FullCoreEnvironmentTest.h"


BEGIN_NAMESPACE_MW


class ActionTestFixture : public FullCoreEnvironmentTestFixture {
	
	
	CPPUNIT_TEST_SUITE( ActionTestFixture );
	CPPUNIT_TEST( testSimpleAssignment1 );
	CPPUNIT_TEST( testSimpleAssignment2 );
	CPPUNIT_TEST( testIncrementCounterWithAssignment );
	CPPUNIT_TEST( testIncrementCounterWithAssignmentManyThreads );
	CPPUNIT_TEST( testScheduledAssignment );
	CPPUNIT_TEST( testScheduledAssignmentWithCancel );
	CPPUNIT_TEST( testScheduledAssignmentWithCancelThatsTooLate );
	CPPUNIT_TEST_SUITE_END();
	
	
public:
	// assigment tests
	void testSimpleAssignment1();
	void testSimpleAssignment2();
	void testIncrementCounterWithAssignment();
	void testIncrementCounterWithAssignmentManyThreads();
	void testScheduledAssignment();
	void testScheduledAssignmentWithCancel();
	void testScheduledAssignmentWithCancelThatsTooLate();
	void testCancelNULLScheduledActions();
	
};


END_NAMESPACE_MW


#endif

