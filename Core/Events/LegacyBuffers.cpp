/*
 *  LegacyBuffers.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 4/5/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "LegacyBuffers.h"

/*
 *  Buffers.cpp
 *  Experimental Control with Cocoa UI
 *
 *  Created by David Cox on Fri Dec 06 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "Buffers.h"
using namespace mw;

// class RingBuffer

#define AGGRESSIVE_LOCKING	1


RingBuffer::RingBuffer(int bufsize) {
	buffer_size = bufsize;	
	reset();
}



void RingBuffer::advance(){
	previous_index = current_index;
	current_index++;
	if(current_index >= buffer_size){
		current_index = 0;
		wrapped_once = 1;
	}
}

int RingBuffer::currentIndex(){
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_lock); 
#endif 
	return current_index;
}

int RingBuffer::previousIndex(){
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_lock); 
#endif 
	return previous_index;
}

int RingBuffer::previousIndex(int someindex){
	
	int tempvar;
	
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_lock); 
#endif 		
	if(!someindex){
		tempvar = buffer_size - 1;
		//return buffer_size - 1;
	} else {
		tempvar = someindex - 1;
		//return someindex - 1;
	}
	return tempvar;
}

int RingBuffer::getBufferSize(){
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_lock); 
#endif 
	
	return buffer_size;
}

bool RingBuffer::hasWrapped(){
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_lock); 
#endif 
	
	return wrapped_once;
}

void RingBuffer::reset(){
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_lock); 
#endif 
	
	current_index = 0;
	previous_index = -1;
	wrapped_once = false;
}

TimeStampedRingBuffer::TimeStampedRingBuffer(int bufsize) : RingBuffer(bufsize){
	time = (MonkeyWorksTime *)calloc(buffer_size, sizeof(MonkeyWorksTime));
	for(int i = 0; i < buffer_size; i++){
		time[i] = 0;
	}
}


MonkeyWorksTime *TimeStampedRingBuffer::getTime(){
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_lock); 
#endif 
	
	return time;			
}

MonkeyWorksTime TimeStampedRingBuffer::getTime(int index){
	if(index < 0 || index > buffer_size){
		merror(M_SYSTEM_MESSAGE_DOMAIN,"Buffer read out of bounds");
	}
	
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_lock); 
#endif 
	return time[index];
}




// class RingBufferReader 

RingBufferReader::RingBufferReader(RingBuffer *_buffer) {
	current_index = _buffer->currentIndex();
	previous_index = current_index-1;
	buffer = _buffer;
	buffer_size = buffer->getBufferSize();
}

RingBufferReader::RingBufferReader() {
	current_index = 0;
	previous_index = 0;
}

int RingBufferReader::getCurrentIndex(){
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_reader_lock); 
#endif 
	return current_index;
}


// TODO: may want to make this a boolean to indicate whether the
// reader actually advanced or not
// This method checks to make sure that there are in fact
// unserviced items before advancing
bool RingBufferReader::advance(){
	
	
	if(!getNItemsUnserviced()){
		// Jim, maybe put a message in here for debugging?
		return false;
	}
	
	forceAdvance();
	return true;
}

// This method advances the reader without any error checking
// if you need to move the reader no matter what (i.e. to catch up)
// use this one.				          
void RingBufferReader::forceAdvance(){
	
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_reader_lock); 
#endif 
	
	previous_index = current_index;
	current_index++;
	
	if(current_index >= buffer_size){
		current_index = 0;
	}
}

// TODO: make more efficient
// TODO: may want to return int to indicate how many spaces it advanced
int RingBufferReader::advance(int n_times){	
	int n_advanced = 0;
	for(int i = 0; i < n_times; i++){
		if(advance()){
			n_advanced++;
		}
	}
	
	return n_advanced;
}

// TODO: make more efficient
void RingBufferReader::forceAdvance(int n_times){	
	for(int i = 0; i < n_times; i++){
		forceAdvance();
	}
}

int RingBufferReader::getNItemsUnserviced(){
	
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_reader_lock); 
#endif 
	
	int nitems = 0;
	
	int buffer_current = buffer->currentIndex();
	if(buffer_current > current_index){
		nitems = buffer_current - current_index;
	} else if(buffer_current == current_index){
		nitems =0;
	} else {
		nitems = buffer_current + (buffer_size - current_index);
	}
	
	return nitems;
}

// JJD added August, 2005
// method to move reader to a position relative to the WRITER
//  ->seek(0) will thus synch the reader and the writer
// negative values mean move backwards
//  valid input range: [-(buffer_size-1), 0]
//   (values outside this range will be cut at the limits)
//  we also keep track of "lapping" of the writer and we do not 
//    allow the seek to move back to a position of no data
//  The return is thus the ACTUAL reader position relative to the buffer 
//  writer head.  output range: [-(buffer_size-1), 0]
int RingBufferReader::seek(int relativePosition){	
	
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_reader_lock); 
#endif 
	
	// this is how much we want to move the reader
	//ldiv_t temp = ldiv(relativePosition,buffer_size);
	//int desired_relative_position = temp.rem;
	int desired_relative_position = relativePosition;
	if (desired_relative_position > 0) {
		desired_relative_position = 0;
	}
	else if (desired_relative_position < -(buffer_size-1)) {
		desired_relative_position = -(buffer_size-1);
	}
	
	// current position of the buffer writer
	int buffer_current = buffer->currentIndex();
	
	// this is the index of the reader if we move the desired amount
	int desired_index = buffer_current + desired_relative_position;
	
	
	// bring the index into range
	if (desired_index >= buffer_size){
		desired_index = desired_index-buffer_size;
	}
	if (desired_index < 0 ) {
		desired_index = desired_index+buffer_size;
	}
	
	int actual_relative_position = desired_relative_position;
	
	// now desired_index should be in range [0,buffer_current), 
	//  special case when we have not yet wrapped 
	if ((!(buffer->hasWrapped())) && 
		((desired_index>buffer_current) || (desired_index<0))){
		desired_index = 0;
		actual_relative_position = -1*buffer_current;
	}
	
	
	// move the reader position
	current_index = desired_index;            
	previous_index = -1;   // not well defined in this case
	
	return actual_relative_position;
}



double RingBufferReader::getFillRatio(){
	int numunserviced = getNItemsUnserviced();	
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_reader_lock); 
#endif
	return (double)numunserviced/(double)buffer_size;
}


MonkeyWorksTime TimeStampedBufferReader::getTime(){
#ifdef AGGRESSIVE_LOCKING
	boost::mutex::scoped_lock locker(buffer_reader_lock); 
#endif 
	return ((TimeStampedRingBuffer *)buffer)->getTime(current_index);
}


