/*
 *  ParsedExpressionVariableTest.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 1/1/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "ParsedExpressionVariableTest.h"

#include "VariableRegistry.h"
#include "TrialBuildingBlocks.h"
#include "EventBuffer.h"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ParsedExpressionVariableTestFixture, "Unit Test" );


void ParsedExpressionVariableTestFixture::createGlobalVariable(const std::string &name, Datum value) {
    VariableProperties props(&value, 
                             name,
                             name,
                             name,
                             M_NEVER, 
                             M_WHEN_CHANGED,
                             true, 
                             false,
                             M_INTEGER_INFINITE,
                             "");
    
    global_variable_registry->createGlobalVariable(&props);
}


Datum ParsedExpressionVariableTestFixture::getExpressionValue(const std::string &expr) {
    return ParsedExpressionVariable(expr).getValue();
}


void ParsedExpressionVariableTestFixture::testSimpleExpression() {
	
	shared_ptr<VariableRegistry> r(new VariableRegistry(global_outgoing_event_buffer));
	global_variable_registry = r;
	
 Datum testInt((float)1);
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
 Datum data = e->getValue();
	
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
	} catch (FatalParserException &e) {
		threw_exception = true;
	}
	
	CPPUNIT_ASSERT( threw_exception );
	
	threw_exception = false;
	try {
		e = shared_ptr<ParsedExpressionVariable>(new ParsedExpressionVariable("other_var + 1"));
	} catch (SimpleException &e) {
		threw_exception = true;
	}
	
	CPPUNIT_ASSERT( threw_exception );
	
}


void ParsedExpressionVariableTestFixture::testAlternativeLogicalOperators() {
    createGlobalVariable("x", Datum(0L));
    
    // not
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("not 0")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("NOT 1")));
    createGlobalVariable("notx", Datum(17L));
    CPPUNIT_ASSERT_EQUAL(18L, long(getExpressionValue("(notx + 1)")));
    
    // and
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1 and 2")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("1 AND 0")));
    createGlobalVariable("andx", Datum(5L));
    CPPUNIT_ASSERT_THROW(getExpressionValue("(1 andx)"), FatalParserException);
    
    // or
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1 or 0")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("0 OR 0")));
    createGlobalVariable("orx", Datum(6L));
    CPPUNIT_ASSERT_THROW(getExpressionValue("(0 orx)"), FatalParserException);
    
    // #AND
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1 #AND 2")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("0 #AND 2")));
    CPPUNIT_ASSERT_THROW(getExpressionValue("1 #and 2"), FatalParserException);
    CPPUNIT_ASSERT_EQUAL(std::string("1 #AND 2"), std::string(getExpressionValue("\"1 #AND 2\"")));
    
    // #OR
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("0 #OR 1")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("0 #OR 0")));
    CPPUNIT_ASSERT_THROW(getExpressionValue("0 #or 1"), FatalParserException);
    CPPUNIT_ASSERT_EQUAL(std::string("0 #OR 1"), std::string(getExpressionValue("\"0 #OR 1\"")));
}


END_NAMESPACE_MW

























