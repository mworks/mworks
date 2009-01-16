/*
 *  TimerTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "TimerTest.h"
#include "MonkeyWorksCore/TrialBuildingBlocks.h"
#include "MonkeyWorksCore/GlobalVariable.h"

//CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TimerTestFixture, "Unit Test" );


#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/thread_policy.h>
#include <mach/task_policy.h>
#include <mach/thread_act.h>
#include <sys/sysctl.h>
using namespace mw;
int set_realtime(int priority){
		kern_return_t                       result = 0;
	    
		integer_t	timeShareData;
		integer_t	precedenceData;
		//thread_extended_policy_data_t       timeShareData;
	    //thread_precedence_policy_data_t     precedenceData;

	    //Set up some variables that we need for the task
	   
		precedenceData = priority;
		
		if(priority > 64){
			timeShareData = 0;
		} else {
			timeShareData = 1;
		}
	   // precedenceData.importance = priority;
	   // timeShareData.timeshare = true;//isTimeshare;
	    
	    mach_port_t  machThread = mach_thread_self();

	    //Set the scheduling flavor. We want to do this first, since doing so
	    //can alter the priority
	    result = thread_policy_set( machThread,
	                                THREAD_EXTENDED_POLICY,
	                                &timeShareData,
	                                THREAD_EXTENDED_POLICY_COUNT );

	    if( 0 != result )
	        return 0;
	
	    //Now set the priority
	    result =   thread_policy_set( machThread,
	                                THREAD_PRECEDENCE_POLICY,
	                                &precedenceData,
	                                THREAD_PRECEDENCE_POLICY_COUNT );

	    if( 0 != result )
	        return 0;

	return 1;
}


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
		Timer t;
		
		if(i%10 == 0) {
			fprintf(stderr, "%d timer resets\n", i); fflush(stderr);
		}
		
		t.startMS(100);
		
		shared_ptr <Clock> clock = Clock::instance();
		clock->sleepMS(10);
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(50);
		
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(70);
		
		CPPUNIT_ASSERT( t.hasExpired() == true );
	}
}

void TimerTestFixture::testTimerResetting(){
	Timer t;
	
	for(int i = 0; i<NUM_TIMER_RESETS; ++i) {
		
		if(i%10 == 0) {
			fprintf(stderr, "%d timer resets\n", i); fflush(stderr);
		}
		
		t.startMS(100);
		
		shared_ptr <Clock> clock = Clock::instance();
		clock->sleepMS(10);
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(50);
		
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(70);
		
		CPPUNIT_ASSERT( t.hasExpired() == true );
	}
}

void TimerTestFixture::testTimerOverride(){
	for(int i = 0; i < NUM_TIMER_RESETS; i++){
		if(i%10 == 0) {
			fprintf(stderr, "%d timer resets\n", i); fflush(stderr);
		}
		Timer t;		
		
		t.startMS(100);
		
		shared_ptr <Clock> clock = Clock::instance();
		clock->sleepMS(10);
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(60);
		
		// 30 ms left to go
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		// Bump that up to 100
		t.startMS(100);
		
		clock->sleepMS(70);
		
		// Should be 30 ms left.
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(40);
		
		// Should be done
		CPPUNIT_ASSERT( t.hasExpired() == true );
		
		t.startMS(100);
		t.startMS(200);
		
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(150);
		
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(60);
		
		CPPUNIT_ASSERT( t.hasExpired() == true );
	}
}

void TimerTestFixture::testTimerOverrideMultipleTimes(){
	Timer t;		

	for(int i = 0; i < NUM_TIMER_RESETS; i++){
		if(i%10 == 0) {
			fprintf(stderr, "%d timer resets\n", i); fflush(stderr);
		}
		
		t.startMS(100);
		
		shared_ptr <Clock> clock = Clock::instance();
		clock->sleepMS(10);
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(60);
		
		// 30 ms left to go
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		// Bump that up to 100
		t.startMS(100);
		
		clock->sleepMS(70);
		
		// Should be 30 ms left.
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(40);
		
		// Should be done
		CPPUNIT_ASSERT( t.hasExpired() == true );
		
		t.startMS(100);
		t.startMS(200);
		
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(150);
		
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(60);
		
		CPPUNIT_ASSERT( t.hasExpired() == true );
	}
}


void TimerTestFixture::testTimerUnderAttack(){

	Timer t;
	
	set_realtime(94);
	
	for(int i = 0; i < 1000; i++){
		MonkeyWorksTime delay = 10000;
		
		shared_ptr <Clock> clock = Clock::instance();
		MonkeyWorksTime then = clock->getCurrentTimeUS();
		t.startUS(delay);
		
		while(!t.hasExpired()) clock->sleepUS(50);
		
		MonkeyWorksTime now = clock->getCurrentTimeUS();
		
		cerr << "Timer fell at expiration-relative time of: " 
			 << (now - then) - delay << endl;
		
		CPPUNIT_ASSERT( (now - then) > delay );
		CPPUNIT_ASSERT( abs( (now - then) - delay ) < 1000 );
		
		
	}

}


void TimerTestFixture::testTimerOverrideMultipleTimesFast(){
	Timer t;		

	set_realtime(94);

	for(int i = 0; i < NUM_TIMER_RESETS; i++){
		if(i%10 == 0) {
			fprintf(stderr, "%d timer resets (fast)\n", i); fflush(stderr);
		}
		
		t.startMS(10);
		
		shared_ptr <Clock> clock = Clock::instance();
		clock->sleepMS(1);
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(6);
		
		// 3 ms left to go
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		// Bump that up to 10
		t.startMS(10);
		
		clock->sleepMS(7);
		
		// Should be 3 ms left.
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(4);
		
		// Should be done
		CPPUNIT_ASSERT( t.hasExpired() == true );
		
		t.startMS(10);
		t.startMS(20);
		
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(15);
		
		CPPUNIT_ASSERT( t.hasExpired() == false );
		
		clock->sleepMS(6);
		
		CPPUNIT_ASSERT( t.hasExpired() == true );
	}
}



void TimerTestFixture::testTimerValue(){
	pthread_t threads[NUM_TIMER_THREADS];
	pthread_attr_t pthread_custom_attr;
	
	pthread_attr_init(&pthread_custom_attr);
	
	Timer t;
	t.setExpired(false);
	timer_CPPUNIT_ASSERT_TS( t.hasExpired() == false );

	const int numberOfSecondsToWait = 1;
	g_timeToEnd = clock() + numberOfSecondsToWait*CLOCKS_PER_SEC;	
	
	for (int i=0; i<NUM_TIMER_THREADS; ++i) {
		timer_CPPUNIT_ASSERT_TS(pthread_create(&threads[i], 
									  &pthread_custom_attr, 
									  &checkTimerValue, (void *)&t) == 0);
	}
	
	t.startMS(2000);
	timer_CPPUNIT_ASSERT_TS( t.hasExpired() == false );
	
	shared_ptr <Clock> clock = Clock::instance();
	clock->sleepMS(1500);
	timer_CPPUNIT_ASSERT_TS( t.hasExpired() == false );
	
	clock->sleepMS(750);
	
	timer_CPPUNIT_ASSERT_TS( t.hasExpired() == true );
				
	for (int i=0; i<NUM_TIMER_THREADS; i++) {
		timer_CPPUNIT_ASSERT_TS(pthread_join(threads[i], NULL) == 0);
	}

	g_timer_cppunit_lock->lock();
	CPPUNIT_ASSERT(g_timerTestPassed);
	g_timer_cppunit_lock->unlock();

}


void TimerTestFixture::testTimerClone(){
	
	Timer t;
	Timer t2(t);
	
	t.startMS(100);
	
	shared_ptr <Clock> clock = Clock::instance();
	clock->sleepMS(10);
	CPPUNIT_ASSERT( t.hasExpired() == false );
	CPPUNIT_ASSERT( t2.hasExpired() == false );
	
	clock->sleepMS(50);
	
	CPPUNIT_ASSERT( t.hasExpired() == false );
	CPPUNIT_ASSERT( t2.hasExpired() == false );
	
	clock->sleepMS(70);
	
	CPPUNIT_ASSERT( t.hasExpired() == true );
	CPPUNIT_ASSERT( t2.hasExpired() == true );
}


void TimerTestFixture::textTimerWithTimebase(){

	set_realtime(94);

	shared_ptr<TimeBase> tb(new TimeBase());
	shared_ptr<Timer> t(new Timer());
	
	MonkeyWorksTime time_amount = 1000;
	shared_ptr<Variable> time_to_wait(new ConstantVariable(time_amount));
	StartTimer a(t,tb, time_to_wait);

	shared_ptr <Clock> clock = Clock::instance();
	MonkeyWorksTime then = clock->getCurrentTimeUS();
	tb->setNow();

	for(int i = 1; i < 5000; i++){
		a.execute();
		
		while(!t->hasExpired())  clock->sleepUS(10);
		MonkeyWorksTime now = clock->getCurrentTimeUS();
		//cerr << "Now: " << now << endl;
		
		MonkeyWorksTime error = (now - then) - (MonkeyWorksTime)(*time_to_wait);
		cerr << "Error: " << (long long) error << endl;
		CPPUNIT_ASSERT( abs(error) < 2000 );
		CPPUNIT_ASSERT( error > 0 );
		
		time_to_wait->setValue(i * time_amount);
		
		
		//cerr << "Waiting: " << (long long)(*time_to_wait) << endl;
	}

}
