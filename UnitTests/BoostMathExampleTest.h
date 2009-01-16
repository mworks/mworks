#ifndef	BOOST_MATH_EXAMPLE_TEST_H_
#define BOOST_MATH_EXAMPLE_TEST_H_

/*
 *  BoostMathExampleTest.h
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy 12022006
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace mw {
	
	class BoostMathExampleTestFixture : public CppUnit::TestFixture {
		
		CPPUNIT_TEST_SUITE( BoostMathExampleTestFixture );
		CPPUNIT_TEST( example1Test );
		CPPUNIT_TEST( example2Test );
		CPPUNIT_TEST( example3Test );
		CPPUNIT_TEST( example4Test );
		CPPUNIT_TEST_SUITE_END();
		
		
	private:
		
	public:
		
		void setUp();
		void tearDown();
		void example1Test();
		void example2Test();
		void example3Test();
		void example4Test();
	};
	
}

#endif

