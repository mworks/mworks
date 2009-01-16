/*
 *  VariableTest.h
 *  MonkeyWorksCore
 *
 *  Created by labuser on 10/2/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#ifndef _VARIABLE_TEST_H_
#define _VARIABLE_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "FullCoreEnvironmentTest.h"
namespace mw {
class VariableTestFixture : public FullCoreEnvironmentTestFixture {
	
	
	CPPUNIT_TEST_SUITE( VariableTestFixture );
	CPPUNIT_TEST( testSimpleConstant );
	CPPUNIT_TEST( testSimpleGlobal );
	CPPUNIT_TEST( testSimpleExpression );
	CPPUNIT_TEST( testSimpleClone );
	CPPUNIT_TEST( testExpressionClone );
	CPPUNIT_TEST_SUITE_END();
	
	
private:
public:

	void testSimpleConstant();
	void testSimpleGlobal();
	void testSimpleExpression();
	void testSimpleClone();
	void testExpressionClone();
	
	
};
}

#endif

