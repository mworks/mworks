/*
 *  FilterTransforms.cpp
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 8/9/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "FilterTransforms.h"
#include "ComponentRegistry_new.h"
#include <boost/lexical_cast.hpp>
using namespace mw;

#define VERBOSE_FILTERS 0       // set to 1 to see messages from filter methods 


// ==== specific types of transforms follow:

// Filters:

//    BoxcarFilter1D ................................................

BoxcarFilter1D::BoxcarFilter1D(int boxcarWidthInElements, bool _lagCompensate): TransformWithMemory(NUM_INPUTS, boxcarWidthInElements) {

    lagCompensate = _lagCompensate;
    dataVector = new double [boxcarWidthInElements]; // DDC changed: this is more efficient
    timeVectorUS = new MonkeyWorksTime [boxcarWidthInElements];
}

BoxcarFilter1D::~BoxcarFilter1D() {
    delete dataVector;
    delete timeVectorUS;
} 

void BoxcarFilter1D::input(double inputValue, MonkeyWorksTime inputTimeUS) {
    
    //lock();
    //int i = 1;
	// protected method of the TransformsWithMemoryClass 
	this->addElementToMemory(THE_INPUT_INDEX, (Data)inputValue, inputTimeUS);	
    //unlock();
    
}

void BoxcarFilter1D::input(double inputValue) {

    //lock();
    //int i = 1;
	// protected method of the TransformsWithMemoryClass 
	this->addElementToMemory(THE_INPUT_INDEX, (Data)inputValue);	
	//unlock();
}


bool BoxcarFilter1D::output(double *output, MonkeyWorksTime *outputTimeUS) {

    lock();
    // cause pull of latest values from buffer into a vector for processing
    // (this keeps the buffers free to do their thing and simplifies the writing 
    //  of these filters)
    //MonkeyWorksTime compTimeUS;
	int numData = getAllElements(THE_INPUT_INDEX, dataVector, timeVectorUS);
	
    if (numData == 0) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Boxcar transform requested without any input data.");
		*output = DEFAULT_OUTPUT;
		shared_ptr <Clock> clock = Clock::instance();
        *outputTimeUS = clock->getCurrentTimeUS();
        unlock();	
		return false;
    }

    // boxcar function in here
    double temp = 0.0;
    for (int i=0;i<numData;i++) {
        temp = temp + (double)(dataVector[i]);
    }
    temp = temp/(double)numData;
    *output = temp;
    
    if (lagCompensate) {
        MonkeyWorksTime base = timeVectorUS[0];
        long temp = 0;
        for (int i=0;i<numData;i++) {
            temp = temp + (long)(timeVectorUS[i]-base);
        }
        temp = temp/numData;
        *outputTimeUS = base + (MonkeyWorksTime)temp; 
    }
    else {
        *outputTimeUS = timeVectorUS[numData-1];      // latest time
    }
    
    unlock();
    return true;

}	

bool BoxcarFilter1D::output(double *output) {

    lock();
    // cause pull of latest values from buffer into a vector for processing
    // (this keeps the buffers free to do their thing and simplifies the writing 
    //  of these filters)
	int numData = getAllElements(THE_INPUT_INDEX, dataVector);
	
    if (numData == 0) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Boxcar transform requested without any input data.");
		*output = DEFAULT_OUTPUT;
		unlock();
        return false;
    }

    // boxcar function in here
    double temp = 0.0;
    for (int i=0;i<numData;i++) {
        temp = temp + (double)(dataVector[i]);
    }
    temp = temp/(double)numData;
    *output = temp;
    unlock();
    return true;

}





//    LinearFilter1D ................................................

LinearFilter1D::LinearFilter1D(double _gain, double _offset, double _noiseSD): TransformWithMemory(NUM_INPUTS, NUM_ITEMS_IN_MEMORY) {
    
    dataVector = new double [NUM_ITEMS_IN_MEMORY];
    timeVectorUS = new MonkeyWorksTime [NUM_ITEMS_IN_MEMORY];
    offset = _offset;
    gain = _gain;
    noiseSD = _noiseSD;
    
}

LinearFilter1D::~LinearFilter1D() {
    delete dataVector;
    delete timeVectorUS;
} 

void LinearFilter1D::input(double inputValue, MonkeyWorksTime inputTimeUS) {
	
    //lock();
    this->addElementToMemory(THE_INPUT_INDEX, (Data)inputValue, inputTimeUS);		
    //unlock();
}

bool LinearFilter1D::output(double *output, MonkeyWorksTime *outputTimeUS) {

    lock();
    // cause pull of latest values from buffer into a vector for processing
    // (this keeps the buffers free to do their thing and simplifies the writing 
    //  of these filters)
    // in this case, this should just be one element
	int numData = getAllElements(THE_INPUT_INDEX, dataVector, timeVectorUS);
	
    if (numData == 0) {
		mwarning(M_SYSTEM_MESSAGE_DOMAIN,
			"Linear Filter requested without any input data.");
		*output = DEFAULT_OUTPUT;
		shared_ptr <Clock> clock = Clock::instance();
        *outputTimeUS = clock->getCurrentTimeUS();
        unlock();	
		return false;
    }

    // filter function in here
    double temp = (double)(dataVector[0]);
    double noiseUniform = (noiseSD*2*(((double)rand() / (double)RAND_MAX) - 0.5));
    double temp2 = offset + (gain*temp) + noiseUniform;
      
    *output = temp2;
    *outputTimeUS = timeVectorUS[0];      // latest time
    unlock();
    return true;

}	





// =================================================================

VelocityComputer1D::VelocityComputer1D(bool _lagCompensate): TransformWithMemory(NUM_INPUTS, NUM_FILTER_ELEMENTS) {
    
    lagCompensate = _lagCompensate;
    dataVector = new double [NUM_FILTER_ELEMENTS];
    timeVectorUS = new MonkeyWorksTime [NUM_FILTER_ELEMENTS];
}

VelocityComputer1D::~VelocityComputer1D() {
    delete dataVector;
    delete timeVectorUS;
} 


void VelocityComputer1D::input(double eyeLoc, MonkeyWorksTime timeUS) {
    
    //lock();
    this->addElementToMemory(THE_INPUT_INDEX, (Data)eyeLoc, timeUS);
    //unlock();
    
}

  
    
// this routine assume 2 values!!!    
bool VelocityComputer1D::output(double *output, MonkeyWorksTime *outputTimeUS) {
            
	int numData = getAllElements(THE_INPUT_INDEX, dataVector, timeVectorUS);
	
    if (numData ==0) {
		*output = DEFAULT_OUTPUT;
		shared_ptr <Clock> clock = Clock::instance();
        *outputTimeUS = clock->getCurrentTimeUS();
        unlock();	
		return false;
    }
    if (numData ==1) {
        *output = DEFAULT_OUTPUT;
        *outputTimeUS = timeVectorUS[0];
        unlock();	
		return false;
    }
    if (numData > NUM_FILTER_ELEMENTS) return false;  // error!! (should not happen)


    double diff = (double)(dataVector[1]) - (double)(dataVector[0]);
    long timeDiffUS = (long)(timeVectorUS[1]) - (long)(timeVectorUS[0]); 
    double velocity_DataUnitsPerSec = diff/(((double)timeDiffUS)/(1000*1000));

    //mprintf("Eye status velocity data[1] [0] = %f %f", (double)(dataVector[1]), (double)(dataVector[0]));
    //mprintf("Eye status velocity time diff = %d us", timeDiffUS);
    //mprintf("Eye status velocity velocity_DataUnitsPerSec = %f", velocity_DataUnitsPerSec);

    *output = velocity_DataUnitsPerSec;
    if (lagCompensate) {
        *outputTimeUS = timeVectorUS[0] + (timeDiffUS/2);      
    }
    else {
        *outputTimeUS = timeVectorUS[NUM_FILTER_ELEMENTS-1];
    }
    unlock();
    return true;

}
    
        

// -------------


Filter_BoxcarFilter1D::Filter_BoxcarFilter1D(shared_ptr<Variable> _inputVar, shared_ptr<Variable> _outputVar, int _filterWidthSamples) {
    filter = new BoxcarFilter1D(_filterWidthSamples);
    this->registerInput(_inputVar);
    this->registerOutput(_outputVar);
     if (VERBOSE_FILTERS) mprintf(" ****  Filter_BoxcarFilter1D constructor called");
}    
    
Filter_BoxcarFilter1D::~Filter_BoxcarFilter1D() {
    delete filter;
}
void Filter_BoxcarFilter1D::newDataReceived(int inputIndex, const Data& data, MonkeyWorksTime timeUS) {
 
	// DDC: be careful
    //lock();
    
    // pop the value into the filter and pull a value out of the filter
    MonkeyWorksTime outputTimeUS;
    double outputValue;
    filter->input((double)data, timeUS);
    filter->output(&outputValue, &outputTimeUS);   
    
    if (VERBOSE_FILTERS) mprintf(" ****  Filter_BoxcarFilter1D newDataReceived:  input = %f  input time = %d us",
                                (double)data,(long)timeUS);
    if (VERBOSE_FILTERS) mprintf(" ****  Filter_BoxcarFilter1D newDataReceived:  output = %f output time = %d us",
                                (double)outputValue, (long)outputTimeUS);
    
    // updaate the only output variable using base class functionality    
    //mVector *output = new Vector();
    //mVector->?? = (Data)outputValue;
    //postResults(output, outputTimeUS);

    postResults(0,(Data)outputValue, outputTimeUS);
    //unlock();
    
}

shared_ptr<mw::Component> Filter_BoxcarFilter1DFactory::createObject(std::map<std::string, std::string> parameters,
													 mwComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters,
					   
					   "in1", 
					   "out1", 
					   "width_samples");
	
	shared_ptr<Variable> in1 = reg->getVariable(parameters.find("in1")->second);	
	shared_ptr<Variable> out1 = reg->getVariable(parameters.find("out1")->second);	
	unsigned int width_samples = 0;
	try {
		width_samples = boost::lexical_cast< unsigned int >(parameters.find("width_samples")->second);
	} catch(bad_lexical_cast &) {
		throw InvalidReferenceException(parameters.find("reference_id")->second, "width_samples", parameters.find("width_samples")->second);
	}
	
	checkAttribute(in1, parameters.find("reference_id")->second, "in1", parameters.find("in1")->second);
	checkAttribute(out1, parameters.find("reference_id")->second, "out1", parameters.find("out1")->second);
	
	shared_ptr <mw::Component> newFilter_BoxcarFilter1D = shared_ptr<mw::Component>(new Filter_BoxcarFilter1D(in1, out1, width_samples));
	return newFilter_BoxcarFilter1D;
}

Filter_LinearFilter1D::Filter_LinearFilter1D(shared_ptr<Variable> _inputVar, shared_ptr<Variable> _outputVar, double _gain, double _offset, double _noiseSD) {
    filter = new LinearFilter1D(_gain, _offset, _noiseSD);
    this->registerInput(_inputVar);
    this->registerOutput(_outputVar);
    if (VERBOSE_FILTERS) mprintf(" ****   Filter_mLinearFilter1D constructor called");
}    
    
Filter_LinearFilter1D::~Filter_LinearFilter1D() {
    delete filter;
}
void Filter_LinearFilter1D::newDataReceived(int inputIndex, const Data& data, MonkeyWorksTime timeUS) {
	
	// DDC: be careful
    //lock();
    
	// pop the value into the filter and pull a value out of the filter
    MonkeyWorksTime outputTimeUS;
    double outputValue;
    filter->input((double)data, timeUS);
    filter->output(&outputValue, &outputTimeUS);   
    
    if (VERBOSE_FILTERS) mprintf(" ****  Filter_mLinearFilter1D newDataReceived:  input = %f  input time = %d us",
                                (double)data,(long)timeUS);
    if (VERBOSE_FILTERS) mprintf(" ****  Filter_mLinearFilter1D newDataReceived:  output = %f output time = %d us",
                                (double)outputValue, (long)outputTimeUS);

    postResults(0,(Data)outputValue, outputTimeUS);
    //unlock();
    
}

shared_ptr<mw::Component> Filter_LinearFilter1DFactory::createObject(std::map<std::string, std::string> parameters,
															mwComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters,
					   
					   "in1", 
					   "out1", 
					   "gain",
					   "offset",
					   "noise_sd");
	
	shared_ptr<Variable> in1 = reg->getVariable(parameters.find("in1")->second);	
	shared_ptr<Variable> out1 = reg->getVariable(parameters.find("out1")->second);	
	double gain = 0;
	try {
		gain = boost::lexical_cast<double>(parameters.find("gain")->second);
	} catch(bad_lexical_cast &) {
		throw InvalidReferenceException(parameters.find("reference_id")->second, "gain", parameters.find("gain")->second);
	}
	
	double offset = 0;
	try {
		offset = boost::lexical_cast<double>(parameters.find("offset")->second);
	} catch(bad_lexical_cast &) {
		throw InvalidReferenceException(parameters.find("reference_id")->second, "offset", parameters.find("offset")->second);
	}
	
	double noise_sd = 0;
	try {
		noise_sd = boost::lexical_cast<double>(parameters.find("noise_sd")->second);
	} catch(bad_lexical_cast &) {
		throw InvalidReferenceException(parameters.find("reference_id")->second, "noise_sd", parameters.find("noise_sd")->second);
	}
	
	checkAttribute(in1, parameters.find("reference_id")->second, "in1", parameters.find("in1")->second);
	checkAttribute(out1, parameters.find("reference_id")->second, "out1", parameters.find("out1")->second);
	
	shared_ptr <mw::Component> newFilter_LinearFilter1D = shared_ptr<mw::Component>(new Filter_LinearFilter1D(in1, out1, gain, offset, noise_sd));
	return newFilter_LinearFilter1D;
}




