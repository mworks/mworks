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


void ActionTestFixture::setUp() {
    FullCoreEnvironmentTestFixture::setUp();
    
    messageCallbackNotification = shared_ptr<VariableCallbackNotification>(new VariableCallbackNotification(boost::bind(&ActionTestFixture::messageCallback, this, _1, _2)));
    shared_ptr<Variable> messageVar = global_variable_registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
    messageVar->addNotification(messageCallbackNotification);
}


void ActionTestFixture::tearDown() {
    messageCallbackNotification->remove();
    FullCoreEnvironmentTestFixture::tearDown();
}


void ActionTestFixture::messageCallback(const Datum &data, MWorksTime time) {
    if (data.isDictionary()) {
        messagePackage = data;
    }
}


void ActionTestFixture::testSimpleAssignment1() {
	shared_ptr<ConstantVariable>c1 = shared_ptr<ConstantVariable>(new ConstantVariable(Datum(M_FLOAT, 7.62)));
	shared_ptr<GlobalVariable>v1 =  global_variable_registry->createGlobalVariable( new VariableProperties(new Datum(0L), "test1",
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
	shared_ptr<GlobalVariable>v1 =  global_variable_registry->createGlobalVariable( new VariableProperties(
																										   new Datum(0L), "test1",
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
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(0L), "test1",
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	Variable *v1 = v1_ptr.get();
	ConstantVariable c((long)1);
	ExpressionVariable e = *v1 + c;
	shared_ptr<Variable> e_ptr(e.clone());
	
	Assignment a(v1_ptr, e_ptr);
	
	for(int i = 0; i < 5000; i++){
		CPPUNIT_ASSERT( (long)(*v1) == i );		
		a.execute(); // do it with the action
	}
	
}


void ActionTestFixture::testIncrementCounterWithAssignmentManyThreads(){
	
	
	shared_ptr<GlobalVariable>v1_ptr =  
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(0L), "test1",
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	Variable *v1 = v1_ptr.get();
	ConstantVariable c((long)1);
	ExpressionVariable e = *v1 + c;
	shared_ptr<Variable> e_ptr(e.clone());
	
	Assignment a(v1_ptr, e_ptr);
	
	for(int i = 0; i < 5000; i++){
		CPPUNIT_ASSERT( (long)(*v1) == i );		
		a.execute(); // do it with the action
	}
	
}

void ActionTestFixture::testConstantReport() {
	std::string testStringSeed("This is my test string!!!!");	
	
	shared_ptr <Variable> messageVar = global_variable_registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(testStringSeed);
		}
		
		ReportString r(testString);
		r.execute();

		CPPUNIT_ASSERT(messagePackage.isDictionary());
		CPPUNIT_ASSERT(messagePackage.getElement(M_MESSAGE).isString());
		
		
	 Datum msgType(messagePackage.getElement(M_MESSAGE_TYPE));
		CPPUNIT_ASSERT(msgType.isInteger());
		MessageType type = (MessageType)msgType.getInteger();
		
		CPPUNIT_ASSERT(type == M_GENERIC_MESSAGE);
		std::string message(messagePackage.getElement(M_MESSAGE).getString());
		std::string stringToCompare(testString + " (");
		
		if(stringToCompare.size() < message.size()) {
			CPPUNIT_ASSERT(message.compare(0, stringToCompare.size(), stringToCompare) == 0);
		} else {
			CPPUNIT_ASSERT(stringToCompare.compare(0, message.size(), message) == 0);			
		}
	}
}


void ActionTestFixture::testReportWithVariableAtEnd() {
	std::string testVarName("testVar");
	std::string testStringSeed("This is my test var: ");	
	std::string testVar("$" + testVarName);
	std::string testVarValueString("testVarValueString");
	
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(testVarValueString), 
																		  testVarName,
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	
	
	shared_ptr <Variable> messageVar = global_variable_registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		std::string testStringCompare = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(" " + testStringSeed + testVar);
			testStringCompare.append(" " + testStringSeed + testVarValueString);
		}
		
		ReportString r(testString);
		r.execute();

		CPPUNIT_ASSERT(messagePackage.isDictionary());
		CPPUNIT_ASSERT(messagePackage.getElement(M_MESSAGE).isString());
		
		
	 Datum msgType(messagePackage.getElement(M_MESSAGE_TYPE));
		CPPUNIT_ASSERT(msgType.isInteger());
		MessageType type = (MessageType)msgType.getInteger();
		
		CPPUNIT_ASSERT(type == M_GENERIC_MESSAGE);
		
		std::string message(messagePackage.getElement(M_MESSAGE).getString());
		std::string stringToCompare(testStringCompare + " (");
		
		if(stringToCompare.size() < message.size()) {
			CPPUNIT_ASSERT(message.compare(0, stringToCompare.size(), stringToCompare) == 0);
		} else {
			CPPUNIT_ASSERT(stringToCompare.compare(0, message.size(), message) == 0);			
		}
	}
}	

