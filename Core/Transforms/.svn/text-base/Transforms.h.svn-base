/*
 *  Transforms.h
 *  MonkeyWorksCore
 *
 *  Created by James DiCarlo on 8/8/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 *  This class is a standard interface for defining transforms between
 *  variables (e.g. filters).  
 *  It's main "value add" is the support for filters with memory.
 */

#ifndef _TRANSFORMS
#define _TRANSFORMS

#include "MonkeyWorksCore/GenericVariable.h"
#include "MonkeyWorksCore/ExpandableList.h"
#include "MonkeyWorksCore/Experiment.h"
#include "MonkeyWorksCore/Buffers.h"
namespace mw {
#define VERBOSE_TRANSFORMS 0

// this basically just defines the transform interface
// (no functionality)

class Transform :  public Lockable {

	public:
		
		virtual ~Transform(){ }
		
		// this method should take the number of inputs required of the transform
		//	and update the internal state of the transform
		//	e.g. input(double *input);
		virtual void input();	
		
		// this method should cause the object to report the current output(s)
		// (i.e. given your current state, tell me what your output is
		// e.g. call should look like:  ouptut(&output)
		// where output can be a vector
		virtual bool output();

		// should cause the transform to adopt a default state
		virtual void setToDefaults();
        
        // should reset any internal things back to start conditions
        //  (this is implemented for memory transforms)
        virtual void reset();

};

// this class should be used as a base for transforms that require memory (e.g. filters) 
// this class supports times with those data, but it can be used without time
// _lagCompensate:  (default = true)
//      true: means that that the object will try to return output times that are the
//          best estimate of when things actually happened ("neuroscientist mode")
//      false:  return the time of the most recent piece of data used in the filter
//          ("engineer mode")
//
class TransformWithMemory : public Transform {
	
	private:
		//memory arrays -- buffers and buffer readers to hold Data
		int numInputs;
		int numMemoryElementsPerInput;
		int numMemoryElementsInBuffer;
		ExpandableList<FloatDataTimeStampedRingBuffer> *memoryBuffers; 
		ExpandableList<FloatDataTimeStampedBufferReader> *memoryBufferReaders;

	protected:
		TransformWithMemory(int _numInputs, int _numMemoryElementsPerInput);
		~TransformWithMemory();
			
		int getNumInputs() { return(numInputs); }
		int getNumMemoryElementsPerInput() { return(numMemoryElementsPerInput); }
		        
        // request last numMemoryElementsPerInput elements (if less then numMemoryElementsPerInput exist, max avaialble returned)
        int getAllElements(int inputIndex, double *dataVector);
        int getAllElements(int inputIndex, double *dataVector, MonkeyWorksTime *timeVectorUS);
    
		//virtual void addElementToMemory(int inputIndex, Data data);
        // TODO PAJ the timeUS argument is not a pointer so it cannot be NULL
        // TODO PAJ it should either be 0 or the argument should be a pointer type.	
        virtual void addElementToMemory(int inputIndex, double data, MonkeyWorksTime timeUS = NULL);  
        virtual void resetProtected(int inputIndex);


    public:
        // calls when IO is about to re-start -- make sure we are fresh
		virtual void reset();					// clear the values in all buffers
		virtual void reset(int inputIndex);     // clear the values in a particular buffer

    
			
};

}

#endif

