/*
 *  ClockTest.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 10/30/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ClockTest.h"
#include "Clock.h"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ClockTestFixture, "Unit Test" );

void ClockTestFixture::setUp() {
	shared_ptr <Clock> shared_clock = Clock::instance(false);
	CPPUNIT_ASSERT(shared_clock == NULL);	
}

void ClockTestFixture::tearDown() {
	shared_ptr <Clock> shared_clock = Clock::instance(false);
	CPPUNIT_ASSERT(shared_clock == NULL);
}

void ClockTestFixture::testClock(){
	shared_ptr <Clock> shared_clock = Clock::instance(false);
	CPPUNIT_ASSERT(shared_clock == NULL);
	for(int clock_creation = 0; clock_creation < 5; ++clock_creation) {
		shared_clock = Clock::instance(false);
		CPPUNIT_ASSERT(shared_clock == NULL);
		
		bool threw_exception = false;
		try {
			shared_clock = Clock::instance(true);		
		} catch (SimpleException &e) {
			threw_exception = true;
		}
		
		CPPUNIT_ASSERT(threw_exception);
		
		threw_exception = false;
		try {
			shared_clock = Clock::instance();		
		} catch (SimpleException &e) {
			threw_exception = true;
		}
		
		CPPUNIT_ASSERT(threw_exception);
		
		shared_clock = Clock::instance(false);	
		CPPUNIT_ASSERT(shared_clock == NULL);
		shared_ptr <Clock> new_clock = shared_ptr<Clock>(new Clock(0));
		Clock::registerInstance(new_clock);
		shared_clock = Clock::instance(false);	
		CPPUNIT_ASSERT(shared_clock != NULL);
		
		Clock::destroy();
		shared_clock = Clock::instance(false);	
		CPPUNIT_ASSERT(shared_clock == NULL);
		
		new_clock = shared_ptr<Clock>(new Clock(0));
		Clock::registerInstance(new_clock);
		shared_clock = Clock::instance(true);	
		CPPUNIT_ASSERT(shared_clock != NULL);
		
		Clock::destroy();
		shared_clock = Clock::instance(false);	
		CPPUNIT_ASSERT(shared_clock == NULL);
		
		new_clock = shared_ptr<Clock>(new Clock(0));
		Clock::registerInstance(new_clock);
		shared_clock = Clock::instance();	
		CPPUNIT_ASSERT(shared_clock != NULL);	
		
		Clock::destroy();
		shared_clock = Clock::instance(false);	
		CPPUNIT_ASSERT(shared_clock == NULL);
	}
	shared_clock = Clock::instance(false);	
	CPPUNIT_ASSERT(shared_clock == NULL);
}


END_NAMESPACE_MW