void ActionTestFixture::testReportWithVariableAtBeginning() {
	std::string testVarName("testVar");
	std::string testStringSeed("<= This was my test var");	
	std::string testVar("$" + testVarName);
	std::string testVarValueString("testVarValueString");
	
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(testVarValueString), 
																		  testVarName,
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	
	
	shared_ptr <Variable> messageVar = global_variable_registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		std::string testStringCompare = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(testVar + " " + testStringSeed);
			testStringCompare.append(testVarValueString + " " + testStringSeed);
		}
		
		ReportString r(testString);
		r.execute();

		CPPUNIT_ASSERT(messagePackage.isDictionary());
		CPPUNIT_ASSERT(messagePackage.getElement(M_MESSAGE).isString());
		
		
	 Datum msgType(messagePackage.getElement(M_MESSAGE_TYPE));
		CPPUNIT_ASSERT(msgType.isInteger());
		MessageType type = (MessageType)msgType.getInteger();
		
		CPPUNIT_ASSERT(type == M_GENERIC_MESSAGE);
		
		std::string message(messagePackage.getElement(M_MESSAGE).getString());
		std::string stringToCompare(testStringCompare + " (");
		
		if(stringToCompare.size() < message.size()) {
			CPPUNIT_ASSERT(message.compare(0, stringToCompare.size(), stringToCompare) == 0);
		} else {
			CPPUNIT_ASSERT(stringToCompare.compare(0, message.size(), message) == 0);			
		}
	}
}

void ActionTestFixture::testReportWithVariableInMiddle() {
	std::string testVarName("testVar");
	std::string testStringSeed1("This is my test var =>");	
	std::string testStringSeed2("<= This was my test var");	
	std::string testVar("$" + testVarName);
	std::string testVarValueString("testVarValueString");
	
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(testVarValueString), 
																		  testVarName,
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	
	
	shared_ptr <Variable> messageVar = global_variable_registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		std::string testStringCompare = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(testStringSeed1 + testVar + " " + testStringSeed2);
			testStringCompare.append(testStringSeed1 + testVarValueString + " " + testStringSeed2);
		}
		
		ReportString r(testString);
		r.execute();

		CPPUNIT_ASSERT(messagePackage.isDictionary());
		CPPUNIT_ASSERT(messagePackage.getElement(M_MESSAGE).isString());
		
		
	 Datum msgType(messagePackage.getElement(M_MESSAGE_TYPE));
		CPPUNIT_ASSERT(msgType.isInteger());
		MessageType type = (MessageType)msgType.getInteger();
		
		CPPUNIT_ASSERT(type == M_GENERIC_MESSAGE);
		
		std::string message(messagePackage.getElement(M_MESSAGE).getString());
		std::string stringToCompare(testStringCompare + " (");
		
		if(stringToCompare.size() < message.size()) {
			CPPUNIT_ASSERT(message.compare(0, stringToCompare.size(), stringToCompare) == 0);
		} else {
			CPPUNIT_ASSERT(stringToCompare.compare(0, message.size(), message) == 0);			
		}
	}
}

void ActionTestFixture::testReportWith2Variables() {
	std::string testVarName("testVar");
	std::string testVarName2("testVar2");
	std::string testStringSeed("<= This was my test var and this is testVar2 =>");	
	std::string testVar("$" + testVarName);
	std::string testVar2("$" + testVarName2);
	std::string testVarValueString("testVarValueString");
	std::string testVarValueString2("testVarValueString2");
	
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(testVarValueString), 
																		  testVarName,
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(testVarValueString2), 
																		  testVarName2,
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	
	
	shared_ptr <Variable> messageVar = global_variable_registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		std::string testStringCompare = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(testVar + " " + testStringSeed + testVar2 + " ");
			testStringCompare.append(testVarValueString + " " + testStringSeed + testVarValueString2 + " ");
		}
		
		ReportString r(testString);
		r.execute();

		CPPUNIT_ASSERT(messagePackage.isDictionary());
		CPPUNIT_ASSERT(messagePackage.getElement(M_MESSAGE).isString());
		
		
	 Datum msgType(messagePackage.getElement(M_MESSAGE_TYPE));
		CPPUNIT_ASSERT(msgType.isInteger());
		MessageType type = (MessageType)msgType.getInteger();
		
		CPPUNIT_ASSERT(type == M_GENERIC_MESSAGE);
		
		std::string message(messagePackage.getElement(M_MESSAGE).getString());
		std::string stringToCompare(testStringCompare + " (");
		
		if(stringToCompare.size() < message.size()) {
			CPPUNIT_ASSERT(message.compare(0, stringToCompare.size(), stringToCompare) == 0);
		} else {
			CPPUNIT_ASSERT(stringToCompare.compare(0, message.size(), message) == 0);			
		}
	}	
}


