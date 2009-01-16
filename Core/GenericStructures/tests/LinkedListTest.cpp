/*
 *  LinkedListTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 1/19/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "LinkedListTest.h"
using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( LinkedListTestFixture, "Unit Test" );

void LinkedListTestFixture::setUp(){
	a = shared_ptr<StimulusNode>(new StimulusNode());
	b = shared_ptr<StimulusNode>(new StimulusNode());
	c = shared_ptr<StimulusNode>(new StimulusNode());
	d = shared_ptr<StimulusNode>(new StimulusNode());
}

void LinkedListTestFixture::tearDown() {}



void LinkedListTestFixture::testAddToFront(){
	fprintf(stderr, "Running LinkedListTestFixture::testAddToFront()\n");
	
	list.addToFront(a);
	list.addToFront(b);
	list.addToFront(c);
	
	node = list.getFrontmost();
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == b );
	node = node->getNext();
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == NULL );
	
	node = list.getBackmost();
	CPPUNIT_ASSERT( node == a );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == b );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == c );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == NULL );
}

void LinkedListTestFixture::testAddToBack(){
	fprintf(stderr, "Running LinkedListTestFixture::testAddToBack()\n");
	
	list.addToBack(a);
	list.addToBack(b);
	list.addToBack(c);
	
	node = list.getBackmost();
	CPPUNIT_ASSERT( node == c );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == b );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == a );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == NULL );
	
	node = list.getFrontmost();
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == b );
	node = node->getNext();
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == NULL );
}	


void LinkedListTestFixture::testMoveForward(){
	fprintf(stderr, "Running LinkedListTestFixture::testMoveForward()\n");
	
	list.addToFront(d);
	list.addToFront(c);
	list.addToFront(b);
	list.addToFront(a);
	
	c->moveForward();
	
	node = list.getFrontmost();
	
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == b );
	node = node->getNext();
	CPPUNIT_ASSERT( node == d );
	node = node->getNext();			
	CPPUNIT_ASSERT( node == NULL );
	
	
	c->moveForward();
	node = list.getFrontmost();
	
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == b );
	node = node->getNext();
	CPPUNIT_ASSERT( node == d );
	node = node->getNext();			
	CPPUNIT_ASSERT( node == NULL );
	
	c->moveForward();
	node = list.getFrontmost();
	
	CPPUNIT_ASSERT( node == c );
	
}



void LinkedListTestFixture::testMoveBackward(){
	fprintf(stderr, "Running LinkedListTestFixture::testMoveBackward()\n");
	
	list.addToFront(d);
	list.addToFront(c);
	list.addToFront(b);
	list.addToFront(a);
	
	b->moveBackward();
	
	node = list.getFrontmost();
	
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == b );
	node = node->getNext();
	CPPUNIT_ASSERT( node == d );
	node = node->getNext();			
	CPPUNIT_ASSERT( node == NULL );
	
	
	b->moveBackward();
	node = list.getFrontmost();
	
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == d );
	node = node->getNext();
	CPPUNIT_ASSERT( node == b );
	node = node->getNext();			
	CPPUNIT_ASSERT( node == NULL );
	
	b->moveBackward();
	node = list.getBackmost();
	
	CPPUNIT_ASSERT( node == b );
	
}

void LinkedListTestFixture::testBringToFront(){
	fprintf(stderr, "Running LinkedListTestFixture::testBringToFront()\n");
	
	
	list.addToFront(d);
	list.addToFront(c);
	list.addToFront(b);
	list.addToFront(a);
	
	c->bringToFront();
	
	node = list.getFrontmost();
	
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == b );
	node = node->getNext();
	CPPUNIT_ASSERT( node == d );
	node = node->getNext();			
	CPPUNIT_ASSERT( node == NULL );
	
	node = list.getBackmost();
	
	CPPUNIT_ASSERT( node == d );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == b );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == a );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == c );
	node = node->getPrevious();			
	CPPUNIT_ASSERT( node == NULL );
	
}


void LinkedListTestFixture::testShortList(){
	fprintf(stderr, "Running LinkedListTestFixture::testShortList()\n");
	
	list.addToFront(b);
	list.addToFront(a);
	
	a->sendToBack();
	
	CPPUNIT_ASSERT( list.getFrontmost() == b );
	CPPUNIT_ASSERT( list.getBackmost() == a );
	
	a->bringToFront();
	
	CPPUNIT_ASSERT( list.getFrontmost() == a );
	CPPUNIT_ASSERT( list.getBackmost() == b );
	
}


void LinkedListTestFixture::testSendToBack(){
	fprintf(stderr, "Running LinkedListTestFixture::testSendToBack()\n");
	
	list.addToFront(d);
	list.addToFront(c);
	list.addToFront(b);
	list.addToFront(a);
	
	b->sendToBack();
	
	node = list.getFrontmost();
	
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == d );
	node = node->getNext();
	CPPUNIT_ASSERT( node == b );
	node = node->getNext();			
	CPPUNIT_ASSERT( node == NULL );
	
	node = list.getBackmost();
	
	CPPUNIT_ASSERT( node == b );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == d );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == c );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == a );
	node = node->getPrevious();			
	CPPUNIT_ASSERT( node == NULL );
	
}

void LinkedListTestFixture::testRemove(){
	fprintf(stderr, "Running LinkedListTestFixture::testRemove()\n");
	
	list.addToFront(d);
	list.addToFront(c);
	list.addToFront(b);
	list.addToFront(a);
	
	b->remove();
	
	node = list.getFrontmost();
	
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == d );
	node = node->getNext();
	CPPUNIT_ASSERT( node == NULL );
	
	node = list.getBackmost();
	
	CPPUNIT_ASSERT( node == d );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == c );
	node = node->getPrevious();
	CPPUNIT_ASSERT( node == a );
	node = node->getPrevious();			
	CPPUNIT_ASSERT( node == NULL );
	
}

void LinkedListTestFixture::testEmpty(){
	fprintf(stderr, "Running LinkedListTestFixture::testEmpty()\n");
	
	node = list.getFrontmost();
	CPPUNIT_ASSERT( node == NULL );
	
	node = list.getBackmost();
	CPPUNIT_ASSERT( node == NULL );
}

void LinkedListTestFixture::testOneNode(){
	fprintf(stderr, "Running LinkedListTestFixture::testOneNode()\n");
	
	
	list.addToFront(a);
	
	node = list.getFrontmost();
	CPPUNIT_ASSERT( node == a );
	
	node = list.getBackmost();
	CPPUNIT_ASSERT( node == a );
	
	a->remove();
}

void LinkedListTestFixture::testGetNElements(){
	fprintf(stderr, "Running LinkedListTestFixture::testGetNElements()\n");
	
	CPPUNIT_ASSERT( list.getNElements() == 0 );
	
	list.addToFront(a);
	CPPUNIT_ASSERT( list.getNElements() == 1 );
	
	list.addToFront(b);
	CPPUNIT_ASSERT( list.getNElements() == 2 );
	
	list.addToFront(c);
	CPPUNIT_ASSERT( list.getNElements() == 3 );
	
	list.addToFront(d);
	CPPUNIT_ASSERT( list.getNElements() == 4 );
	
	a->remove();
	CPPUNIT_ASSERT( list.getNElements() == 3 );
	
	b->remove();
	CPPUNIT_ASSERT( list.getNElements() == 2 );
	
	c->remove();
	CPPUNIT_ASSERT( list.getNElements() == 1 );
	
	d->remove();
	CPPUNIT_ASSERT( list.getNElements() == 0 );
}

void LinkedListTestFixture::testMultipleLists(){
	fprintf(stderr, "Running LinkedListTestFixture::testMultipleLists()\n");
	
	
	list.addToFront(a);
	a->remove();
	list2.addToFront(a);
	
	CPPUNIT_ASSERT( list.getNElements() == 0 );
	CPPUNIT_ASSERT( list2.getNElements() == 1 );
	
}


void LinkedListTestFixture::testRemoveAndReadd(){
	fprintf(stderr, "Running LinkedListTestFixture::testRemoveAndReadd()\n");
	
	fflush(stderr);
	
	list.addToFront(a);
	list.addToFront(b);
	list.addToFront(c);
	list.addToFront(d);
	c->remove();
	
	list.addToFront(c);
	
	CPPUNIT_ASSERT( list.getNElements() == 4 );
	
	node = list.getFrontmost();
	
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == d );
	node = node->getNext();
	CPPUNIT_ASSERT( node == b );
	node = node->getNext();
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == NULL );
	
	c->remove();
	
	list.addToFront(c);
	
	CPPUNIT_ASSERT( list.getNElements() == 4 );
	
	node = list.getFrontmost();
	
	CPPUNIT_ASSERT( node == c );
	node = node->getNext();
	CPPUNIT_ASSERT( node == d );
	node = node->getNext();
	CPPUNIT_ASSERT( node == b );
	node = node->getNext();
	CPPUNIT_ASSERT( node == a );
	node = node->getNext();
	CPPUNIT_ASSERT( node == NULL );
	
}

void LinkedListTestFixture::testNotificationStyle() {
	LinkedList<VariableNotification> list3;

{
	fprintf(stderr, "\nentering scoped section\n"); 
	shared_ptr<VariableNotification> vn = shared_ptr<VariableNotification>(new VariableNotification());
	ExpandableList<VariableNotification> *notifications = new ExpandableList<VariableNotification>();

	list3.addToFront(vn);
	notifications->addReference(vn);

	CPPUNIT_ASSERT(notifications->getNElements() == 1);
	CPPUNIT_ASSERT(list3.getNElements() == 1);
	
	delete notifications;
	vn->remove();
	fprintf(stderr, "exiting scoped section\n"); 
}
	fprintf(stderr, "everything is out of scope\n"); 

	CPPUNIT_ASSERT(list3.getNElements() == 0);

}
