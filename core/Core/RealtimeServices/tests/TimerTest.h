/*
 *  TimerTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 9/25/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#include "FullCoreEnvironmentTest.h"
#include <vector>

#include "MWorksCore/Timer.h"


BEGIN_NAMESPACE_MW


class TimerTestFixture : public FullCoreEnvironmentTestFixture {

	CPPUNIT_TEST_SUITE( TimerTestFixture );
	
	CPPUNIT_TEST( testTimerFiring );
	CPPUNIT_TEST( testTimerResetting );
	CPPUNIT_TEST( testTimerUnderAttack );
	CPPUNIT_TEST( testTimerOverride );	
	CPPUNIT_TEST( testTimerOverrideMultipleTimes );	
	CPPUNIT_TEST( testTimerOverrideMultipleTimesFast );
	CPPUNIT_TEST( testTimerValue );	
	CPPUNIT_TEST( testTimerClone );	
	CPPUNIT_TEST( testTimerWithTimebase );

	CPPUNIT_TEST_SUITE_END();

protected:
	
 public:
	void setUp();
	void tearDown();

	void testTimerFiring();
	void testTimerResetting();
	void testTimerUnderAttack();
	void testTimerOverride();
	void testTimerOverrideMultipleTimes();
	void testTimerOverrideMultipleTimesFast();
	void testTimerValue();	
	void testTimerClone();
	void testTimerWithTimebase();

};


END_NAMESPACE_MW

