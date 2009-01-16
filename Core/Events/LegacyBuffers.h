/*
 *  LegacyBuffers.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 4/5/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

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

#ifndef LEGACY_MONKEYWORKSCORE_BUFFERS_H
#define LEGACY_MONKEYWORKSCORE_BUFFERS_H


#include "Lockable.h"
#include "GenericData.h"
#include "boost/shared_ptr.hpp"
#include "Clock.h"

namespace mw {

class RingBuffer {
    protected:
        int previous_index, current_index, buffer_size;
        int nreaders;
		bool wrapped_once;
	boost::mutex buffer_lock;

    public:
        RingBuffer() {}
        RingBuffer(int bufsize);
        void advance();
        int currentIndex();
        int previousIndex();
        int previousIndex(int someindex);
        int getBufferSize();
        bool hasWrapped();
        void reset();
};

class TimeStampedRingBuffer : public RingBuffer {
	protected:
		MonkeyWorksTime *time;
		
	public:
        TimeStampedRingBuffer() : RingBuffer() {}
		TimeStampedRingBuffer(int buffersize);
		MonkeyWorksTime *getTime();
        MonkeyWorksTime getTime(int index);
};

// A simple interface class for reading from a buffer.  This allows the program 
// to track progress in reading a buffer, and allows for tracking of overflows.
class RingBufferReader {
    protected:
        int current_index, previous_index;
        int buffer_size;
        RingBuffer *buffer;
		boost::mutex buffer_reader_lock;
        
    public:
        RingBufferReader();
        RingBufferReader(RingBuffer *_buffer);
        int getCurrentIndex();
        bool advance();
		int advance(int ntimes);
        
		void forceAdvance();
		void forceAdvance(int ntimes);
		
		int getNItemsUnserviced();
        double getFillRatio();
        int seek(int relativePosition);	
};

class TimeStampedBufferReader : public RingBufferReader {
	public:
        TimeStampedBufferReader() : RingBufferReader() {}
		MonkeyWorksTime getTime();
};


template <class Class> 
class GenericRingBuffer : public RingBuffer {
	protected:
		Class *data;

	public:

        
		GenericRingBuffer() : RingBuffer() {}
		
		void putData(Class newdata){
			boost::mutex::scoped_lock locker(buffer_lock); 

            data[current_index] = newdata;
            advance();
		}
		
		Class getData(int index){
			boost::mutex::scoped_lock locker(buffer_lock); 
			
			return data[index];
		}
};



#define GenericDataRingBuffer GenericRingBuffer<Data>
#define BoolDataRingBuffer GenericRingBuffer<bool>;
#define ShortDataRingBuffer GenericRingBuffer<short>;
#define IntDataRingBuffer GenericRingBuffer<int>;
#define LongDataRingBuffer GenericRingBuffer<long>;
#define FloatDataRingBuffer GenericRingBuffer<float>;
#define DoubleDataRingBuffer GenericRingBuffer<double>;
#define StringDataRingBuffer GenericRingBuffer<char *>;

template <class Class>
class GenericBufferReader : public RingBufferReader{

    protected:
        GenericRingBuffer<Class> *the_buffer;

    public:
	
		GenericBufferReader(){ /* because GCC says so */ }
    
        GenericBufferReader(GenericRingBuffer<Class> *_buffer){
			the_buffer = _buffer;
			buffer = _buffer;
			buffer_size = buffer->getBufferSize();
		}
            
        Class *getData(){
			boost::mutex::scoped_lock locker(buffer_reader_lock); 
			return (static_cast<GenericRingBuffer<Class> *>(buffer))->getData(
																	  current_index);
		}

};


#define GenericDataBufferReader GenericBufferReader<Data>
#define BoolDataBufferReader GenericBufferReader<bool>
#define ShortDataBufferReader GenericBufferReader<short>
#define IntDataBufferReader GenericBufferReader<int>
#define LongDataBufferReader GenericBufferReader<long>
#define FloatDataBufferReader GenericBufferReader<float>
#define DoubleDataBufferReader GenericBufferReader<double>
#define StringDataBufferReader GenericBufferReader<char *>


template <class Class> 
class GenericTimeStampedRingBuffer : public TimeStampedRingBuffer {
	protected:
		Class *data;

	public:
	
		
        GenericTimeStampedRingBuffer() : TimeStampedRingBuffer() {}
		
		GenericTimeStampedRingBuffer(int size) : TimeStampedRingBuffer(size){
			data = new Class[size];
		}
			
			
		void putData(Class newdata){
			boost::mutex::scoped_lock locker(buffer_lock); 

			boost::shared_ptr <Clock> clock = Clock::instance();
			
            data[current_index] = newdata;
			time[current_index] = clock->getCurrentTimeUS();
            advance();
		}
		

		void putData(Class newdata, MonkeyWorksTime thetime){
			boost::mutex::scoped_lock locker(buffer_lock); 

            data[current_index] = newdata;
			time[current_index] = thetime;

            advance();
		}
		
		Class getData(int index){
			boost::mutex::scoped_lock locker(buffer_lock); 
			if(index >= buffer_size || index < 0){
				// warn?
				return Class();
			}
			
			return data[index];
		}
};

#define GenericDataTimeStampedRingBuffer GenericTimeStampedRingBuffer<Data>
#define BoolDataTimeStampedRingBuffer    GenericTimeStampedRingBuffer<bool> 
#define ShortDataTimeStampedRingBuffer   GenericTimeStampedRingBuffer<short>
#define IntDataTimeStampedRingBuffer     GenericTimeStampedRingBuffer<int>
#define LongDataTimeStampedRingBuffer    GenericTimeStampedRingBuffer<long>
#define FloatDataTimeStampedRingBuffer   GenericTimeStampedRingBuffer<float>
#define DoubleDataTimeStampedRingBuffer  GenericTimeStampedRingBuffer<double>
#define StringDataTimeStampedRingBuffer  GenericTimeStampedRingBuffer<char *>



template <class Class>
class GenericTimeStampedBufferReader : public TimeStampedBufferReader{

    protected:
        //mGenericTimeStampedRingBuffer<Class> *the_buffer;

    public:
	
		GenericTimeStampedBufferReader(){ /* because GCC says so */ }
    
        GenericTimeStampedBufferReader(
						GenericTimeStampedRingBuffer<Class> *_buffer):
													TimeStampedBufferReader(){
			//the_buffer = _buffer;
			buffer = _buffer;
			buffer_size = _buffer->getBufferSize();
		}
            
        Class getData(){
			boost::mutex::scoped_lock locker(buffer_reader_lock); 
			return (static_cast<GenericTimeStampedRingBuffer<Class> *>(buffer))->getData(current_index);
		}

};


#define GenericDataTimeStampedBufferReader GenericTimeStampedBufferReader<Data>
#define BoolDataTimeStampedBufferReader GenericTimeStampedBufferReader<bool>
#define ShortDataTimeStampedBufferReader GenericTimeStampedBufferReader<short>
#define IntDataTimeStampedBufferReader GenericTimeStampedBufferReader<int> 
#define LongDataTimeStampedBufferReader GenericTimeStampedBufferReader<long>
#define FloatDataTimeStampedBufferReader GenericTimeStampedBufferReader<float> 
#define DoubleDataTimeStampedBufferReader GenericTimeStampedBufferReader<double> 
#define StringDataTimeStampedBufferReader GenericTimeStampedBufferReader<char *>
}
#endif
