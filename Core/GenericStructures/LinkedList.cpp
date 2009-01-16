/*
 *  LinkedList.cpp
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on 10/20/04.
 *  Copyright 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "LinkedList.h"
#include "Utilities.h"
using namespace mw;


template <class T>
LinkedListNode<T>::LinkedListNode(){}


template <class T>
void LinkedListNode<T>::remove(){
	
	Lockable *list_lock = NULL;
	if(list != NULL){
		list_lock = list;
		list_lock->lock();
	}
	
	
	if(list != NULL){
		if(this == (list->getFrontmost()).get()){
			list->setFrontmost(getNext());
		}
		
		if(this == list->getBackmost()){
			list->setBackmost(getPrevious());
		}
	}
	
	if(previous != NULL){
		previous->setNext(next);
	}
	
	if(next != NULL){
		next->setPrevious(previous);
	}
	
	if(list != NULL){
		list->decrement();
	}
	
	next = NULL;
	previous = NULL;
	list = NULL;
	
	if(list_lock != NULL){
		list_lock->unlock();
	}
}


template <class T>
shared_ptr< LinkedListNode<T> > LinkedListNode<T>::getPrevious(){
	
	if(previous == this){
		return NULL;
	}
	
	return previous;
}

template <class T>
shared_ptr< LinkedListNode<T> > LinkedListNode<T>::getNext(){
	
	
	if(next == this){
		return NULL;
	}
	
	return next;
}

template <class T>
void LinkedListNode<T>::setPrevious(shared_ptr< LinkedListNode<T> > newprev){
	
	previous = newprev;
}

template <class T>
void LinkedListNode<T>::setNext(shared_ptr< LinkedListNode<T> > newnext){
	next = newnext;
}

template <class T>
void LinkedListNode<T>::moveUp(){
	moveForward();
}

template <class T>
void LinkedListNode<T>::moveDown(){
	moveBackward();
}

//void LinkedListNode<T>::moveForward(){
//	moveUp();
//}
//void LinkedListNode<T>::moveBackward(){
//	moveDown();
//}

template <class T>
void LinkedListNode<T>::moveForward(){
//void LinkedListNode<T>::moveDown(){
	
	Lockable *list_lock = NULL;
	if(list != NULL){
		list_lock = list;
		list_lock->lock();
	}
	
	shared_ptr< LinkedListNode<T> > theprev; 
	shared_ptr< LinkedListNode<T> > theprevprev; 
	shared_ptr< LinkedListNode<T> > thenext;

	if(previous == NULL){  // already at head
		return;
	}

	theprev = previous;
	thenext = next;
	
	// if this is the tail, it won't be after this operation
	if(list != NULL && list->getBackmost() == this){
		list->setBackmost(previous);
	}

	theprevprev = theprev->getPrevious();
	
	if(theprevprev){
		theprevprev->setNext(this);
	}
	
	previous = theprevprev;
	next = theprev;
	theprev->setPrevious(this);
	theprev->setNext(thenext);
	
	if(thenext){
		thenext->setPrevious(theprev);
	}
	
	if(list != NULL && previous == NULL){  // at head
		list->setFrontmost(this);
	}
	
	if(list_lock != NULL){
		list_lock->unlock();
	}
	
	// [* prev->prev *] [* prev *] [* this *] [* next *] 
}


template <class T>
void LinkedListNode<T>::moveBackward() {
	
	Lockable *list_lock = NULL;
	if(list != NULL){
		list_lock = list;
		list_lock->lock();
	}
	
	shared_ptr< LinkedListNode<T> > theprev; 
	shared_ptr< LinkedListNode<T> > thenext; 
	shared_ptr< LinkedListNode<T> > thenextnext;

	if(next == NULL){  // already at head
		return;
	}

	thenext = next;
	theprev = previous;

	// if this is the head, it won't be after this operation
	if(list != NULL && list->getFrontmost() == this){
		list->setFrontmost(next);
	}

	
	if(thenext == NULL){
		return;
	}
	thenextnext = thenext->getNext();
	
	if(thenextnext) {
		thenextnext->setPrevious(this);
	}
	
	next = thenextnext;
	previous = thenext;
	thenext->setNext(this);
	thenext->setPrevious(theprev);
	
	if(theprev) {
		theprev->setNext(thenext);
	}
	
	if(list != NULL && next == NULL){  // at tail
		list->setBackmost(this);
	}
	
	if(list_lock != NULL){
		list_lock->unlock();
	}
	//  [* prev *] [* this *] [* next *] [* next->next *]
}

template <class T>
void LinkedListNode<T>::bringToFront(){

	shared_ptr< LinkedListNode<T> > previous;
	while(previous = getPrevious()){
		moveForward();
	}
}

template <class T>
void LinkedListNode<T>::sendToBack(){

	shared_ptr< LinkedListNode<T> > next;
	while(next = getNext()){
		moveBackward();
	}
}

template <class T>
LinkedList<T>::LinkedList() : Lockable() {
	head = NULL;
	tail = NULL;
    nelements = 0;
}

template <class T>
shared_ptr< LinkedListNode<T> > LinkedList<T>::getFrontmost() {
	return head;
}

template <class T>
shared_ptr< LinkedListNode<T> > LinkedList<T>::getBackmost() {
	return tail;
}

template <class T>
void LinkedList<T>::addToFront(shared_ptr< LinkedListNode<T> > newnode) {
	lock();
	
	if(!head) {
		head = newnode;
	} else {
		newnode->setNext(head);
		head->setPrevious(newnode);
		head = newnode;
	}
	if(!tail) {
		tail = newnode;
	}
	
	newnode->setList(this);
	nelements++;
	
	unlock();
}

template <class T>
void LinkedList<T>::addToBack(shared_ptr< LinkedListNode<T> > newnode) {
	
	lock();
	
	if(!tail) {
		tail = newnode;
		tail->setNext(NULL);
	} else {
		newnode->setPrevious(tail);
		tail->setNext(newnode);
		tail = newnode;
	}
	if(!head) {
		head = newnode;
		head->setPrevious(NULL);
	}
	nelements++;
	
	unlock();
}

template <class T>
shared_ptr< LinkedListNode<T> > LinkedList<T>::popElement(){

	lock();
	
	if(head == NULL){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Attempt to pop an element off of a headless list");
		return NULL;
	}
	
	shared_ptr< LinkedListNode<T> > node = head;
	head = head->getNext();
	if(head != NULL){
		head->setPrevious(NULL);
	}
	
	// disconnect the old head
	node->setNext(NULL);
	nelements--;
	
	unlock();
	
	return node;
}
