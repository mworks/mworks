#ifndef	PLATFORM_DEPENDENT_SERVICES_TEST_H_
#define PLATFORM_DEPENDENT_SERVICES_TEST_H_

/*
 *  PlatformDependentServicesTest.h
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy 07142006
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
namespace mw {
class PlatformDependentServicesTestFixture : public CppUnit::TestFixture {
  
  
	CPPUNIT_TEST_SUITE( PlatformDependentServicesTestFixture );
	
	CPPUNIT_TEST( testConstStrings );	
	
	CPPUNIT_TEST_SUITE_END();
	
	
 private:

	bool bejesus_beater(std::string saved, const char * charPtr, int counter);
	bool bejesus_beater(std::string *savedArray, 
						std::string *charPtrArray,
						int counter,
						int index);
	
 public:
	
	void setUp();
	void tearDown();
	void testConstStrings();
};
}


#endif

