/*
 *  ActionTest.h
 *  MonkeyWorksCore
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

namespace mw {
class ActionTestFixture : public FullCoreEnvironmentTestFixture {
	
	
	CPPUNIT_TEST_SUITE( ActionTestFixture );
	CPPUNIT_TEST( testSimpleAssignment1 );
	CPPUNIT_TEST( testSimpleAssignment2 );
	CPPUNIT_TEST( testIncrementCounterWithAssignment );
	CPPUNIT_TEST( testIncrementCounterWithAssignmentManyThreads );
	CPPUNIT_TEST( testConstantReport );
	CPPUNIT_TEST( testReportWithVariableAtEnd );
	CPPUNIT_TEST( testReportWithVariableAtBeginning );
	CPPUNIT_TEST( testReportWithVariableInMiddle );
	CPPUNIT_TEST( testReportWith2Variables );
	CPPUNIT_TEST( testReportWithBadVariable );
	CPPUNIT_TEST( testReportWithOneGoodVariableAndOneBadVariable );
	CPPUNIT_TEST( testAssert );
	CPPUNIT_TEST( testAssertWithVariableAtEnd );
	CPPUNIT_TEST( testScheduledAssignment );
	CPPUNIT_TEST( testScheduledAssignmentWithCancel );
	CPPUNIT_TEST( testScheduledAssignmentWithCancelThatsTooLate );
	CPPUNIT_TEST( testCancelNULLScheduledActions );
	CPPUNIT_TEST_SUITE_END();
	
	
private:
public:

	// assigment tests
	void testSimpleAssignment1();
	void testSimpleAssignment2();
	void testIncrementCounterWithAssignment();
	void testIncrementCounterWithAssignmentManyThreads();
	void testConstantReport();
	void testReportWithVariableAtEnd();
	void testReportWithVariableAtBeginning();
	void testReportWithVariableInMiddle();
	void testReportWith2Variables();
	void testReportWithBadVariable();
	void testReportWithOneGoodVariableAndOneBadVariable();
	void testAssert();
	void testAssertWithVariableAtEnd();
	void testScheduledAssignment();
	void testScheduledAssignmentWithCancel();
	void testScheduledAssignmentWithCancelThatsTooLate();
	void testCancelNULLScheduledActions();
	
};
}

#endif

