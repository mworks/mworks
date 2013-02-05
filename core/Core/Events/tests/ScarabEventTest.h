#ifndef	SCARAB_EVENT_TEST_H_
#define SCARAB_EVENT_TEST_H_

/*
 *  ScarabEventTest.h
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

#include "MWorksCore/VariableRegistry.h"
#include "MWorksCore/ScarabServices.h"


BEGIN_NAMESPACE_MW


class ScarabEventTestFixture : public CppUnit::TestFixture {


	CPPUNIT_TEST_SUITE( ScarabEventTestFixture );
	CPPUNIT_TEST( testToFromScarabDatum );
	CPPUNIT_TEST_SUITE_END();


	private:
	
		VariableRegistry *reg;

	public:
	
		void setUp();
		void tearDown();
					
		void testToFromScarabDatum();
};


END_NAMESPACE_MW


#endif

