/*
 *  VariableTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by labuser on 10/2/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#include "VariableTest.h"
#include "VariableRegistry.h"
#include "TrialBuildingBlocks.h"
using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( VariableTestFixture, "Unit Test" );

void VariableTestFixture::testSimpleConstant() {
	Data seven_point_six_two(M_FLOAT, 7.62);
	ConstantVariable c(seven_point_six_two);
	
	CPPUNIT_ASSERT(seven_point_six_two == c.getValue());	
}


void VariableTestFixture::testSimpleGlobal() {
	shared_ptr<GlobalVariable>v =  GlobalVariableRegistry->createGlobalVariable( new VariableProperties(
																										   new Data(42L), "test1",
																										   "Test",
																										   "Test",
																										   M_NEVER, M_WHEN_CHANGED,
																										   true, false,
																										   M_CONTINUOUS_INFINITE,""));	
	CPPUNIT_ASSERT((long)*v == 42);
	CPPUNIT_ASSERT(v->getValue() == Data(M_INTEGER, 42));
	CPPUNIT_ASSERT(v->getVariableName() == "test1");
	CPPUNIT_ASSERT(v->getLogging() == M_WHEN_CHANGED);
	
	v->setLogging(M_NEVER);
	CPPUNIT_ASSERT(v->getLogging() == M_NEVER);
	
	v->setValue(Data(M_INTEGER, 666));
	CPPUNIT_ASSERT((long)*v == 666);
	
	v->setValue(2112L);
	CPPUNIT_ASSERT((long)*v == 2112);
	
}

void VariableTestFixture::testSimpleExpression() {
	Data six(M_INTEGER, 6);
	ConstantVariable c1(six);
	Data five(M_INTEGER, 5);
	ConstantVariable c2(five);
	
	// boolean expressions
	shared_ptr<ExpressionVariable> e;
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_IS_EQUAL));
	CPPUNIT_ASSERT(!e->getValue().getBool());

	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c2, &c2, M_IS_EQUAL));
	CPPUNIT_ASSERT(e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_IS_NOT_EQUAL));
	CPPUNIT_ASSERT(e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c1, M_IS_NOT_EQUAL));
	CPPUNIT_ASSERT(!e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_IS_GREATER_THAN));
	CPPUNIT_ASSERT(e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c2, &c1, M_IS_GREATER_THAN));
	CPPUNIT_ASSERT(!e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c1, M_IS_GREATER_THAN));
	CPPUNIT_ASSERT(!e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_IS_GREATER_THAN_OR_EQUAL));
	CPPUNIT_ASSERT(e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c2, &c1, M_IS_GREATER_THAN_OR_EQUAL));
	CPPUNIT_ASSERT(!e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c1, M_IS_GREATER_THAN_OR_EQUAL));
	CPPUNIT_ASSERT(e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_IS_LESS_THAN));
	CPPUNIT_ASSERT(!e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c2, &c1, M_IS_LESS_THAN));
	CPPUNIT_ASSERT(e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c1, M_IS_LESS_THAN));
	CPPUNIT_ASSERT(!e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_IS_LESS_THAN_OR_EQUAL));
	CPPUNIT_ASSERT(!e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c2, &c1, M_IS_LESS_THAN_OR_EQUAL));
	CPPUNIT_ASSERT(e->getValue().getBool());
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c1, M_IS_LESS_THAN_OR_EQUAL));
	CPPUNIT_ASSERT(e->getValue().getBool());
	
	// arithmatic expressions
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_PLUS));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six + five));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_MINUS));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six - five));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_TIMES));
	CPPUNIT_ASSERT(e->getValue().getFloat() == (float)(six * five));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_DIVIDE));
	CPPUNIT_ASSERT(e->getValue().getFloat() == (float)(six / five));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_MOD));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six % five));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_INCREMENT));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six + Data(M_FLOAT,1)));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_DECREMENT));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six - Data(M_FLOAT,1)));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, NULL, M_INCREMENT));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six + Data(M_FLOAT,1)));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, NULL, M_DECREMENT));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six - Data(M_FLOAT,1)));
	
	Data seven(M_INTEGER, 0x7);
	ConstantVariable c3(seven);
	Data eight(M_INTEGER, 0x8);
	ConstantVariable c4(eight);
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c3, &c4, M_AND));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(seven && eight));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c3, &c4, M_OR));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(seven || eight));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c3, &c4, M_NOT));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(!seven));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c3, NULL, M_NOT));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(!seven));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_UNARY_MINUS));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six)*-1);
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, NULL, M_UNARY_MINUS));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six)*-1);
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, &c2, M_UNARY_PLUS));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six));
	
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(&c1, NULL, M_UNARY_PLUS));
	CPPUNIT_ASSERT(e->getValue().getInteger() == (long)(six));
}

void VariableTestFixture::testSimpleClone() {
	
	// constant clone
	Data seven_point_six_two(M_FLOAT, 7.62);
	ConstantVariable c(seven_point_six_two);
	
	shared_ptr<Variable> cClone(c.clone());
	CPPUNIT_ASSERT(cClone->getValue() == c.getValue());
	shared_ptr<Variable> cFrozenClone(c.frozenClone());
	CPPUNIT_ASSERT(cFrozenClone->getValue() == c.getValue());

	// global variable clone
	shared_ptr<GlobalVariable>v =  GlobalVariableRegistry->createGlobalVariable( new VariableProperties(new Data(42L), "test1",
																										  "Test",
																										  "Test",
																										  M_NEVER, M_WHEN_CHANGED,
																										  true, false,
																										  M_CONTINUOUS_INFINITE,""));	
	
	
	shared_ptr<Variable> vClone(v->clone());
	CPPUNIT_ASSERT(vClone->getValue() == v->getValue());
	shared_ptr<Variable> vFrozenClone(v->frozenClone());
	CPPUNIT_ASSERT(vFrozenClone->getValue() == v->getValue());
	
	
	shared_ptr<GlobalVariable>v2 =  GlobalVariableRegistry->createGlobalVariable( new VariableProperties(new Data(42L), "test2",
																										  "Test",
																										  "Test",
																										  M_NEVER, M_WHEN_CHANGED,
																										  true, false,
																										  M_CONTINUOUS_INFINITE,""));	
	
	
	shared_ptr<Variable> v2Clone(v2->clone());
	shared_ptr<Variable> v2Clone2(v2->frozenClone());
	CPPUNIT_ASSERT(v2Clone->getValue() == v2->getValue());
	CPPUNIT_ASSERT(v2Clone2->getValue() == v2->getValue());
	v2->setValue(2112L);
	CPPUNIT_ASSERT(v2Clone->getValue() == v2->getValue());
	CPPUNIT_ASSERT(v2Clone2->getValue() != v2->getValue());
}

void VariableTestFixture::testExpressionClone() {
	shared_ptr<GlobalVariable>v =  GlobalVariableRegistry->createGlobalVariable( new VariableProperties(new Data(42L), "test1",
																										  "Test",
																										  "Test",
																										  M_NEVER, M_WHEN_CHANGED,
																										  true, false,
																										  M_CONTINUOUS_INFINITE,""));	
	shared_ptr<Variable> vClone(v->clone());
	CPPUNIT_ASSERT(vClone->getValue() == v->getValue());
	shared_ptr<Variable> vFrozenClone(v->frozenClone());
	CPPUNIT_ASSERT(vFrozenClone->getValue() == v->getValue());
	
	shared_ptr<ExpressionVariable> e;
	e = shared_ptr<ExpressionVariable>(new ExpressionVariable(vFrozenClone.get(), vClone.get(), M_IS_EQUAL));
	CPPUNIT_ASSERT(e->getValue().getBool());
	
	v->setValue(2112L);
	CPPUNIT_ASSERT(!e->getValue().getBool());
	
	v->setValue(42L);
	CPPUNIT_ASSERT(e->getValue().getBool());
	
	
	
}

























