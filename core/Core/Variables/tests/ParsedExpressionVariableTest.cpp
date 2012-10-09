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
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("0#AND2")));
    CPPUNIT_ASSERT_THROW(getExpressionValue("1 #and 2"), FatalParserException);
    CPPUNIT_ASSERT_EQUAL(std::string("1 #AND 2"), std::string(getExpressionValue("\"1 #AND 2\"")));
    
    // #OR
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("0 #OR 1")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("0 #OR 0")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("0#OR0")));
    CPPUNIT_ASSERT_THROW(getExpressionValue("0 #or 1"), FatalParserException);
    CPPUNIT_ASSERT_EQUAL(std::string("0 #OR 1"), std::string(getExpressionValue("\"0 #OR 1\"")));
}


void ParsedExpressionVariableTestFixture::testAlternativeComparisonOperators() {
    // #GT
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("2 #GT 1")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("2 #GT 2")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("1 #GT 2")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("1#GT2")));
    CPPUNIT_ASSERT_THROW(getExpressionValue("2 #gt 1"), FatalParserException);
    CPPUNIT_ASSERT_EQUAL(std::string("2 #GT 1"), std::string(getExpressionValue("\"2 #GT 1\"")));
    
    // #LT
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("2 #LT 1")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("2 #LT 2")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1 #LT 2")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1#LT2")));
    CPPUNIT_ASSERT_THROW(getExpressionValue("2 #lt 1"), FatalParserException);
    CPPUNIT_ASSERT_EQUAL(std::string("2 #LT 1"), std::string(getExpressionValue("\"2 #LT 1\"")));
    
    // #GE
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("2 #GE 1")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("2 #GE 2")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("1 #GE 2")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("1#GE2")));
    CPPUNIT_ASSERT_THROW(getExpressionValue("2 #ge 1"), FatalParserException);
    CPPUNIT_ASSERT_EQUAL(std::string("2 #GE 1"), std::string(getExpressionValue("\"2 #GE 1\"")));
    
    // #LE
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("2 #LE 1")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("2 #LE 2")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1 #LE 2")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1#LE2")));
    CPPUNIT_ASSERT_THROW(getExpressionValue("2 #le 1"), FatalParserException);
    CPPUNIT_ASSERT_EQUAL(std::string("2 #LE 1"), std::string(getExpressionValue("\"2 #LE 1\"")));
}


void ParsedExpressionVariableTestFixture::testBooleanConstants() {
    // true
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("(true)")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("(TRUE)")));
    CPPUNIT_ASSERT_EQUAL(std::string("true"), std::string(getExpressionValue("\"true\"")));
    
    // false
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("(false)")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("(FALSE)")));
    CPPUNIT_ASSERT_EQUAL(std::string("false"), std::string(getExpressionValue("\"false\"")));
    
    // YES
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("(YES)")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("(yes)")));
    CPPUNIT_ASSERT_EQUAL(std::string("YES"), std::string(getExpressionValue("\"YES\"")));
    
    // NO
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("(NO)")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("(no)")));
    CPPUNIT_ASSERT_EQUAL(std::string("NO"), std::string(getExpressionValue("\"NO\"")));
}


void ParsedExpressionVariableTestFixture::testTimeUnits() {
    const double delta = 1e-12;
    createGlobalVariable("x", Datum(3L));
    
    // us
    CPPUNIT_ASSERT_EQUAL(2L, long(getExpressionValue("2us")));
    CPPUNIT_ASSERT_EQUAL(2L, long(getExpressionValue("2 us")));
    CPPUNIT_ASSERT_EQUAL(2L, long(getExpressionValue("2US")));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-7, double(getExpressionValue("1e-7us")), delta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-7, double(getExpressionValue("1e-7 us")), delta);
    CPPUNIT_ASSERT_EQUAL(3L, long(getExpressionValue("1 + 2 us")));
    CPPUNIT_ASSERT_EQUAL(3L, long(getExpressionValue("2 us + 1")));
    CPPUNIT_ASSERT_EQUAL(3L, long(getExpressionValue("x us")));
    CPPUNIT_ASSERT_EQUAL(std::string("2us"), std::string(getExpressionValue("\"2us\"")));
    
    // ms
    CPPUNIT_ASSERT_EQUAL(2000L, long(getExpressionValue("2ms")));
    CPPUNIT_ASSERT_EQUAL(2000L, long(getExpressionValue("2 ms")));
    CPPUNIT_ASSERT_EQUAL(2000L, long(getExpressionValue("2MS")));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-4, double(getExpressionValue("1e-7ms")), delta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-4, double(getExpressionValue("1e-7 ms")), delta);
    CPPUNIT_ASSERT_EQUAL(2001L, long(getExpressionValue("1 + 2 ms")));
    CPPUNIT_ASSERT_EQUAL(2001L, long(getExpressionValue("2 ms + 1")));
    CPPUNIT_ASSERT_EQUAL(3000L, long(getExpressionValue("x ms")));
    CPPUNIT_ASSERT_EQUAL(std::string("2ms"), std::string(getExpressionValue("\"2ms\"")));
    
    // s
    CPPUNIT_ASSERT_EQUAL(2000000L, long(getExpressionValue("2s")));
    CPPUNIT_ASSERT_EQUAL(2000000L, long(getExpressionValue("2 s")));
    CPPUNIT_ASSERT_EQUAL(2000000L, long(getExpressionValue("2S")));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-1, double(getExpressionValue("1e-7s")), delta);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1e-1, double(getExpressionValue("1e-7 s")), delta);
    CPPUNIT_ASSERT_EQUAL(2000001L, long(getExpressionValue("1 + 2 s")));
    CPPUNIT_ASSERT_EQUAL(2000001L, long(getExpressionValue("2 s + 1")));
    CPPUNIT_ASSERT_EQUAL(3000000L, long(getExpressionValue("x s")));
    CPPUNIT_ASSERT_EQUAL(std::string("2s"), std::string(getExpressionValue("\"2s\"")));
}


