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


void ParsedExpressionVariableTestFixture::testSubscriptExpression() {
    // Non-subscriptable variable
    {
        createGlobalVariable("x", Datum(1L));
        
        CPPUNIT_ASSERT_EQUAL(1L, long(getExpressionValue("(x)")));
        
        Datum value = getExpressionValue("(x[0])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL(0L, long(value));
    }
    
    // List variable
    {
        Datum value(M_LIST, 3);
        value.setElement(0, 2L);
        value.setElement(1, 3.0);
        value.setElement(2, "four");
        createGlobalVariable("y", value);
        
        value = getExpressionValue("(y[0])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL(2L, long(value));
        
        value = getExpressionValue("(y[1])");
        CPPUNIT_ASSERT( value.isFloat() );
        CPPUNIT_ASSERT_EQUAL(3.0, double(value));
        
        value = getExpressionValue("(y[2])");
        CPPUNIT_ASSERT( value.isString() );
        CPPUNIT_ASSERT_EQUAL(std::string("four"), std::string(value));
        
        // Out-of-bounds index
        value = getExpressionValue("(y[3])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL(0L, long(value));
        
        // Negative index
        value = getExpressionValue("(y[-1])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL(0L, long(value));
    }
    
    // Dictionary variable
    {
        Datum value(M_DICTIONARY, 2);
        value.addElement("foo", 1);
        value.addElement("bar", 2.5);
        createGlobalVariable("z", value);
        
        value = getExpressionValue("(z['foo'])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 1LL, value.getInteger() );
        
        value = getExpressionValue("(z['bar'])");
        CPPUNIT_ASSERT( value.isFloat() );
        CPPUNIT_ASSERT_EQUAL( 2.5, value.getFloat() );
        
        // Non-existent key
        value = getExpressionValue("(z['blah'])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL(0L, long(value));
    }
    
    // Subscripts on literals
    {
        Datum value = getExpressionValue("([1, 2, 3][1])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 2LL, value.getInteger() );
        
        value = getExpressionValue("([1, 2, 3][3])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 0LL, value.getInteger() );
        
        value = getExpressionValue("({'a': 1, 'b': 2}['b'])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 2LL, value.getInteger() );
        
        value = getExpressionValue("({'a': 1, 'b': 2}['c'])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 0LL, value.getInteger() );
    }
    
    // Chained subscripts
    {
        Datum value = getExpressionValue("([1, 2, [3, 4, [5, 6]]][2][2][1])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 6LL, value.getInteger() );
        
        value = getExpressionValue("({'a': 1, 'b': {'c': 2, 'd': {'e': 3, 'f': 4}}}['b']['d']['f'])");
        CPPUNIT_ASSERT( value.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 4LL, value.getInteger() );
    }
}


void ParsedExpressionVariableTestFixture::testRangeExpression() {
    typedef std::vector<Datum> valueList;
    valueList values;
    
    // Can't evaluate range expr as a scalar
    {
        stx::ParseTreeList trees = ParsedExpressionVariable::parseExpressionList("1:3");
        CPPUNIT_ASSERT_EQUAL(1, int(trees.size()));
        CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateParseTree(trees[0]), FatalParserException);
    }
    
    // Start and stop must be integers
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpressionList("1.1:3", values), FatalParserException);
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpressionList("1:3.3", values), FatalParserException);
    
    // start < stop
    ParsedExpressionVariable::evaluateExpressionList("1:3", values);
    CPPUNIT_ASSERT_EQUAL(3, int(values.size()));
    for (valueList::size_type i = 0; i < values.size(); i++) {
        CPPUNIT_ASSERT( values[i].isInteger() );
        CPPUNIT_ASSERT_EQUAL((long long)(i+1), values[i].getInteger());
    }
    
    // start > stop
    values.clear();
    ParsedExpressionVariable::evaluateExpressionList("5:-2", values);
    CPPUNIT_ASSERT_EQUAL(8, int(values.size()));
    for (valueList::size_type i = 0; i < values.size(); i++) {
        CPPUNIT_ASSERT( values[i].isInteger() );
        CPPUNIT_ASSERT_EQUAL((long long)(5) - (long long)(i), values[i].getInteger());
    }
    
    // start == stop
    values.clear();
    ParsedExpressionVariable::evaluateExpressionList("2:2", values);
    CPPUNIT_ASSERT_EQUAL(1, int(values.size()));
    CPPUNIT_ASSERT( values[0].isInteger() );
    CPPUNIT_ASSERT_EQUAL((long long)(2), values[0].getInteger());
    
    // Mixed expression list
    values.clear();
    ParsedExpressionVariable::evaluateExpressionList("1,2,3:5,6,7:10", values);
    CPPUNIT_ASSERT_EQUAL(10, int(values.size()));
    for (valueList::size_type i = 0; i < values.size(); i++) {
        CPPUNIT_ASSERT( values[i].isInteger() );
        CPPUNIT_ASSERT_EQUAL((long long)(i+1), values[i].getInteger());
    }
    
    // Single-value list (not a range expr, but we should verify that it still works)
    values.clear();
    ParsedExpressionVariable::evaluateExpressionList("2", values);
    CPPUNIT_ASSERT_EQUAL(1, int(values.size()));
    CPPUNIT_ASSERT( values[0].isInteger() );
    CPPUNIT_ASSERT_EQUAL((long long)(2), values[0].getInteger());
    
    // More complex expressions for start and stop
    createGlobalVariable("x", Datum(1L));
    values.clear();
    ParsedExpressionVariable::evaluateExpressionList("x:x+2", values);
    CPPUNIT_ASSERT_EQUAL(3, int(values.size()));
    for (valueList::size_type i = 0; i < values.size(); i++) {
        CPPUNIT_ASSERT( values[i].isInteger() );
        CPPUNIT_ASSERT_EQUAL((long long)(i+1), values[i].getInteger());
    }
}


void ParsedExpressionVariableTestFixture::testListLiteral() {
    // Empty list
    {
        Datum d = ParsedExpressionVariable::evaluateExpression("[]");
        CPPUNIT_ASSERT( d.isList() );
        CPPUNIT_ASSERT_EQUAL( 0, d.getNElements() );
    }
    
    // Single-element list
    {
        Datum d = ParsedExpressionVariable::evaluateExpression("[1.5]");
        CPPUNIT_ASSERT( d.isList() );
        CPPUNIT_ASSERT_EQUAL( 1, d.getNElements() );
        
        Datum item = d.getElement(0);
        CPPUNIT_ASSERT( item.isFloat() );
        CPPUNIT_ASSERT_EQUAL( 1.5, item.getFloat() );
    }
    
    // Multiple-element list
    {
        Datum d = ParsedExpressionVariable::evaluateExpression("[1, 2.5, 'foo']");
        CPPUNIT_ASSERT( d.isList() );
        CPPUNIT_ASSERT_EQUAL( 3, d.getNElements() );
        
        {
            Datum item = d.getElement(0);
            CPPUNIT_ASSERT( item.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 1LL, item.getInteger() );
        }
        
        {
            Datum item = d.getElement(1);
            CPPUNIT_ASSERT( item.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 2.5, item.getFloat() );
        }
        
        {
            Datum item = d.getElement(2);
            CPPUNIT_ASSERT( item.isString() );
            CPPUNIT_ASSERT_EQUAL( std::string("foo"), std::string(item) );
        }
    }
    
    // Range expressions
    {
        Datum d = ParsedExpressionVariable::evaluateExpression("[1,2,3:5,6,7:10]");
        CPPUNIT_ASSERT( d.isList() );
        CPPUNIT_ASSERT_EQUAL( 10, d.getNElements() );
        
        for (int i = 0; i < 10; i++) {
            Datum item = d.getElement(i);
            CPPUNIT_ASSERT( item.isInteger() );
            CPPUNIT_ASSERT_EQUAL( (long long)(i+1), item.getInteger() );
        }
    }
    
    // Missing closing bracket
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpression("[1,2,3"), FatalParserException);
    
    // Extra comma
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpression("[1,2,3,]"), FatalParserException);
}


void ParsedExpressionVariableTestFixture::testDictionaryLiteral() {
    // Empty dict
    {
        Datum d = ParsedExpressionVariable::evaluateExpression("{}");
        CPPUNIT_ASSERT( d.isDictionary() );
        CPPUNIT_ASSERT_EQUAL( 0, d.getNElements() );
    }
    
    // Single-element dict
    {
        Datum d = ParsedExpressionVariable::evaluateExpression("{'foo': 1.5}");
        CPPUNIT_ASSERT( d.isDictionary() );
        CPPUNIT_ASSERT_EQUAL( 1, d.getNElements() );
        
        Datum value = d.getElement("foo");
        CPPUNIT_ASSERT( value.isFloat() );
        CPPUNIT_ASSERT_EQUAL( 1.5, value.getFloat() );
    }
    
    // Multiple-element dict
    {
        Datum d = ParsedExpressionVariable::evaluateExpression("{'a': 1, true: 2.5, 3.5: 'foo'}");
        CPPUNIT_ASSERT( d.isDictionary() );
        CPPUNIT_ASSERT_EQUAL( 3, d.getNElements() );
        
        {
            Datum value = d.getElement("a");
            CPPUNIT_ASSERT( value.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 1LL, value.getInteger() );
        }
        
        {
            Datum value = d.getElement(Datum(true));
            CPPUNIT_ASSERT( value.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 2.5, value.getFloat() );
        }
        
        {
            Datum value = d.getElement(Datum(3.5));
            CPPUNIT_ASSERT( value.isString() );
            CPPUNIT_ASSERT_EQUAL( std::string("foo"), std::string(value) );
        }
    }
    
    // Missing closing brace
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpression("{'a':1, 'b': 2"), FatalParserException);
    
    // Missing colon
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpression("{'a':1, 'b' 2}"), FatalParserException);
    
    // Extra comma
    CPPUNIT_ASSERT_THROW(ParsedExpressionVariable::evaluateExpression("{'a':1, 'b': 2,}"), FatalParserException);
}


void ParsedExpressionVariableTestFixture::testIntegerOverflow() {
    CPPUNIT_ASSERT_EQUAL(LLONG_MAX - 1,
                         ParsedExpressionVariable::evaluateExpression("9223372036854775806").getInteger());
    CPPUNIT_ASSERT_EQUAL(LLONG_MAX,
                         ParsedExpressionVariable::evaluateExpression("9223372036854775807").getInteger());
    CPPUNIT_ASSERT_EQUAL(LLONG_MAX,
                         ParsedExpressionVariable::evaluateExpression("9223372036854775808").getInteger());
}


void ParsedExpressionVariableTestFixture::testFloatOverflow() {
    CPPUNIT_ASSERT_EQUAL(std::nextafter(DBL_MAX, 0.0),
                         ParsedExpressionVariable::evaluateExpression("1.7976931348623156e+308").getFloat());
    CPPUNIT_ASSERT_EQUAL(DBL_MAX,
                         ParsedExpressionVariable::evaluateExpression("1.7976931348623157e+308").getFloat());
    CPPUNIT_ASSERT_EQUAL(DBL_MAX,
                         ParsedExpressionVariable::evaluateExpression("1.7976931348623158e+308").getFloat());
    CPPUNIT_ASSERT_EQUAL(HUGE_VAL,
                         ParsedExpressionVariable::evaluateExpression("1.7976931348623159e+308").getFloat());
}


void ParsedExpressionVariableTestFixture::testExpRandFunction() {
    auto tryExpr = [](const std::string &expr) {
        std::string msg("No exception thrown");
        try {
            ParsedExpressionVariable::evaluateExpression(expr);
        } catch (FatalParserException &e) {
            msg = e.getMessage();
        }
        return msg;
    };
    
    // Too few parameters
    CPPUNIT_ASSERT_EQUAL(std::string("Expression parser error: Function EXP_RAND() requires 1 to 3 parameters"),
                         tryExpr("exp_rand()"));
    
    // Too many parameters
    CPPUNIT_ASSERT_EQUAL(std::string("Expression parser error: Function EXP_RAND() requires 1 to 3 parameters"),
                         tryExpr("exp_rand(1,1,1,1)"));
    
    // Bad beta
    CPPUNIT_ASSERT_EQUAL(std::string("Expression parser error: First parameter to function EXP_RAND() must be a positive number"),
                         tryExpr("exp_rand(-1)"));
    CPPUNIT_ASSERT_EQUAL(std::string("Expression parser error: First parameter to function EXP_RAND() must be a positive number"),
                         tryExpr("exp_rand(0)"));
    
    // Bad min
    CPPUNIT_ASSERT_EQUAL(std::string("Expression parser error: Second parameter to function EXP_RAND() must be a non-negative number"),
                         tryExpr("exp_rand(1, -1)"));
    
    // Bad max
    CPPUNIT_ASSERT_EQUAL(std::string("Expression parser error: Third parameter to function EXP_RAND() must be greater than second parameter"),
                         tryExpr("exp_rand(1, 1, 1)"));
    CPPUNIT_ASSERT_EQUAL(std::string("Expression parser error: Third parameter to function EXP_RAND() must be greater than second parameter"),
                         tryExpr("exp_rand(1, 1, 0)"));
    
    // One parameter
    for (int i = 0; i < 10; i++) {
        Datum result = ParsedExpressionVariable::evaluateExpression("exp_rand(1)");
        CPPUNIT_ASSERT(result.isFloat());
        double value = result.getFloat();
        CPPUNIT_ASSERT(value >= 0.0);
    }
    
    // Two parameters
    for (int i = 0; i < 10; i++) {
        Datum result = ParsedExpressionVariable::evaluateExpression("exp_rand(2, 5)");
        CPPUNIT_ASSERT(result.isFloat());
        double value = result.getFloat();
        CPPUNIT_ASSERT(value >= 5.0);
    }
    
    // Three parameters
    for (int i = 0; i < 10; i++) {
        Datum result = ParsedExpressionVariable::evaluateExpression("exp_rand(2, 5, 10)");
        CPPUNIT_ASSERT(result.isFloat());
        double value = result.getFloat();
        CPPUNIT_ASSERT(value >= 5.0);
        CPPUNIT_ASSERT(value <= 10.0);
    }
    
    // Bad parameters
    for (int i = 0; i < 10; i++) {
        Datum result = ParsedExpressionVariable::evaluateExpression("exp_rand(2, 2.5 - 1e-7, 2.5)");
        CPPUNIT_ASSERT(result.isFloat());
        CPPUNIT_ASSERT_EQUAL(2.5, result.getFloat());
    }
}


END_NAMESPACE_MW

























