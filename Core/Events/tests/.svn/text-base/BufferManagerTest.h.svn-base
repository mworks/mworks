#ifndef	BUFFER_MANAGER_TEST_H_
#define BUFFER_MANAGER_TEST_H_

/*
 *  BufferManagerTest.h
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy 12032006
 *  Copyright 2006 MIT. All rights reserved.
 *
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MonkeyWorksCore/EventBuffer.h"
#include "MonkeyWorksCore/Lockable.h"

namespace mw {

class BufferManagerTestFixture : public CppUnit::TestFixture {
  
  
	CPPUNIT_TEST_SUITE( BufferManagerTestFixture );
	CPPUNIT_TEST( testSimpleBufferManager );
	CPPUNIT_TEST( testForLeaking );
	CPPUNIT_TEST( testForLeaking2 );
	CPPUNIT_TEST( testForLeaking3 );
	CPPUNIT_TEST( testMultipleReaders );
	CPPUNIT_TEST( testMultipleReadersWithAddtionalPuts );
	CPPUNIT_TEST( testBufferManagerDeletion );
	CPPUNIT_TEST( multiThreadInsertTest );
	CPPUNIT_TEST( multiThreadReadTest );
	CPPUNIT_TEST( multiMultiTest );

	CPPUNIT_TEST_SUITE_END();
	
	
 private:
 public:
		
	void setUp();
	void tearDown();
	void testSimpleBufferManager();
	void testForLeaking();
	void testForLeaking2();
	void testForLeaking3();
	void testMultipleReaders();
	void testMultipleReadersWithAddtionalPuts();
	void testBufferManagerDeletion();
	void multiThreadInsertTest();
	void multiThreadReadTest();
	void multiMultiTest();
};
}
#endif

