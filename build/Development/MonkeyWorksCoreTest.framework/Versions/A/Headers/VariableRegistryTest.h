#ifndef	VARIABLE_REGISTRY_TEST_H_
#define VARIABLE_REGISTRY_TEST_H_

/*
 *  VariableRegistryTest.h
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

#include "MonkeyWorksCore/ScarabServices.h"
#include "MonkeyWorksCore/VariableRegistry.h"
namespace mw {
class VariableRegistryTestFixture : public CppUnit::TestFixture {


	CPPUNIT_TEST_SUITE( VariableRegistryTestFixture );
	CPPUNIT_TEST( testCodec );
	CPPUNIT_TEST_SUITE_END();


	private:
		ScarabDatum *test_codec;
		void checkCodec(ScarabDatum *generated_codec, 
						const int number_of_entries) const;

	public:
	

		void setUp();
		void tearDown();
					
		void testCodec();

};
}
#endif

