/**
 * LinkedList.h
 *
 * Description: A simple linked list.
 *
 * History:
 * David Cox on 10/20/04 - Created.
 * Paul Jankunas on 1/24/06 - Adding virtual destructor.
 *
 * Copyright 2004 MIT. All rights reserved.
 */

#ifndef _LINKED_LIST_H
#define _LINKED_LIST_H

#include <stdio.h>
#include "Lockable.h"
#include "Utilities.h"
#include <boost/shared_ptr.hpp>
namespace mw {
using namespace boost;


//#define EMPTY_NODE	shared_ptr<LinkedList<T>>()
#define EMPTY_NODE	shared_ptr<T>()

template <class T>
class LinkedList;

template <class T>
class LinkedListNode {

	protected:

	shared_ptr<T> previous;
	shared_ptr<T> next;
		
		// parent list
		LinkedList<T> *list;
		
		shared_ptr<T> _getSelfReference(){
			shared_ptr<T> empty;
			
			if(previous != empty){
				return previous->getNext();
			}
			
			if(next != empty){
				return next->getPrevious();
			}
			
			if(list != NULL){
				return list->getFrontmost();
			}
			
			return empty;
		}
		
    public:

        LinkedListNode(){
			list = NULL;
		}
		
		virtual ~LinkedListNode(){};
				
        virtual void remove();
   
        
        virtual shared_ptr<T> getPrevious();
        virtual shared_ptr<T> getNext();
        
        virtual void setPrevious(shared_ptr<T> newprev);
        virtual void setNext(shared_ptr<T> newnext);
        
		
		virtual void moveForward();
		virtual void moveBackward();
		
        virtual void moveDown();
        virtual void moveUp();
		
		virtual void sendToBack();
		virtual void bringToFront();
		
		virtual LinkedList<T> *getList(){ return list; }
		virtual void setList(LinkedList<T> *_list){ list = _list; }
};

template <class T>
class LinkedList : public Lockable {

	protected:	

	shared_ptr<T> head;
	shared_ptr<T> tail;

	int nelements;
    
	public:

	LinkedList();

	shared_ptr<T> getFrontmost(); //head
	shared_ptr<T> getBackmost(); //tail

	void addToFront(shared_ptr<T> data);
	void addToBack(shared_ptr<T> data);
	
	shared_ptr<T> popElement();
	
    int getNElements() { return nelements; }
    // must call this after a remove need to fix this
    void decrement() { nelements--; }
	
	void setFrontmost(shared_ptr<T> node){ head = node; }
	void setBackmost(shared_ptr<T> node){tail = node; }

};



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
		
		if(this == (list->getBackmost()).get()){
			list->setBackmost(getPrevious());
		}
	}
	
	if(previous != EMPTY_NODE){
		previous->setNext(next);
	}
	
	if(next != EMPTY_NODE){
		next->setPrevious(previous);
	}
	
	if(list != NULL){
		list->decrement();
	}
	
	if(list_lock != NULL){
		list_lock->unlock();
	}
	
	next = shared_ptr<T>();
	previous = shared_ptr<T>();
	
	list = NULL;
}


template <class T>
shared_ptr<T> LinkedListNode<T>::getPrevious(){
	
	if(previous.get() == this){
		return shared_ptr<T>();
	}
	
	return previous;
}

template <class T>
shared_ptr<T> LinkedListNode<T>::getNext(){
	
	
	if(next.get() == this){
		return shared_ptr<T>();
	}
	
	return next;
}

template <class T>
void LinkedListNode<T>::setPrevious(shared_ptr<T> newprev){
	
	previous = newprev;
}

