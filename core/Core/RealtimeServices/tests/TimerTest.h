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
	CPPUNIT_TEST( testTimerValue );

	CPPUNIT_TEST_SUITE_END();

protected:
	
 public:
	void setUp();
	void tearDown();

	void testTimerFiring();
	void testTimerResetting();
	void testTimerValue();

};


END_NAMESPACE_MW

