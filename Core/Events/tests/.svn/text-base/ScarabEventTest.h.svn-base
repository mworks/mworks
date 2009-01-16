#ifndef	SCARAB_EVENT_TEST_H_
#define SCARAB_EVENT_TEST_H_

/*
 *  ScarabEventTest.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MonkeyWorksCore/VariableRegistry.h"
#include "MonkeyWorksCore/ScarabServices.h"
namespace mw {
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

}

#endif

