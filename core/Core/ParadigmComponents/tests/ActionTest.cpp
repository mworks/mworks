/*
 *  ActionTest.cpp
 *  MWorksCore
 *
 *  Created by labuser on 10/2/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#include "ActionTest.h"
#include "VariableRegistry.h"
#include "TrialBuildingBlocks.h"
#include "GenericVariable.h"
#include "ScheduledActions.h"

#include <boost/bind.hpp>


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ActionTestFixture, "Unit Test" );


void ActionTestFixture::testSimpleAssignment1() {
	shared_ptr<ConstantVariable>c1 = shared_ptr<ConstantVariable>(new ConstantVariable(Datum(M_FLOAT, 7.62)));
	shared_ptr<GlobalVariable>v1 =  global_variable_registry->createGlobalVariable( VariableProperties(Datum(0L), "test1",
																										   "Test",
																										   "Test",
																										   M_NEVER, M_WHEN_CHANGED,
																										   true, false,
																										   M_CONTINUOUS_INFINITE,""));	
	
	Assignment a(v1,c1);
	
	for (int i=0; i<5000; ++i) {
		a.execute();
		CPPUNIT_ASSERT(v1->getValue().getFloat() == c1->getValue().getFloat());
		float seven_point_six_two = 7.62;
		CPPUNIT_ASSERT((float)*v1 == seven_point_six_two);		
	}	
}

void ActionTestFixture::testSimpleAssignment2() {
	shared_ptr<GlobalVariable>v1 =  global_variable_registry->createGlobalVariable( VariableProperties(
																										   Datum(0L), "test1",
																										   "Test",
																										   "Test",
																										   M_NEVER, M_WHEN_CHANGED,
																										   true, false,
																										   M_CONTINUOUS_INFINITE,""));	
	
	
	for (int i=0; i<5000; ++i) {
		shared_ptr<ConstantVariable>c1 = shared_ptr<ConstantVariable>(new ConstantVariable(Datum(M_INTEGER,i)));
		Assignment a(v1,c1);
		a.execute();
		CPPUNIT_ASSERT(v1->getValue().getInteger() == c1->getValue().getInteger());
		CPPUNIT_ASSERT((long)*v1 == i);
	}	
}


void ActionTestFixture::testIncrementCounterWithAssignment(){
	
	
	shared_ptr<GlobalVariable>v1_ptr =  
	global_variable_registry->createGlobalVariable( VariableProperties(
																		  Datum(0L), "test1",
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	Variable *v1 = v1_ptr.get();
	shared_ptr<Variable> e_ptr(new ParsedExpressionVariable("test1 + 1"));
	
	Assignment a(v1_ptr, e_ptr);
	
	for(int i = 0; i < 5000; i++){
		CPPUNIT_ASSERT( (long)(*v1) == i );		
		a.execute(); // do it with the action
	}
	
}


void ActionTestFixture::testIncrementCounterWithAssignmentManyThreads(){
	
	
	shared_ptr<GlobalVariable>v1_ptr =  
	global_variable_registry->createGlobalVariable( VariableProperties(
																		  Datum(0L), "test1",
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	Variable *v1 = v1_ptr.get();
	shared_ptr<Variable> e_ptr(new ParsedExpressionVariable("test1 + 1"));
	
	Assignment a(v1_ptr, e_ptr);
	
	for(int i = 0; i < 5000; i++){
		CPPUNIT_ASSERT( (long)(*v1) == i );		
		a.execute(); // do it with the action
	}
	
}


void ActionTestFixture::testScheduledAssignment() {
	const MWTime DELAY_US = 4000000;
	
	shared_ptr<ConstantVariable>c1 = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	shared_ptr<GlobalVariable>v1 =  global_variable_registry->createGlobalVariable( VariableProperties(Datum(0L), "test1",
																										   "Test",
																										   "Test",
																										   M_NEVER, M_WHEN_CHANGED,
																										   true, false,
																										   M_CONTINUOUS_INFINITE,""));	
	
	CPPUNIT_ASSERT((long)*v1 == 0);	
	
	shared_ptr<ConstantVariable>delay = shared_ptr<ConstantVariable>(new ConstantVariable(DELAY_US));
	shared_ptr<ConstantVariable>repeats = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	shared_ptr<ConstantVariable>interval = shared_ptr<ConstantVariable>(new ConstantVariable(0L));
	shared_ptr<ConstantVariable>cancel(new ConstantVariable(0L));
	
	shared_ptr<ScheduledActions> sa = shared_ptr<ScheduledActions>(new ScheduledActions(repeats, delay, interval, cancel));
	shared_ptr<Assignment>a = shared_ptr<Assignment>(new Assignment(v1,c1));
	CPPUNIT_ASSERT((long)*v1 == 0L);	
	
	sa->addAction(a);
	CPPUNIT_ASSERT((long)*v1 == 0L);		
	
	sa->execute();
	CPPUNIT_ASSERT((long)*v1 == 0L);
	shared_ptr <Clock> clock = Clock::instance();
	MWTime start_time_us = clock->getCurrentTimeUS();
	
	while((long)*v1 == 0 && clock->getCurrentTimeUS() < start_time_us+2*DELAY_US) {
	}
	
	// should be done before 2*DELAY_US has passed
	CPPUNIT_ASSERT(clock->getCurrentTimeUS() < start_time_us+2*DELAY_US);	
	CPPUNIT_ASSERT((long)*v1 == 1L);	
}

void ActionTestFixture::testScheduledAssignmentWithCancel() {
	const MWTime DELAY_US = 4000000;
	const MWTime CANCEL_TIME_US = DELAY_US/10;
	
	shared_ptr<ConstantVariable>c1 = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	shared_ptr<GlobalVariable>v1 =  global_variable_registry->createGlobalVariable( VariableProperties(Datum(0L), "test1",
																										   "Test",
																										   "Test",
																										   M_NEVER, M_WHEN_CHANGED,
																										   true, false,
																										   M_CONTINUOUS_INFINITE,""));	
	
	CPPUNIT_ASSERT((long)*v1 == 0);	
	
	shared_ptr<ConstantVariable>delay = shared_ptr<ConstantVariable>(new ConstantVariable(DELAY_US));
	shared_ptr<ConstantVariable>repeats = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	shared_ptr<ConstantVariable>interval = shared_ptr<ConstantVariable>(new ConstantVariable(0L));
	shared_ptr<GlobalVariable>cancel(new GlobalVariable(0L));
	shared_ptr<ScheduledActions>sa = shared_ptr<ScheduledActions>(new ScheduledActions(repeats, delay, interval, cancel));
	shared_ptr<Assignment>a = shared_ptr<Assignment>(new Assignment(v1,c1));
	CPPUNIT_ASSERT((long)*v1 == 0L);	
	
	sa->addAction(a);
	CPPUNIT_ASSERT((long)*v1 == 0L);
	
	shared_ptr <Clock> clock = Clock::instance();
	MWTime start_time_us = clock->getCurrentTimeUS();
	sa->execute();
	CPPUNIT_ASSERT((long)*v1 == 0L);	
	
	usleep(CANCEL_TIME_US);
	CPPUNIT_ASSERT((long)*v1 == 0L);	
	
	
    cancel->setValue(1L);
	
	CPPUNIT_ASSERT((long)*v1 == 0L);	
	
	// it should never change
	while(clock->getCurrentTimeUS() < start_time_us+2*DELAY_US) {
		CPPUNIT_ASSERT((long)*v1 == 0L);	
	}
}

void ActionTestFixture::testScheduledAssignmentWithCancelThatsTooLate() {
	const MWTime DELAY_US = 1000000;
	const MWTime CANCEL_TIME_US = 2*DELAY_US;
	
	shared_ptr<ConstantVariable>c1 = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	shared_ptr<GlobalVariable>v1 =  global_variable_registry->createGlobalVariable( VariableProperties(Datum(0L), "test1",
																										   "Test",
																										   "Test",
																										   M_NEVER, M_WHEN_CHANGED,
																										   true, false,
																										   M_CONTINUOUS_INFINITE,""));	
	
	CPPUNIT_ASSERT((long)*v1 == 0);	
	
	shared_ptr<ConstantVariable>delay = shared_ptr<ConstantVariable>(new ConstantVariable(DELAY_US));
	shared_ptr<ConstantVariable>repeats = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	shared_ptr<ConstantVariable>interval = shared_ptr<ConstantVariable>(new ConstantVariable(0L));
	shared_ptr<GlobalVariable>cancel(new GlobalVariable(0L));
	shared_ptr<ScheduledActions>sa = shared_ptr<ScheduledActions>(new ScheduledActions(repeats, delay, interval, cancel));
	shared_ptr<Assignment>a = shared_ptr<Assignment>(new Assignment(v1,c1));
	CPPUNIT_ASSERT((long)*v1 == 0L);	
	
	sa->addAction(a);
	CPPUNIT_ASSERT((long)*v1 == 0L);
	
	shared_ptr <Clock> clock = Clock::instance();
	sa->execute();
	CPPUNIT_ASSERT((long)*v1 == 0L);	
	
	usleep(CANCEL_TIME_US);
	CPPUNIT_ASSERT((long)*v1 == 1L);	
	
    cancel->setValue(1L);
	
	CPPUNIT_ASSERT((long)*v1 == 1L);	
}


END_NAMESPACE_MW



