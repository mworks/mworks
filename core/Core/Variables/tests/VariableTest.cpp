/*
 *  VariableTest.cpp
 *  MWorksCore
 *
 *  Created by labuser on 10/2/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#include "VariableTest.h"
#include "VariableRegistry.h"
#include "TrialBuildingBlocks.h"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( VariableTestFixture, "Unit Test" );


void VariableTestFixture::testSimpleConstant() {
 Datum seven_point_six_two(M_FLOAT, 7.62);
	ConstantVariable c(seven_point_six_two);
	
	CPPUNIT_ASSERT(seven_point_six_two == c.getValue());	
}


void VariableTestFixture::testSimpleGlobal() {
	shared_ptr<Variable>v =  global_variable_registry->createGlobalVariable( VariableProperties(
																										   Datum(42L), "test1",
																										   "Test",
																										   "Test",
																										   M_NEVER, M_WHEN_CHANGED,
																										   true, false,
																										   M_CONTINUOUS_INFINITE,""));	
	CPPUNIT_ASSERT((long)*v == 42);
	CPPUNIT_ASSERT(v->getValue() == Datum(M_INTEGER, 42));
	CPPUNIT_ASSERT(v->getVariableName() == "test1");
	CPPUNIT_ASSERT(v->getLogging() == M_WHEN_CHANGED);
	
	v->setLogging(M_NEVER);
	CPPUNIT_ASSERT(v->getLogging() == M_NEVER);
	
	v->setValue(Datum(M_INTEGER, 666));
	CPPUNIT_ASSERT((long)*v == 666);
	
	v->setValue(2112L);
	CPPUNIT_ASSERT((long)*v == 2112);
	
}


END_NAMESPACE_MW





















