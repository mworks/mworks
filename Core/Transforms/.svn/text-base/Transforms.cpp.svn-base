/*
 *  Transforms.cpp
 *  MonkeyWorksCore
 *
 *  Created by James DiCarlo on 8/8/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "Transforms.h"
using namespace mw;

// standard transform interface
void Transform::input() {};
bool Transform::output() {return false;}
void Transform::setToDefaults() {};
void Transform::reset() {};


// constructor	
TransformWithMemory::TransformWithMemory(int _numInputs, int _numMemoryElementsPerInput) {

	numInputs = _numInputs;
	numMemoryElementsPerInput = _numMemoryElementsPerInput;
	numMemoryElementsInBuffer = numMemoryElementsPerInput*2;		// double size for now
    
	memoryBuffers = new ExpandableList<FloatDataTimeStampedRingBuffer>();
	memoryBufferReaders = new ExpandableList<FloatDataTimeStampedBufferReader>();
	
	for (int i=0;i<numInputs;i++) {
		shared_ptr<FloatDataTimeStampedRingBuffer> buffer( 
				new FloatDataTimeStampedRingBuffer(numMemoryElementsInBuffer));
		memoryBuffers->addReference(buffer);
		
		shared_ptr<FloatDataTimeStampedBufferReader> reader(
						new FloatDataTimeStampedBufferReader(buffer.get()));
		memoryBufferReaders->addReference(reader);
	}
    
	reset();	// make sure the buffers are empty
}

// destructor	
TransformWithMemory::~TransformWithMemory() {

	// delete the memory buffers
	memoryBufferReaders->clear();
	delete memoryBufferReaders;
	memoryBuffers->clear();
	delete memoryBuffers;
	
}

// PUBLIC METHOD
// empty the buffers of data 
// (move the reader to the writer and make sure the wrapped flag is false)
void TransformWithMemory::reset() {

    //lock();
	for (int inputIndex=0; inputIndex<memoryBuffers->getNElements(); inputIndex++ ) {
        resetProtected(inputIndex);
	}	

    Transform::reset();        // just good form
    //unlock();
    
}

// PUBLIC METHOD
void TransformWithMemory::reset(int inputIndex) {

    //lock();
    resetProtected(inputIndex);
    //unlock();
    
}

void TransformWithMemory::resetProtected(int inputIndex) {

    (memoryBuffers->getElement(inputIndex))->reset();
    (memoryBufferReaders->getElement(inputIndex))->seek(0);	
    
}



void TransformWithMemory::addElementToMemory(int inputIndex, double data, MonkeyWorksTime timeUS) {
	
	shared_ptr<FloatDataTimeStampedRingBuffer> memoryBuffer = (memoryBuffers->getElement(inputIndex));
	memoryBuffer->putData(data,timeUS); 
	
	// the buffer reader stays exactly numMemoryElementsPerInput behind (unless there is not enough data in the buffer yet)
	//mGenericDataTimeStampedBufferReader *memoryBufferReader = (memoryBufferReaders->getElement(inputIndex));
	//while (memoryBufferReader->getNItemsUnserviced()>numMemoryElementsPerInput) {
	//	memoryBufferReader->advance();
	//}
	
}




// get all useful data out of the buffer (usually the entire buffer)
// the last element in the vector is the most recent
int TransformWithMemory::getAllElements(int inputIndex, double *dataVector) {
	
	shared_ptr<FloatDataTimeStampedBufferReader> memoryBufferReader = 
								(memoryBufferReaders->getElement(inputIndex));
	
    // move the buffer reader back by the desired amount and get all data
    // if that much data is not available
    // note:  actualRelPosition is <= 0
    int actualRelPosition = memoryBufferReader->seek(-numMemoryElementsPerInput);
    int j=0;
    for (int i=actualRelPosition; i<0; i++) {
        dataVector[j] = memoryBufferReader->getData();
		memoryBufferReader->advance();
        j++;
	}
    int nData = j;

	return (nData);    // number of valid data returned
}


// get all useful data out of the buffer (usually the entire buffer)
// the last element in the vector is the most recent
// this routine will try to return a compTime
int TransformWithMemory::getAllElements(int inputIndex, double *dataVector, MonkeyWorksTime *timeVectorUS) {
	

	shared_ptr<FloatDataTimeStampedBufferReader> memoryBufferReader = 
								(memoryBufferReaders->getElement(inputIndex));
	
    // move the buffer reader back by the desired amount and get all data
    // if that much data is not available
    // note:  actualRelPosition is <= 0
    int actualRelPosition = memoryBufferReader->seek(-numMemoryElementsPerInput);
	int j=0;
    for (int i=actualRelPosition; i<0; i++) {
        dataVector[j] = memoryBufferReader->getData();
        timeVectorUS[j] = memoryBufferReader->getTime();
		memoryBufferReader->advance();
        j++;
	}
    int nData = j;

    
	return (nData);    // number of valid data returned
}








