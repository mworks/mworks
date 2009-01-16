/*
 *  ParsedExpressionVariableTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 1/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "ParsedExpressionVariableTest.h"

#include "VariableRegistry.h"
#include "TrialBuildingBlocks.h"
#include "EventBuffer.h"
using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ParsedExpressionVariableTestFixture, "Unit Test" );

void ParsedExpressionVariableTestFixture::testSimpleExpression() {
	
	shared_ptr<VariableRegistry> r(new VariableRegistry(GlobalBufferManager));
	GlobalVariableRegistry = r;
	
	Data testInt((float)1);
	VariableProperties props(&testInt, 
							 "test",
							 "Test test",
							 "Testy test test test",
							 M_NEVER, 
							 M_WHEN_CHANGED,
							 true, 
							 false,
							 M_CONTINUOUS_INFINITE,
							 "");
	
	shared_ptr<Variable> testvar =
	r->createGlobalVariable(&props);
	
	
	shared_ptr<ParsedExpressionVariable> e(new ParsedExpressionVariable("1 + 1"));
	Data data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 2 );

	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("20 % 2"));
	data = e->getValue(); 
	
	CPPUNIT_ASSERT( (long)data == 0 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("test + 1"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 2 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("4-2"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 2 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("20/10"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 2 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("20*3"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 60 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("(test + 1)%2"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 0 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("test%20"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 1 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("sqrt(test+8)"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 3 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("(0 #OR 1)"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 1 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("(0 #AND 1)"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 0 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("(0 #GT 1)"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 0 );

	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("!1"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data == 0 );
	
	e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("!0"));
	data = e->getValue();
	
	CPPUNIT_ASSERT( (long)data != 0 );
	
	bool threw_exception = false;
	try {
		e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("(0 #GT 1))"));
	} catch (std::exception &e) {
		threw_exception = true;
	}
	
	CPPUNIT_ASSERT( threw_exception );
	
	threw_exception = false;
	try {
		e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("other_var + 1"));
	} catch (std::exception &e) {
		threw_exception = true;
	}
	
	CPPUNIT_ASSERT( threw_exception );
	
}

























