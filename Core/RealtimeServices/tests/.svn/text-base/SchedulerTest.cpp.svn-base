/*
 *  SchedulerTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SchedulerTest.h"
#include "boost/bind.hpp"
using namespace mw;


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SchedulerTestFixture, "Unit Test" );

void SchedulerTestFixture::setUp() {
	shared_ptr <Scheduler> shared_scheduler = Scheduler::instance(false);
	CPPUNIT_ASSERT(shared_scheduler == 0);	
	FullCoreEnvironmentTestFixture::setUp();
	st_AssertMessage = "";
	st_Asserted = false;
}

void SchedulerTestFixture::tearDown() {
	boost::mutex::scoped_lock lock(st_cppunit_lock);
	CPPUNIT_ASSERT_MESSAGE(st_AssertMessage, !st_Asserted);
	FullCoreEnvironmentTestFixture::tearDown();
	shared_ptr <Scheduler> shared_scheduler = Scheduler::instance(false);
	CPPUNIT_ASSERT(shared_scheduler == 0);
}


void SchedulerTestFixture::testPeriod10HzNoPayload(){
	fprintf(stderr, "Running SchedulerTestFixture::testPeriod10HzNoPayload()\n");
	
	fflush(stderr);
	
	MonkeyWorksTime interval = 100000;
	
	std::vector<MonkeyWorksTime> times = timeTrial(interval, 400, 0);
	
	reportLatencies(diff(times), interval);
	
	
	for(unsigned int i = 1 ; i < times.size(); i++){
		CPPUNIT_ASSERT(abs(times[i] - times[i-1] - interval) < ACCEPTABLE_ERROR_US);
	}
	
	
}

void SchedulerTestFixture::testPeriod10HzSmallPayload(){
	fprintf(stderr, "Running SchedulerTestFixture::testPeriod10HzSmallPayload()\n");
	
	fflush(stderr);
	
	MonkeyWorksTime interval = 100000;
	
	std::vector<MonkeyWorksTime> times = timeTrialSmallPayload(interval, 400, 0);
	
	reportLatencies(diff(times), interval);
	
	for(unsigned int i = 1 ; i < times.size(); i++){
		CPPUNIT_ASSERT(abs(times[i] - times[i-1] - interval) < ACCEPTABLE_ERROR_US);
	}
	
	
}

void SchedulerTestFixture::testPeriod100HzNoPayload(){
	fprintf(stderr, "Running SchedulerTestFixture::testPeriod100HzNoPayload()\n");
	
	fflush(stderr);
	
	MonkeyWorksTime interval = 10000; // 100Hz
	
	std::vector<MonkeyWorksTime> times = timeTrial(interval, 400, 0);
	
	reportLatencies(diff(times), interval);
	
	for(unsigned int i = 1 ; i < times.size(); i++){
		CPPUNIT_ASSERT(abs(times[i] - times[i-1] - interval) < ACCEPTABLE_ERROR_US);
	}
	
}

void SchedulerTestFixture::testPeriod10HzNoPayloadChaffX4(){
	fprintf(stderr, "Running SchedulerTestFixture::testPeriod10HzNoPayloadChaffX4()\n");
	
	fflush(stderr);
	
	MonkeyWorksTime interval = 100000; // 10Hz
	
	std::vector<MonkeyWorksTime> times = timeTrial(interval, 400, 4);
	
	reportLatencies(diff(times), interval);
	
	for(unsigned int i = 1 ; i < times.size(); i++){
		CPPUNIT_ASSERT(abs(times[i] - times[i-1] - interval) < ACCEPTABLE_ERROR_US);
	}
	
	
}


void SchedulerTestFixture::testOneOffFiringSmallPayload(){
	fprintf(stderr, "Running SchedulerTestFixture::testOneOffFiringSmallPayload()\n");
	
	fflush(stderr);
	
	MonkeyWorksTime interval = 100000;
	
	shared_ptr<Scheduler> scheduler = Scheduler::instance();
	shared_ptr <Clock> clock = Clock::instance();
	
	CPPUNIT_ASSERT(clock != NULL);
	CPPUNIT_ASSERT(scheduler != NULL);
	
	std::vector<MonkeyWorksTime> times;
	
	
	
	for(int i = 0; i < 400; i++){
		
		MonkeyWorksTime now = clock->getCurrentTimeUS();
		
		shared_ptr<std::vector<MonkeyWorksTime> > times_array = shared_ptr<std::vector<MonkeyWorksTime> >(new std::vector<MonkeyWorksTime> ());
		// schedule n executions
		shared_ptr<ScheduleTask> node = scheduler->scheduleUS(FILELINE,
															   interval,				// some delay
															   0,						// no interval
															   1,						// one shot
															   boost::bind(counter_no_payload, times_array),
															   M_DEFAULT_PRIORITY,
															   M_DEFAULT_WARN_SLOP_US,
															   M_DEFAULT_FAIL_SLOP_US,
															   M_MISSED_EXECUTION_DROP);
		
		
		// wait an appropriate amount of time for it to finish
		clock->sleepUS(2 * interval);
		
		
		
		CPPUNIT_ASSERT( times_array->size() > 0 );
		times.push_back(times_array->at(0) - now);
		fprintf(stderr, "diff = %lld\n", times_array->at(0) - now); fflush(stderr);
	}
	
	reportLatencies(times, interval);
	
}

void SchedulerTestFixture::testSchedulerLeaks() {
	fprintf(stderr, "Running SchedulerTestFixture::testSchedulerLeaks()\n");
	
	MonkeyWorksTime howlongus = 1000;
	
	shared_ptr<ScheduleTask> node;
	st_assert("node is not initialized correctly", node == 0);
	st_assert("node is not initialized correctly", node.use_count() == 0);
	
	shared_ptr<Scheduler> scheduler = Scheduler::instance();
	
	node = scheduler->scheduleUS(FILELINE,
								 howlongus,
								 0, 
								 1, 
								 boost::bind(test_for_leaks),
								 M_DEFAULT_PRIORITY,
								 M_DEFAULT_WARN_SLOP_US,
								 M_DEFAULT_FAIL_SLOP_US,
								 M_MISSED_EXECUTION_CATCH_UP);
	
	sleep(5);
	st_assert("something is wrong at this point", node);
	st_assert("node should have one reference at this point", node.use_count() == 1);
}


// protected:

std::vector<MonkeyWorksTime> SchedulerTestFixture::timeTrial(MonkeyWorksTime interval, 
															  int n_executions,
															  int n_chaff_threads){
	
	shared_ptr<Scheduler> scheduler = Scheduler::instance();
	shared_ptr <Clock> clock = Clock::instance();
	
	CPPUNIT_ASSERT(clock != NULL);
	CPPUNIT_ASSERT(scheduler != NULL);
	
	
	shared_ptr<std::vector<MonkeyWorksTime> > times_array = shared_ptr<std::vector<MonkeyWorksTime> >(new std::vector<MonkeyWorksTime>());
	
	std::vector<shared_ptr<ScheduleTask> > chaff;
	
	for(int i = 0; i < n_chaff_threads; i++){
		shared_ptr<ScheduleTask> chaff_node = scheduler->scheduleUS(FILELINE,
																	 0,							// no delay
																	 interval,					// 10Hz
																	 M_REPEAT_INDEFINITELY,		// how many
																	 boost::bind(chaff_1, interval),
																	 M_DEFAULT_PRIORITY,
																	 M_DEFAULT_WARN_SLOP_US,
																	 M_DEFAULT_FAIL_SLOP_US,
																	 M_MISSED_EXECUTION_DROP);
		chaff.push_back(chaff_node);
	}
	
	
	
	// schedule n executions
	shared_ptr<ScheduleTask> node = scheduler->scheduleUS(FILELINE,
														   
														   0,							// no delay
														   interval,					// 10Hz
														   n_executions,		// how many
														   boost::bind(counter_no_payload,
																	   times_array),
														   M_DEFAULT_PRIORITY,
														   M_DEFAULT_WARN_SLOP_US,
														   M_DEFAULT_FAIL_SLOP_US,
														   M_MISSED_EXECUTION_DROP);
	
	
	// wait an appropriate amount of time for it to finish
	clock->sleepUS((SLACK_MARGIN +n_executions) * interval);
	node->cancel();
	
	
	
	for(unsigned int i = 0; i < chaff.size(); i++){
		chaff[i]->cancel();
	}
	
	// wait, just in case
	clock->sleepUS(10000);
	
	return *times_array;
}


// sloppiness
std::vector<MonkeyWorksTime> SchedulerTestFixture::timeTrialSmallPayload(MonkeyWorksTime interval, 
																		  int n_executions,
																		  int n_chaff_threads){
	
	shared_ptr<Scheduler> scheduler = Scheduler::instance();
	shared_ptr <Clock> clock = Clock::instance();
	
	CPPUNIT_ASSERT(clock != NULL);
	CPPUNIT_ASSERT(scheduler != NULL);
	
	
	
	std::vector<shared_ptr<ScheduleTask> > chaff;
	
	for(int i = 0; i < n_chaff_threads; i++){
		shared_ptr<ScheduleTask> chaff_node = scheduler->scheduleUS(FILELINE,
																	 0,							// no delay
																	 interval,					// 10Hz
																	 M_REPEAT_INDEFINITELY,		// how many
																	 boost::bind(chaff_1, interval),
																	 M_DEFAULT_PRIORITY,
																	 M_DEFAULT_WARN_SLOP_US,
																	 M_DEFAULT_FAIL_SLOP_US,
																	 M_MISSED_EXECUTION_DROP);
		chaff.push_back(chaff_node);
	}
	
	
	shared_ptr<std::vector<MonkeyWorksTime> > times_array = shared_ptr<std::vector<MonkeyWorksTime> >(new std::vector<MonkeyWorksTime>());
	
	// schedule n executions
	shared_ptr<ScheduleTask> node = scheduler->scheduleUS(FILELINE,
														   0,							// no delay
														   interval,					// 10Hz
														   n_executions,		// how many
														   boost::bind(counter_small_payload, times_array),
														   M_DEFAULT_PRIORITY,
														   M_DEFAULT_WARN_SLOP_US,
														   M_DEFAULT_FAIL_SLOP_US,
														   M_MISSED_EXECUTION_DROP);
	
	
	// wait an appropriate amount of time for it to finish
	clock->sleepUS((SLACK_MARGIN +n_executions) * interval);
	node->cancel();
	
	
	for(unsigned int i = 0; i < chaff.size(); i++){
		chaff[i]->cancel();
	}
	
	// wait, just in case
	clock->sleepUS(10000);
	
	
	return *times_array;
}


std::vector<MonkeyWorksTime> SchedulerTestFixture::diff(std::vector<MonkeyWorksTime> times){
	std::vector<MonkeyWorksTime> returnval;
	for(unsigned int i = 1 ; i < times.size(); i++){
		MonkeyWorksTime diff = abs(times[i] - times[i-1]);
		returnval.push_back(diff);
	}
	return returnval;
}

void SchedulerTestFixture::reportLatencies(std::vector<MonkeyWorksTime> times_array, 
											MonkeyWorksTime expected){
	
	int less_than_5us = 0;
	int less_than_10us = 0;
	int less_than_50us = 0;
	int less_than_100us = 0;
	int less_than_500us = 0;
	int less_than_1000us = 0;
	int less_than_5000us = 0;
	int less_than_10000us = 0;
	int less_than_15000us = 0;
	int more_than_15000us = 0;
	int n = 0;
	
	for(unsigned int i = 0 ; i < times_array.size(); i++){
		MonkeyWorksTime diff = times_array[i] - expected;
		
		if(diff < 5){
			less_than_5us++;
		} else if(diff < 10){
			less_than_10us++;
		} else if(diff < 50){
			less_than_50us++;
		} else if(diff < 100){
			less_than_100us++;
		} else if(diff < 500){
			less_than_500us++;
		} else if(diff < 1000){
			less_than_1000us++;
		} else if(diff < 5000){
			less_than_5000us++;
		} else if(diff < 10000){
			less_than_10000us++;
		} else if(diff < 15000){
			less_than_15000us++;
		} else {
			more_than_15000us++;
		}
		n++;
	}
	
	fprintf(stderr, "Latency Statistics...\n"
			"\t< 5us:  %.4g\n"
			"\t< 10us:  %.4g\n"
			"\t< 50us:  %.4g\n"
			"\t< 100us:  %.4g\n"
			"\t< 500us:  %.4g\n"
			"\t< 1000us:  %.4g\n"
			"\t< 5000us:  %.4g\n"
			"\t< 10000us:  %.4g\n"
			"\t< 15000us:  %.4g\n"
			"\t> 15000us:  %.4g\n",
			
			100. * (double)less_than_5us / (double)n,
			100. * (double)less_than_10us / (double)n,
			100. * (double)less_than_50us / (double)n,
			100. * (double)less_than_100us / (double)n,
			100. * (double)less_than_500us / (double)n,
			100. * (double)less_than_1000us / (double)n,
			100. * (double)less_than_5000us / (double)n,
			100. * (double)less_than_10000us / (double)n,
			100. * (double)less_than_15000us / (double)n,
			100. * (double)more_than_15000us / (double)n);
	fflush(stderr);
}	


namespace mw {
	void *counter_no_payload(const boost::shared_ptr<std::vector<MonkeyWorksTime> > &times_array){
		shared_ptr <Clock> clock = Clock::instance();
		MonkeyWorksTime now = clock->getCurrentTimeUS();
		
		times_array->push_back(clock->getCurrentTimeUS());
		fprintf(stderr, "tac %lld\n", now); fflush(stderr);
		
		return 0;
	}
	
	void *counter_small_payload(const boost::shared_ptr<std::vector<MonkeyWorksTime> > &times_array){
		shared_ptr <Clock> clock = Clock::instance();	
		
		for(int i = 0; i < 1000; i++){
			random_thing_lock.lock();
			random_thing += (double)rand();
			random_thing_lock.unlock();
		}
		
		times_array->push_back(clock->getCurrentTimeUS());
		//fprintf(stderr, "tac %lld, %g\n", now, random_thing); fflush(stderr);
		return 0;
	}
	
	void *chaff_1(const MonkeyWorksTime interval){
		MonkeyWorksTime how_long = interval - 100;
		
		shared_ptr <Clock> clock = Clock::instance();	
		MonkeyWorksTime then = clock->getCurrentTimeUS(); 
		MonkeyWorksTime now = then;
		
		double blah = 0.5;
		
		while( (now - then) < how_long ){
			blah += (double)rand();
			now = clock->getCurrentTimeUS();
		}
		
		return 0;
	}
	
	void *test_for_leaks() {
		usleep(1000);
		st_assert("test assert", true);
		return 0;
	}
	
	void st_assert(const std::string &message, 
				   const bool assertCondition) {
		boost::mutex::scoped_lock lock(st_cppunit_lock); 
		if(!assertCondition && !st_Asserted) {
			st_Asserted = true;
			st_AssertMessage = message;
		}
	}
} 
