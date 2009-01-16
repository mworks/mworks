/*
 *  FilterTransforms.h
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 8/9/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

// =====================================================================
// specific types of variable transforms:

#ifndef _FILTER_TRANSFORMS
#define _FILTER_TRANSFORMS

#include "Transforms.h"
#include "VariableTransformAdaptors.h"

namespace mw {
// this is a simple, one-input filter with fixed kernel
// it keep track of time and returns a time with the output time that is the
//   time of the most recent value used as part of the filter
class BoxcarFilter1D : public TransformWithMemory {

	private:
		
		static const int DEFAULT_OUTPUT = 0;
		static const int NUM_INPUTS = 1;
		static const int NUM_OUTPUTS = 1;
		static const int THE_INPUT_INDEX = 0;
		double *dataVector;
        MonkeyWorksTime *timeVectorUS;
        bool lagCompensate;
        
	public:
		BoxcarFilter1D(int _boxcarWidthInElements, bool _lagCompensate = true); 
		~BoxcarFilter1D();
		
		// override of base class
		virtual void input(double inputValue, MonkeyWorksTime inputTimeUS);
		virtual bool output(double *outputValue, MonkeyWorksTime *outputTimeUS);
        
         // these versions do not worry about time
        virtual void input(double inputValue);
        virtual bool output(double *outputValue);
        
};



// this is a simple, one-input filter with fixed kernel
// it can be used to induce offsets, gains, and add noise to a variable
class LinearFilter1D : public TransformWithMemory {

	private:
		
		static const int DEFAULT_OUTPUT = 0;
		static const int NUM_INPUTS = 1;
		static const int NUM_OUTPUTS = 1;
		static const int THE_INPUT_INDEX = 0;
        static const int NUM_ITEMS_IN_MEMORY = 1;
		double *dataVector;
        MonkeyWorksTime *timeVectorUS;
        double offset;
        double gain;
        double noiseSD;
        
	public:
		LinearFilter1D(double _gain, double _offset, double _noiseSD); 
		~LinearFilter1D();
		
		// override of base class
		virtual void input(double inputValue, MonkeyWorksTime inputTimeUS);
		virtual bool output(double *outputValue, MonkeyWorksTime *outputTimeUS);
        

};






// simple two-point velocity computer
// output units = change in value per second.
// the output time is either the weighted average of the times 
//  or the latest time, depending on the flag for the TransformWithMemory class
class VelocityComputer1D : public TransformWithMemory {

	private:
		
		static const int DEFAULT_OUTPUT = 0;
		static const int NUM_INPUTS = 1;
		static const int NUM_OUTPUTS = 1;
		static const int THE_INPUT_INDEX = 0;
        static const int NUM_FILTER_ELEMENTS = 2;    // two point velocity computation
		double *dataVector;
        MonkeyWorksTime *timeVectorUS;
        bool lagCompensate;

	public:
		VelocityComputer1D(bool _lagCompensate = true); 
		~VelocityComputer1D();
		
		// override of base class
		virtual void input(double inputValue, MonkeyWorksTime inputTimeUS);
		virtual bool output(double *outputValue, MonkeyWorksTime *outputTimeUS);

};





/*
// each "channel" is a linear function of the inputs
// this filter will always wait until the 
class LinearFilter2D : public TransformWithMemory {
	
	private:
		// filter kernel
		ExpandableList<double> kernel11;
		ExpandableList<double> kernel12;
		ExpandableList<double> kernel21;
		ExpandableList<double> kernel22;

		setKernels(ExpandableList<double> *kernel11, ExpandableList<double> *kernel12, ExpandableList<double> *kernel21, ExpandableList<double> *kernel22);
		
	public:
		LinearFilter2D(shared_ptr<Variable> in1, shared_ptr<Variable> in2, shared_ptr<Variable> out1, shared_ptr<Variable> out1, int numMemoryElements): TransformWithMemory(int numMemoryElements);		// pass to base class
		~LinearFilter2D();
		
		// override of base class
		virtual newData(int inputIndex, Data *data);
		virtual transform();
		virtual setToDefaults();

}

*/


// ---- versions of each filter that are inhrited from the VarTransformAdaptor class
//  these are the things known to the editor

// this is how the editor can create boxcar filters
class Filter_BoxcarFilter1D : public VarTransformAdaptor{

	private:
		BoxcarFilter1D *filter;
		
    public:
        Filter_BoxcarFilter1D(shared_ptr<Variable> _inputVar, shared_ptr<Variable> _outputVar, int _filterWidthSamples);
        virtual ~Filter_BoxcarFilter1D();
        void  newDataReceived(int inputIndex, const Data& data, MonkeyWorksTime timeUS);
	
};
	
class Filter_BoxcarFilter1DFactory :  public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, 
												std::string> parameters,
												mwComponentRegistry *reg);
};        
          
   
// this is how the editor can create linear1D filters
class Filter_LinearFilter1D : public VarTransformAdaptor {

	private:
		LinearFilter1D *filter;
		
    public:
        Filter_LinearFilter1D(shared_ptr<Variable> _inputVar, shared_ptr<Variable> _outputVar, double _gain, double _offset, double _noiseSD);
        virtual ~Filter_LinearFilter1D();
        void  newDataReceived(int inputIndex, const Data& data, MonkeyWorksTime timeUS);
};	

class Filter_LinearFilter1DFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};        
 
}

#endif
