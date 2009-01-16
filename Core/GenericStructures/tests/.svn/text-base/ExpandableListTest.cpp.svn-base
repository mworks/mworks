/*
 *  ExpandableListTest.cpp
 *  MonkeyWorksCore
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
	
	ExpandableList<Data> list;
	
	shared_ptr<Data> shared_data(new Data(10L));
	list.addElement(shared_data);
	
	CPPUNIT_ASSERT( list.getNElements() == 1 );
	CPPUNIT_ASSERT( *(list[0]) == 10L );
	
}

void ExpandableListTestFixture::testGetNElements(){
	fprintf(stderr, "mExpandableListTestFixture::testGetNElements()\n");
	ExpandableList<Data> list;
	
	CPPUNIT_ASSERT( list.getNElements() == 0 );
	
	shared_ptr<Data> a(new Data(10L));
	shared_ptr<Data> b(new Data(12L));
	shared_ptr<Data> c(new Data(15L));
	shared_ptr<Data> d(new Data(20L));
	
	list.addElement(a);
	list.addElement(b);
	list.addElement(c);
	list.addElement(d);
	
	CPPUNIT_ASSERT( list.getNElements() == 4 );
	
	
	
}

void ExpandableListTestFixture::testSetGetElement(){
	fprintf(stderr, "mExpandableListTestFixture::testSetGetElement()\n");
	
	ExpandableList<Data> *list = new ExpandableList<Data>();
	
	for(int i = 0; i < 10; i++){
		shared_ptr<Data> test(new Data((long)(2*i+1)));
		list->setElement(i,test);
	}
	
	for (int i = 0; i < 10; i++){
		Data test2 = *(list->getElement(i));
		
		CPPUNIT_ASSERT( (int) test2 ==  2*i+1 );
	}
	
	delete list;
}

void ExpandableListTestFixture::testBracketOperator(){
	fprintf(stderr, "mExpandableListTestFixture::testBracketOperator()\n");
	
	ExpandableList<Data> list;
	
	shared_ptr<Data> a(new Data(10L));
	list.addElement(a);
	
	CPPUNIT_ASSERT(  list[0] == list.getElement(0) );
	CPPUNIT_ASSERT( (long)(*(list[0])) == (long)(*a) );
}

// void ExpandableListTestFixture::testAddition(); // etc. etc...

