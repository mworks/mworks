#ifndef	VARIABLE_PROPERTIES_TEST_H_
#define VARIABLE_PROPERTIES_TEST_H_

/*
 *  VariablePropertiesTest.h
 *  MWorksCore
 *
 *  Created by Ben Kennedy on 8/28/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksCore/VariableRegistry.h"


BEGIN_NAMESPACE_MW


class VariablePropertiesTestFixture : public CppUnit::TestFixture {
	
	
	CPPUNIT_TEST_SUITE( VariablePropertiesTestFixture );
	CPPUNIT_TEST( testGroups );
	CPPUNIT_TEST_SUITE_END();
	
	
private:
	
	shared_ptr<VariableRegistry> registry;
public:
		
		
	void setUp();
	void tearDown();
	
	void testGroups();
	
	
};


END_NAMESPACE_MW


#endif