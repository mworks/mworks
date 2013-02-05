/*
 *  SchedulerTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 3/20/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SCHEDULER_TEST_H_
#define _SCHEDULER_TEST_H_

#include "FullCoreEnvironmentTest.h"
#include <vector>
#include <boost/thread/mutex.hpp>


BEGIN_NAMESPACE_MW


#define N_SCHEDULED_EXECUTIONS	100
#define SLACK_MARGIN	5
#define ACCEPTABLE_ERROR_US	1000 // 100 us

void *chaff_1(const MWTime interval);

void *counter_no_payload(const boost::shared_ptr<std::vector<MWTime> > &times_array);
void *counter_small_payload(const boost::shared_ptr<std::vector<MWTime> > &times_array);
void *test_for_leaks();
void st_assert(const std::string &message,
				const bool assertCondition);

double random_thing;
Lockable random_thing_lock;

bool st_Asserted;
std::string st_AssertMessage;
boost::mutex st_cppunit_lock;

class SchedulerTestFixture : public FullCoreEnvironmentTestFixture {

	CPPUNIT_TEST_SUITE( SchedulerTestFixture );
	
	CPPUNIT_TEST( testPeriod10HzNoPayload );
	CPPUNIT_TEST( testPeriod10HzSmallPayload );
	CPPUNIT_TEST( testPeriod100HzNoPayload );
	CPPUNIT_TEST( testPeriod10HzNoPayloadChaffX4 );
	//CPPUNIT_TEST( testPeriod10HzNoPayloadChaffX20 );
	CPPUNIT_TEST( testOneOffFiringSmallPayload );	
	CPPUNIT_TEST (testSchedulerLeaks);
																																																																
	CPPUNIT_TEST_SUITE_END();

	
 public:
		void setUp();
	void tearDown();
		
	void testPeriod10HzNoPayload();
	void testPeriod10HzSmallPayload();
	void testPeriod100HzNoPayload();
	void testPeriod10HzNoPayloadChaffX4();
	void testOneOffFiringSmallPayload();
	void testSchedulerLeaks();
	
	
	
protected:
	
	std::vector<MWTime> timeTrial(MWTime interval, 
										   int n_executions,
										   int n_chaff_threads);
	std::vector<MWTime> timeTrialSmallPayload(MWTime interval, 
										   int n_executions,
													   int n_chaff_threads);
	std::vector<MWTime> diff(std::vector<MWTime> times);
	void reportLatencies(std::vector<MWTime> times_array, 
						 MWTime expected);

};


END_NAMESPACE_MW


#endif

