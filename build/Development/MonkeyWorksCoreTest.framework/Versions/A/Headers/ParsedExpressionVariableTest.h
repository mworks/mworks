/*
 *  ParsedExpressionVariableTest.h
 *  MonkeyWorksCore
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
namespace mw {
class ParsedExpressionVariableTestFixture : public FullCoreEnvironmentTestFixture {
	
	
	CPPUNIT_TEST_SUITE( ParsedExpressionVariableTestFixture );
	CPPUNIT_TEST( testSimpleExpression );
	CPPUNIT_TEST_SUITE_END();
	
	
private:
public:

	
	void testSimpleExpression();
	
};
}

#endif