template <class T>
void LinkedListNode<T>::setNext(shared_ptr<T> newnext){
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
	
	shared_ptr<T> theprev; 
	shared_ptr<T> theprevprev; 
	shared_ptr<T> thenext;
	shared_ptr<T> self_ref;
	

	if(previous == EMPTY_NODE){  // already at head
		return;
	}

	theprev = previous;
	thenext = next;
	self_ref = _getSelfReference();
	
	// if this is the tail, it won't be after this operation
	if(list != NULL && (list->getBackmost()) == self_ref){
	   
		list->setBackmost(previous);
	}

	theprevprev = theprev->getPrevious();
	
	if(theprevprev){
		//theprevprev->setNext(this);
		theprevprev->setNext(theprev->getNext());
	}
	
	previous = theprevprev;
	next = theprev;
	theprev->setPrevious(self_ref);
	theprev->setNext(thenext);
	
	if(thenext){
		thenext->setPrevious(theprev);
	}
	
	if(list != NULL && previous == EMPTY_NODE){  // at head
		list->setFrontmost(self_ref);
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
	
	shared_ptr<T> theprev; 
	shared_ptr<T> thenext; 
	shared_ptr<T> thenextnext;
	shared_ptr<T> self_ref;

	if(next == EMPTY_NODE){  // already at head
		return;
	}

	thenext = next;
	theprev = previous;
	self_ref = _getSelfReference();

	// if this is the head, it won't be after this operation
	if(list != NULL && list->getFrontmost() == self_ref){
		list->setFrontmost(next);
	}

	
	if(thenext == EMPTY_NODE){
		return;
	}
	thenextnext = thenext->getNext();
	
	if(thenextnext) {
		thenextnext->setPrevious(self_ref);
	}
	
	next = thenextnext;
	previous = thenext;
	thenext->setNext(self_ref);
	thenext->setPrevious(theprev);
	
	if(theprev) {
		theprev->setNext(thenext);
	}
	
	if(list != NULL && next == EMPTY_NODE){  // at tail
		list->setBackmost(self_ref);
	}
	
	if(list_lock != NULL){
		list_lock->unlock();
	}
	//  [* prev *] [* this *] [* next *] [* next->next *]
}

template <class T>
void LinkedListNode<T>::bringToFront(){

	shared_ptr<T> previous;
	while(previous = getPrevious()){
		moveForward();
	}
}

template <class T>
void LinkedListNode<T>::sendToBack(){

	shared_ptr<T> next;
	while(next = getNext()){
		moveBackward();
	}
}

template <class T>
LinkedList<T>::LinkedList() : Lockable() {
    nelements = 0;
}

template <class T>
shared_ptr<T> LinkedList<T>::getFrontmost() {
	return head;
}

template <class T>
shared_ptr<T> LinkedList<T>::getBackmost() {
	return tail;
}

template <class T>
void LinkedList<T>::addToFront(shared_ptr<T> newnode) {
	lock();
	
	if(head == EMPTY_NODE) {
		head = newnode;
	} else {
		newnode->setNext(head);
		head->setPrevious(newnode);
		head = newnode;
	}
	if(tail == EMPTY_NODE) {
		tail = newnode;
	}
	
	newnode->setList(this);
	nelements++;
	
	unlock();
}

template <class T>
void LinkedList<T>::addToBack(shared_ptr<T> newnode) {
	
	lock();
	
	shared_ptr<T> empty;
	
	if(tail == EMPTY_NODE) {
		tail = newnode;
		tail->setNext(empty);
	} else {
		newnode->setPrevious(tail);
		tail->setNext(newnode);
		tail = newnode;
	}
	if(head == EMPTY_NODE) {
		head = newnode;
		head->setPrevious(empty);
	}
	nelements++;
	
	unlock();
}

template <class T>
shared_ptr<T> LinkedList<T>::popElement(){

	lock();
	
	if(head == EMPTY_NODE){
		mwarning(M_SYSTEM_MESSAGE_DOMAIN, "Attempt to pop an element off of a headless list");
		return EMPTY_NODE;
	}
	
	shared_ptr<T> node = head;
	head = head->getNext();
	if(head != EMPTY_NODE){
		head->setPrevious(EMPTY_NODE);
	}
	
	// disconnect the old head
	node->setNext(EMPTY_NODE);
	nelements--;
	
	unlock();
	
	return node;
}
}


#endif