void ActionTestFixture::testReportWithBadVariable() {
	std::string testVarName("testVar");
	std::string testStringSeed("<= This was my test var");	
	std::string testVar("$" + testVarName);
	std::string testVarValueString("testVarValueString");
	std::string unknownVar("UNKNOWNVAR");
	
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(testVarValueString), 
																		  testVarName + "Extra",
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	
	
	shared_ptr <Variable> messageVar = global_variable_registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		std::string testStringCompare = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(testVar + " " + testStringSeed);
			testStringCompare.append(unknownVar + " " + testStringSeed);
		}
		
		ReportString r(testString);
		r.execute();

		CPPUNIT_ASSERT(messagePackage.isDictionary());
		CPPUNIT_ASSERT(messagePackage.getElement(M_MESSAGE).isString());
		
		
	 Datum msgType(messagePackage.getElement(M_MESSAGE_TYPE));
		CPPUNIT_ASSERT(msgType.isInteger());
		MessageType type = (MessageType)msgType.getInteger();
		
		if(i>0) {
			CPPUNIT_ASSERT(type == M_ERROR_MESSAGE);
			
			std::string message(messagePackage.getElement(M_MESSAGE).getString());
			std::string stringToCompare("ERROR: " + testStringCompare + " (");
			
			if(stringToCompare.size() < message.size()) {
				CPPUNIT_ASSERT(message.compare(0, stringToCompare.size(), stringToCompare) == 0);
			} else {
				CPPUNIT_ASSERT(stringToCompare.compare(0, message.size(), message) == 0);			
			}
		} else {
			// this first time through the message size is 0 so there is no error
			CPPUNIT_ASSERT(type == M_GENERIC_MESSAGE);
		}
	}
}


void ActionTestFixture::testReportWithOneGoodVariableAndOneBadVariable() {
	std::string testVarName("testVar");
	std::string testVarName2("testVar2");
	std::string testStringSeed("<= This was my test var and this is testVar2 =>");	
	std::string testVar("$" + testVarName);
	std::string testVar2("$" + testVarName2);
	std::string testVarValueString("testVarValueString");
	std::string testVarValueString2("testVarValueString2");
	std::string unknownVar("UNKNOWNVAR");
	
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(testVarValueString), 
																		  testVarName + "Extra",
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(testVarValueString2), 
																		  testVarName2,
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	
	
	shared_ptr <Variable> messageVar = global_variable_registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		std::string testStringCompare = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(testVar + " " + testStringSeed + testVar2 + " ");
			testStringCompare.append(unknownVar + " " + testStringSeed + testVarValueString2 + " ");
		}
		
		ReportString r(testString);
		r.execute();

		CPPUNIT_ASSERT(messagePackage.isDictionary());
		CPPUNIT_ASSERT(messagePackage.getElement(M_MESSAGE).isString());
		
		
	 Datum msgType(messagePackage.getElement(M_MESSAGE_TYPE));
		CPPUNIT_ASSERT(msgType.isInteger());
		MessageType type = (MessageType)msgType.getInteger();
		
		if(i>0) {
			CPPUNIT_ASSERT(type == M_ERROR_MESSAGE);
			
			std::string message(messagePackage.getElement(M_MESSAGE).getString());
			std::string stringToCompare("ERROR: " + testStringCompare + " (");
			
			if(stringToCompare.size() < message.size()) {
				CPPUNIT_ASSERT(message.compare(0, stringToCompare.size(), stringToCompare) == 0);
			} else {
				CPPUNIT_ASSERT(stringToCompare.compare(0, message.size(), message) == 0);			
			}
		} else {
			// this first time through the message size is 0 so there is no error
			CPPUNIT_ASSERT(type == M_GENERIC_MESSAGE);
		}
	}	
}

