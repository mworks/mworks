/*
 *  TimerTest.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 9/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "TimerTest.h"
#include "MWorksCore/TrialBuildingBlocks.h"
#include "MWorksCore/GlobalVariable.h"
#include "MWorksCore/MachUtilities.h"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TimerTestFixture, "Unit Test" );


Lockable *g_timer_cppunit_lock;
bool g_timerTestPassed;
clock_t g_timeToEnd;
#define NUM_TIMER_THREADS 400
#define NUM_TIMER_RESETS 50

#define timer_CPPUNIT_ASSERT_TS(x)	g_timer_cppunit_lock->lock(); \
if(g_timerTestPassed) { \
g_timerTestPassed = x; \
} \
g_timer_cppunit_lock->unlock()

void *checkTimerValue(void *args) {
	Timer *t = (Timer *)args;
	
	while(clock() < g_timeToEnd) {
		timer_CPPUNIT_ASSERT_TS(!t->getValue().getBool());
	}
	
	return NULL;
}


void TimerTestFixture::setUp() {
	g_timer_cppunit_lock = new Lockable();
	g_timerTestPassed = true;
	FullCoreEnvironmentTestFixture::setUp();
}

void TimerTestFixture::tearDown() {
	FullCoreEnvironmentTestFixture::tearDown();
	delete g_timer_cppunit_lock;
}


void TimerTestFixture::testTimerFiring(){
	
	for(int i = 0; i<NUM_TIMER_RESETS; ++i) {
		shared_ptr<Timer> t(new Timer());
		
		//if(i%10 == 0) {
		//	fprintf(stderr, "%d timer resets\n", i); fflush(stderr);
		//}
		
		t->startMS(100);
		
		shared_ptr <Clock> clock = Clock::instance();
		clock->sleepMS(10);
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
		clock->sleepMS(50);
		
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
		clock->sleepMS(70);
		
		CPPUNIT_ASSERT( t->hasExpired() == true );
	}
}

void TimerTestFixture::testTimerResetting(){
	shared_ptr<Timer> t(new Timer());
	
	for(int i = 0; i<NUM_TIMER_RESETS; ++i) {
		
		//if(i%10 == 0) {
		//	fprintf(stderr, "%d timer resets\n", i); fflush(stderr);
		//}
		
		t->startMS(100);
		
		shared_ptr <Clock> clock = Clock::instance();
		clock->sleepMS(10);
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
		clock->sleepMS(50);
		
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
		clock->sleepMS(70);
		
		CPPUNIT_ASSERT( t->hasExpired() == true );
	}
}


void TimerTestFixture::testTimerValue(){
	pthread_t threads[NUM_TIMER_THREADS];
	pthread_attr_t pthread_custom_attr;
	
	pthread_attr_init(&pthread_custom_attr);
	
	shared_ptr<Timer> t(new Timer());
    t->startMS(10000);  // Ensure that timer is not expired initially
	timer_CPPUNIT_ASSERT_TS( t->hasExpired() == false );

	const int numberOfSecondsToWait = 1;
	g_timeToEnd = clock() + numberOfSecondsToWait*CLOCKS_PER_SEC;	
	
	for (int i=0; i<NUM_TIMER_THREADS; ++i) {
		timer_CPPUNIT_ASSERT_TS(0 == pthread_create(&threads[i], 
                                                    &pthread_custom_attr, 
                                                    &checkTimerValue, t.get()));
	}
	
	t->startMS(2000);
	timer_CPPUNIT_ASSERT_TS( t->hasExpired() == false );
	
	shared_ptr <Clock> clock = Clock::instance();
	clock->sleepMS(1500);
	timer_CPPUNIT_ASSERT_TS( t->hasExpired() == false );
	
	clock->sleepMS(750);
	
	timer_CPPUNIT_ASSERT_TS( t->hasExpired() == true );
				
	for (int i=0; i<NUM_TIMER_THREADS; i++) {
		timer_CPPUNIT_ASSERT_TS(pthread_join(threads[i], NULL) == 0);
	}

	g_timer_cppunit_lock->lock();
	CPPUNIT_ASSERT(g_timerTestPassed);
	g_timer_cppunit_lock->unlock();

}


END_NAMESPACE_MW
