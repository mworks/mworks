/**
 * Buffers.h
 *
 * Description: a simple base encapsulation of a timestamping ring buffer
 * This class is not meant to be used directly, but rather should be extended 
 * through inheritance to include data arrays.  This class contains only the
 * looping mechanism and the time stamp buffer + a mutex for locking 
 * access to the buffer.
 *
 * NOTE: This file generates a lot of warnings.  It seems that the classes
 * need some default constructors defined because of the inherited classes
 * that have no functions in them.
 * 
 * History:
 * Dave Cox on ??/??/?? - Created.
 * Paul Jankunas on 05/24/05 - Fixed a typo that was generating a warning.
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#ifndef MONKEYWORKSCORE_BUFFERS_H
#define MONKEYWORKSCORE_BUFFERS_H


#include "Lockable.h"
#include "GenericData.h"

#include "LegacyBuffers.h"

namespace mw {

// A singly linked list

template <class T>
class BufferElement : public Lockable {
protected:
	shared_ptr< T > data;
	shared_ptr< BufferElement<T> > next;
public:

	BufferElement(shared_ptr< T > _data){  data = _data; }

	void setNext(shared_ptr< BufferElement<T> > _next){
		lock();
		next = _next;
		unlock();
	}
	
	shared_ptr<T> get() {  return data; }

};

template <class T>
class Buffer : public Lockable {

protected:

	shared_ptr< BufferElement<T> > current;
	
public:

	shared_ptr< BufferElement<T> > getCurrent(){  
		lock();
		shared_ptr< BufferElement<T> > returnval = current;
		unlock();
		return returnval; 
	}
	
	void putData(shared_ptr<T> data){
		shared_ptr< BufferElement<T> > new_element(new BufferElement<T>(data));
		lock();
		current->setNext(new_element);
		unlock();
	}
};

template <class T>
class BufferReader : public Lockable {

protected:
	shared_ptr< Buffer<T> > buffer;
	shared_ptr< BufferElement<T> > current;
	
public:

	BufferReader(shared_ptr< Buffer<T> > _buffer){  
		buffer = _buffer; 
		current = buffer->getCurrent();
	}
	
	shared_ptr<T> getData(){
		lock();
		if(current == shared_ptr<T>()){
			unlock();
			return shared_ptr<T>();
		}
		
		shared_ptr<T> returnval = current->get();
		//current = current->getNext();
		unlock();
		return returnval;
	}
	
	bool advance(){
		lock();
		shared_ptr<T> next = current->getNext();
		if(next == shared_ptr<T>()){
			unlock();
			return false;
		} else {
			current = next;
		}
		
		unlock();
		return true;
	}
};
}

#endif
