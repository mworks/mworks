/*
 *  ExpandableListTest.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "ExpandableListTest.h"
using namespace mw;


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ExpandableListTestFixture, "Unit Test" );
//CPPUNIT_TEST_SUITE_REGISTRATION( ExpandableListTestFixture );

void ExpandableListTestFixture::setUp(){}

void ExpandableListTestFixture::tearDown(){}


void ExpandableListTestFixture::testAddSharedPtr(){
	fprintf(stderr, "mExpandableListTestFixture::testAddSharedPtr()\n");
	
	ExpandableList<Datum> list;
	
	shared_ptr<Datum> shared_data(new Datum(10L));
	list.addElement(shared_data);
	
	CPPUNIT_ASSERT( list.getNElements() == 1 );
	CPPUNIT_ASSERT( *(list[0]) == 10L );
	
}

void ExpandableListTestFixture::testGetNElements(){
	fprintf(stderr, "mExpandableListTestFixture::testGetNElements()\n");
	ExpandableList<Datum> list;
	
	CPPUNIT_ASSERT( list.getNElements() == 0 );
	
	shared_ptr<Datum> a(new Datum(10L));
	shared_ptr<Datum> b(new Datum(12L));
	shared_ptr<Datum> c(new Datum(15L));
	shared_ptr<Datum> d(new Datum(20L));
	
	list.addElement(a);
	list.addElement(b);
	list.addElement(c);
	list.addElement(d);
	
	CPPUNIT_ASSERT( list.getNElements() == 4 );
	
	
	
}

void ExpandableListTestFixture::testSetGetElement(){
	fprintf(stderr, "mExpandableListTestFixture::testSetGetElement()\n");
	
	ExpandableList<Datum> *list = new ExpandableList<Datum>();
	
	for(int i = 0; i < 10; i++){
		shared_ptr<Datum> test(new Datum((long)(2*i+1)));
		list->setElement(i,test);
	}
	
	for (int i = 0; i < 10; i++){
	 Datum test2 = *(list->getElement(i));
		
		CPPUNIT_ASSERT( (int) test2 ==  2*i+1 );
	}
	
	delete list;
}

void ExpandableListTestFixture::testBracketOperator(){
	fprintf(stderr, "mExpandableListTestFixture::testBracketOperator()\n");
	
	ExpandableList<Datum> list;
	
	shared_ptr<Datum> a(new Datum(10L));
	list.addElement(a);
	
	CPPUNIT_ASSERT(  list[0] == list.getElement(0) );
	CPPUNIT_ASSERT( (long)(*(list[0])) == (long)(*a) );
}

// void ExpandableListTestFixture::testAddition(); // etc. etc...

