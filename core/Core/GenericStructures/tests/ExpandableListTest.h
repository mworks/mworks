
#ifndef	EXPANDABLE_LIST_TEST_H_
#define EXPANDABLE_LIST_TEST_H_

/*
 *  ExpandableListTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksCore/GenericData.h"
#include "MWorksCore/ExpandableList.h"


BEGIN_NAMESPACE_MW


class ExpandableListTestFixture : public CppUnit::TestFixture {


	CPPUNIT_TEST_SUITE( ExpandableListTestFixture );
	
	CPPUNIT_TEST( testAddSharedPtr );
	CPPUNIT_TEST( testGetNElements );
	CPPUNIT_TEST( testSetGetElement );
	CPPUNIT_TEST( testBracketOperator );
	
	CPPUNIT_TEST_SUITE_END();


	private:
	

	public:
	
		void setUp();

	void tearDown();
			
					
	void testAddSharedPtr();
	void testGetNElements();
	void testSetGetElement();
	void testBracketOperator();
		
};


END_NAMESPACE_MW


#endif

 