void ParsedExpressionVariableTestFixture::testBinaryLogicOperatorsWithVariableOperand() {
    createGlobalVariable("x", Datum(1L));
    createGlobalVariable("y", Datum(0L));
    
    //
    // and
    //
    
    // Constant (expression value does not depend on variable value)
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1 && 2")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("x && 0")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("0 && x")));
    
    // Non-constant (expression value depends on variable value)
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("x && 1")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1 && x")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("x && y")));
    
    //
    // or
    //
    
    // Constant (expression value does not depend on variable value)
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1 || 2")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("y || 1")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("1 || y")));
    
    // Non-constant (expression value depends on variable value)
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("y || 0")));
    CPPUNIT_ASSERT_EQUAL(false, bool(getExpressionValue("0 || y")));
    CPPUNIT_ASSERT_EQUAL(true, bool(getExpressionValue("y || x")));
}


void ParsedExpressionVariableTestFixture::testVariableSubscript() {
    // Unknown variable
    CPPUNIT_ASSERT_THROW(getExpressionValue("(x)"), SimpleException);
    CPPUNIT_ASSERT_THROW(getExpressionValue("(x[0])"), SimpleException);
    
    // Scalar variable
    createGlobalVariable("x", Datum(1L));
    CPPUNIT_ASSERT_EQUAL(1L, long(getExpressionValue("(x)")));
    CPPUNIT_ASSERT_THROW(getExpressionValue("(x[0])"), SimpleException);
}


void ParsedExpressionVariableTestFixture::testRangeExpression() {
    typedef std::vector<stx::AnyScalar> valueList;
    valueList values;
    
    // Can't evaluate range expr as a scalar
    {
        stx::ParseTreeList trees = ParsedExpressionVariable::parseExpressionList("1:3");
        CPPUNIT_ASSERT_EQUAL(1, int(trees.size()));
        CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateParseTree(trees[0]), FatalParserException);
    }
    
    // Start and stop must be signed integers
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpressionList("1.1:3", values), FatalParserException);
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpressionList("1:3.3", values), FatalParserException);
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpressionList("(word)1:3", values), FatalParserException);
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpressionList("1:(word)3", values), FatalParserException);
    
    // start < stop
    ParsedExpressionVariable::evaluateExpressionList("1:3", values);
    CPPUNIT_ASSERT_EQUAL(3, int(values.size()));
    for (valueList::size_type i = 0; i < values.size(); i++) {
        CPPUNIT_ASSERT( values[i].isIntegerType() );
        CPPUNIT_ASSERT_EQUAL((long long)(i+1), values[i].getLong());
    }
    
    // start > stop
    values.clear();
    ParsedExpressionVariable::evaluateExpressionList("5:-2", values);
    CPPUNIT_ASSERT_EQUAL(8, int(values.size()));
    for (valueList::size_type i = 0; i < values.size(); i++) {
        CPPUNIT_ASSERT( values[i].isIntegerType() );
        CPPUNIT_ASSERT_EQUAL((long long)(5) - (long long)(i), values[i].getLong());
    }
    
    // start == stop
    values.clear();
    ParsedExpressionVariable::evaluateExpressionList("2:2", values);
    CPPUNIT_ASSERT_EQUAL(1, int(values.size()));
    CPPUNIT_ASSERT( values[0].isIntegerType() );
    CPPUNIT_ASSERT_EQUAL((long long)(2), values[0].getLong());
    
    // Mixed expression list
    values.clear();
    ParsedExpressionVariable::evaluateExpressionList("1,2,3:5,6,7:10", values);
    CPPUNIT_ASSERT_EQUAL(10, int(values.size()));
    for (valueList::size_type i = 0; i < values.size(); i++) {
        CPPUNIT_ASSERT( values[i].isIntegerType() );
        CPPUNIT_ASSERT_EQUAL((long long)(i+1), values[i].getLong());
    }
    
    // Single-value list (not a range expr, but we should verify that it still works)
    values.clear();
    ParsedExpressionVariable::evaluateExpressionList("2", values);
    CPPUNIT_ASSERT_EQUAL(1, int(values.size()));
    CPPUNIT_ASSERT( values[0].isIntegerType() );
    CPPUNIT_ASSERT_EQUAL((long long)(2), values[0].getLong());
    
    // More complex expressions for start and stop
    createGlobalVariable("x", Datum(1L));
    values.clear();
    ParsedExpressionVariable::evaluateExpressionList("x:x+2", values);
    CPPUNIT_ASSERT_EQUAL(3, int(values.size()));
    for (valueList::size_type i = 0; i < values.size(); i++) {
        CPPUNIT_ASSERT( values[i].isIntegerType() );
        CPPUNIT_ASSERT_EQUAL((long long)(i+1), values[i].getLong());
    }
}


END_NAMESPACE_MW

























