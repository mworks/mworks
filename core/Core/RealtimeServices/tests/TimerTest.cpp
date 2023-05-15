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

void TimerTestFixture::testTimerOverride(){
    auto clock = Clock::instance();
    
    MWTime startTime = 0;
    auto waitUntilMS = [&clock, &startTime](MWTime offsetMS) {
        clock->sleepUS((startTime + offsetMS * 1000) - clock->getCurrentTimeUS());
    };
    
    auto t = boost::make_shared<Timer>();
    
	for(int i = 0; i < NUM_TIMER_RESETS; i++){
		//if(i%10 == 0) {
		//	fprintf(stderr, "%d timer resets\n", i); fflush(stderr);
		//}
        
        startTime = clock->getCurrentTimeUS();
		t->startMS(100);
		
        waitUntilMS(10);
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(70);
		
		// 30 ms left to go
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
		// Bump that up to 100
        startTime = clock->getCurrentTimeUS();
		t->startMS(100);
		
        waitUntilMS(70);
		
		// Should be 30 ms left.
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(110);
		
		// Should be done
		CPPUNIT_ASSERT( t->hasExpired() == true );
        
        startTime = clock->getCurrentTimeUS();
		t->startMS(100);
		t->startMS(200);
		
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(150);
		
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(210);
		
		CPPUNIT_ASSERT( t->hasExpired() == true );
	}
}

void TimerTestFixture::testTimerOverrideMultipleTimes(){
    auto clock = Clock::instance();
    
    MWTime startTime = 0;
    auto waitUntilMS = [&clock, &startTime](MWTime offsetMS) {
        clock->sleepUS((startTime + offsetMS * 1000) - clock->getCurrentTimeUS());
    };
    
    auto t = boost::make_shared<Timer>();

	for(int i = 0; i < NUM_TIMER_RESETS; i++){
		//if(i%10 == 0) {
		//	fprintf(stderr, "%d timer resets\n", i); fflush(stderr);
		//}
        
        startTime = clock->getCurrentTimeUS();
		t->startMS(100);
		
        waitUntilMS(10);
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(70);
		
		// 30 ms left to go
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
		// Bump that up to 100
        startTime = clock->getCurrentTimeUS();
		t->startMS(100);
		
        waitUntilMS(70);
		
		// Should be 30 ms left.
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(110);
		
		// Should be done
		CPPUNIT_ASSERT( t->hasExpired() == true );
        
        startTime = clock->getCurrentTimeUS();
		t->startMS(100);
		t->startMS(200);
		
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(150);
		
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(210);
		
		CPPUNIT_ASSERT( t->hasExpired() == true );
	}
}


void TimerTestFixture::testTimerUnderAttack(){
    auto clock = Clock::instance();
    auto t = boost::make_shared<Timer>();
	
	//set_realtime(94);
	
	for(int i = 0; i < 1000; i++){
		MWTime delay = 10000;
		
		MWTime then = clock->getCurrentTimeUS();
		t->startUS(delay);
		
		while(!t->hasExpired()) clock->yield();
		
		MWTime now = clock->getCurrentTimeUS();
		
		//cerr << "Timer fell at expiration-relative time of: " 
		//	 << (now - then) - delay << endl;
		
        CPPUNIT_ASSERT_GREATEREQUAL( delay, (now - then) );
        CPPUNIT_ASSERT_LESS( 1000ll, std::abs( (now - then) - delay ) );
	}
}


void TimerTestFixture::testTimerOverrideMultipleTimesFast(){
    auto clock = Clock::instance();
    
    MWTime startTime = 0;
    auto waitUntilMS = [&clock, &startTime](MWTime offsetMS) {
        clock->sleepUS((startTime + offsetMS * 1000) - clock->getCurrentTimeUS());
    };
    
    auto t = boost::make_shared<Timer>();

	//set_realtime(94);

	for(int i = 0; i < NUM_TIMER_RESETS; i++){
		//if(i%10 == 0) {
		//	fprintf(stderr, "%d timer resets (fast)\n", i); fflush(stderr);
		//}
		
        startTime = clock->getCurrentTimeUS();
		t->startMS(10);
		
        waitUntilMS(1);
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(7);
		
		// 3 ms left to go
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
		// Bump that up to 10
        startTime = clock->getCurrentTimeUS();
		t->startMS(10);
		
        waitUntilMS(7);
		
		// Should be 3 ms left.
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(11);
		
		// Should be done
		CPPUNIT_ASSERT( t->hasExpired() == true );
        
        startTime = clock->getCurrentTimeUS();
		t->startMS(10);
		t->startMS(20);
		
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(15);
		
		CPPUNIT_ASSERT( t->hasExpired() == false );
		
        waitUntilMS(21);
		
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
