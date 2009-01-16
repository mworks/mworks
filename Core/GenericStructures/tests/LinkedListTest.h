#ifndef	LINKED_LIST_TEST_H_
#define LINKED_LIST_TEST_H_

/*
 *  LinkedListTest.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 1/19/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MonkeyWorksCore/Stimulus.h"
namespace mw {

class LinkedListTestFixture : public CppUnit::TestFixture {


	CPPUNIT_TEST_SUITE( LinkedListTestFixture );
	
	CPPUNIT_TEST( testAddToFront );
	CPPUNIT_TEST( testAddToBack );
	CPPUNIT_TEST( testMoveForward );
	CPPUNIT_TEST( testMoveBackward );
	CPPUNIT_TEST( testBringToFront );
	CPPUNIT_TEST( testSendToBack );
	CPPUNIT_TEST( testRemove );
	CPPUNIT_TEST( testEmpty );
	CPPUNIT_TEST( testOneNode );
	CPPUNIT_TEST( testGetNElements );
	CPPUNIT_TEST( testMultipleLists );
	CPPUNIT_TEST( testRemoveAndReadd );
	CPPUNIT_TEST( testShortList );
	CPPUNIT_TEST(testNotificationStyle);
	
	CPPUNIT_TEST_SUITE_END();


	private:
	
		LinkedList<StimulusNode> list, list2;
		shared_ptr<StimulusNode> a,b,c,d,node;

	public:
		void setUp();

		void tearDown();
					
		
		void testAddToFront();
		void testAddToBack();
		void testMoveForward();
		void testMoveBackward();
		void testBringToFront();
		void testShortList();
		void testSendToBack();
		void testRemove();
		void testEmpty();
		void testOneNode();
		void testGetNElements();
		void testMultipleLists();
		void testRemoveAndReadd();	
		void testNotificationStyle();
};
}
#endif
