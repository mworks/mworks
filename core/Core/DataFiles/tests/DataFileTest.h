#ifndef	DATA_FILE_TEST_H_
#define DATA_FILE_TEST_H_

/*
 *  DatumFileTest.h
 *  MWorksCore
 *
 *  Created by Ben Kennedy 12032006
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksCore/EventBuffer.h"
#include "MWorksCore/Lockable.h"
#include "MWorksCore/DataFileManager.h"


BEGIN_NAMESPACE_MW


class DatumFileTestFixture : public CppUnit::TestFixture {
  
  
	CPPUNIT_TEST_SUITE( DatumFileTestFixture );
	CPPUNIT_TEST( initializationTest );	
	CPPUNIT_TEST( dataFileWriterTest );
	CPPUNIT_TEST( multiDataFileWriterTest );

	CPPUNIT_TEST_SUITE_END();
	
	
 private:
 DataFileManager *dfm;
 public:
		
	void setUp();
	void tearDown();
	void initializationTest();
	void dataFileWriterTest();
	void multiDataFileWriterTest();
};


END_NAMESPACE_MW


#endif

