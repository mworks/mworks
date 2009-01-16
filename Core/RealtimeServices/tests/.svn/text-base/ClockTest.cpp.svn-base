/*
 *  ClockTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 10/30/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ClockTest.h"
#include "Clock.h"
using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ClockTestFixture, "Unit Test" );

void ClockTestFixture::setUp() {
	shared_ptr <Clock> shared_clock = Clock::instance(false);
	CPPUNIT_ASSERT(shared_clock == 0);	
}

void ClockTestFixture::tearDown() {
	shared_ptr <Clock> shared_clock = Clock::instance(false);
	CPPUNIT_ASSERT(shared_clock == 0);
}

void ClockTestFixture::testClock(){
	shared_ptr <Clock> shared_clock = Clock::instance(false);
	CPPUNIT_ASSERT(shared_clock == 0);
	for(int clock_creation = 0; clock_creation < 5; ++clock_creation) {
		shared_clock = Clock::instance(false);
		CPPUNIT_ASSERT(shared_clock == 0);
		
		bool threw_exception = false;
		try {
			shared_clock = Clock::instance(true);		
		} catch (std::exception &e) {
			threw_exception = true;
		}
		
		CPPUNIT_ASSERT(threw_exception);
		
		threw_exception = false;
		try {
			shared_clock = Clock::instance();		
		} catch (std::exception &e) {
			threw_exception = true;
		}
		
		CPPUNIT_ASSERT(threw_exception);
		
		shared_clock = Clock::instance(false);	
		CPPUNIT_ASSERT(shared_clock == 0);
		shared_ptr <Clock> new_clock = shared_ptr<Clock>(new Clock(0));
		Clock::registerInstance(new_clock);
		shared_clock = Clock::instance(false);	
		CPPUNIT_ASSERT(shared_clock != 0);
		
		Clock::destroy();
		shared_clock = Clock::instance(false);	
		CPPUNIT_ASSERT(shared_clock == 0);
		
		new_clock = shared_ptr<Clock>(new Clock(0));
		Clock::registerInstance(new_clock);
		shared_clock = Clock::instance(true);	
		CPPUNIT_ASSERT(shared_clock != 0);
		
		Clock::destroy();
		shared_clock = Clock::instance(false);	
		CPPUNIT_ASSERT(shared_clock == 0);
		
		new_clock = shared_ptr<Clock>(new Clock(0));
		Clock::registerInstance(new_clock);
		shared_clock = Clock::instance();	
		CPPUNIT_ASSERT(shared_clock != 0);	
		
		Clock::destroy();
		shared_clock = Clock::instance(false);	
		CPPUNIT_ASSERT(shared_clock == 0);
	}
	shared_clock = Clock::instance(false);	
	CPPUNIT_ASSERT(shared_clock == 0);
}