void ActionTestFixture::testAssert() {
	std::string testStringSeed("This is my test string!!!!");	
	
	shared_ptr <Variable> messageVar = global_variable_registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	shared_ptr <ConstantVariable> conditionFail = shared_ptr<ConstantVariable>(new ConstantVariable(0L));
	shared_ptr <ConstantVariable> conditionPass = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(testStringSeed);
		}
		
		AssertionAction aa(conditionPass, testString);
		aa.execute();
		// not much to test...it passes
	}
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(testStringSeed);
		}
		
		AssertionAction aa(conditionFail, testString);
		aa.execute();
		
		CPPUNIT_ASSERT(messagePackage.isDictionary());
		CPPUNIT_ASSERT(messagePackage.getElement(M_MESSAGE).isString());
		
		
	 Datum msgType(messagePackage.getElement(M_MESSAGE_TYPE));
		CPPUNIT_ASSERT(msgType.isInteger());
		MessageType type = (MessageType)msgType.getInteger();
		
		CPPUNIT_ASSERT(type == M_ERROR_MESSAGE);
		
		std::string message(messagePackage.getElement(M_MESSAGE).getString());
		std::string stringToCompare("ERROR: Assertion: " + testString + " (");
		
		if(stringToCompare.size() < message.size()) {
			CPPUNIT_ASSERT(message.compare(0, stringToCompare.size(), stringToCompare) == 0);
		} else {
			CPPUNIT_ASSERT(stringToCompare.compare(0, message.size(), message) == 0);			
		}
	}
}


void ActionTestFixture::testAssertWithVariableAtEnd() {
	std::string testVarName("testVar");
	std::string testStringSeed("This is my test var: ");	
	std::string testVar("$" + testVarName);
	std::string testVarValueString("testVarValueString");
	
	global_variable_registry->createGlobalVariable( new VariableProperties(
																		  new Datum(testVarValueString), 
																		  testVarName,
																		  "Test",
																		  "Test",
																		  M_NEVER, M_WHEN_CHANGED,
																		  true, false,
																		  M_CONTINUOUS_INFINITE,""));	
	
	
	
	shared_ptr <Variable> messageVar = global_variable_registry->getVariable(ANNOUNCE_MESSAGE_VAR_TAGNAME);
	shared_ptr <ConstantVariable> conditionFail = shared_ptr<ConstantVariable>(new ConstantVariable(0L));
	shared_ptr <ConstantVariable> conditionPass = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		std::string testStringCompare = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(" " + testStringSeed + testVar);
			testStringCompare.append(" " + testStringSeed + testVarValueString);
		}
		
		AssertionAction aa(conditionPass, testString);
		aa.execute();
		// not much to test...it passes
	}
	
	for(int i = 0; i<75; ++i) {
		std::string testString = "";
		std::string testStringCompare = "";
		
		for(int j=0; j<i; ++j) {
			testString.append(" " + testStringSeed + testVar);
			testStringCompare.append(" " + testStringSeed + testVarValueString);
		}
		
		AssertionAction aa(conditionFail, testString);
		aa.execute();

		CPPUNIT_ASSERT(messagePackage.isDictionary());
		CPPUNIT_ASSERT(messagePackage.getElement(M_MESSAGE).isString());
		
		
	 Datum msgType(messagePackage.getElement(M_MESSAGE_TYPE));
		CPPUNIT_ASSERT(msgType.isInteger());
		MessageType type = (MessageType)msgType.getInteger();
		
		CPPUNIT_ASSERT(type == M_ERROR_MESSAGE);
		
		std::string message(messagePackage.getElement(M_MESSAGE).getString());
		std::string stringToCompare("ERROR: Assertion: " + testStringCompare + " (");
		
		if(stringToCompare.size() < message.size()) {
			CPPUNIT_ASSERT(message.compare(0, stringToCompare.size(), stringToCompare) == 0);
		} else {
			CPPUNIT_ASSERT(stringToCompare.compare(0, message.size(), message) == 0);			
		}
	}
}	


void ActionTestFixture::testScheduledAssignment() {
	const MWTime DELAY_US = 4000000;
	
	shared_ptr<ConstantVariable>c1 = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	shared_ptr<GlobalVariable>v1 =  global_variable_registry->createGlobalVariable( new VariableProperties(new Datum(0L), "test1",
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
	shared_ptr<GlobalVariable>v1 =  global_variable_registry->createGlobalVariable( new VariableProperties(new Datum(0L), "test1",
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
	shared_ptr<GlobalVariable>v1 =  global_variable_registry->createGlobalVariable( new VariableProperties(new Datum(0L), "test1",
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



