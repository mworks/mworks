/*
 *  ParsedExpressionVariableTest.h
 *  MWorksCore
 *
 *  Created by labuser on 10/2/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#ifndef _PARSED_EXPRESSION_VARIABLE_TEST_H_
#define _PARSED_EXPRESSION_VARIABLE_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "FullCoreEnvironmentTest.h"


BEGIN_NAMESPACE_MW


class ParsedExpressionVariableTestFixture : public FullCoreEnvironmentTestFixture {
	
	
	CPPUNIT_TEST_SUITE( ParsedExpressionVariableTestFixture );
	CPPUNIT_TEST( testSimpleExpression );
	CPPUNIT_TEST( testAlternativeLogicalOperators );
	CPPUNIT_TEST( testAlternativeComparisonOperators );
	CPPUNIT_TEST( testBooleanConstants );
	CPPUNIT_TEST( testTimeUnits );
	CPPUNIT_TEST( testBinaryLogicOperatorsWithVariableOperand );
	CPPUNIT_TEST( testVariableSubscript );
	CPPUNIT_TEST( testRangeExpression );
	CPPUNIT_TEST_SUITE_END();
	
    
private:
    void createGlobalVariable(const std::string &name, Datum value);
    Datum getExpressionValue(const std::string &expr);
	
public:
	void testSimpleExpression();
	void testAlternativeLogicalOperators();
	void testAlternativeComparisonOperators();
    void testBooleanConstants();
    void testTimeUnits();
    void testBinaryLogicOperatorsWithVariableOperand();
    void testVariableSubscript();
    void testRangeExpression();
	
};


END_NAMESPACE_MW


#endif

