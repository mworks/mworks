#ifndef	BIAS_DETECTOR_H_
#define BIAS_DETECTOR_H_


/*
 *  BiasMonitor.h
 *  MonkeyWorksCore
 *
 *  Evaluates the extent to which a "biased" strategy fits available data
 *  better than either random guessing or correct responses
 *
 *  Algorithm: compute correlation with the correct sequence
 *              compute correlation with bias left, bias right
 *
 *  Created by David Cox on 2/28/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "VariableTransformAdaptors.h"
#include "FilterTransforms.h"
#include <deque>

namespace mw {

class BiasMonitor : public VarTransformAdaptor{
	
protected:
	
    // Variable to pay attention to (INPUTS)
	shared_ptr<Variable> success;     // variable to watch for correct/incorrect
	shared_ptr<Variable> response_index;   // e.g. physical response location
                                            // N.B. relevant value is when 
                                            // success changes
                                            
    // Consistency with various scenarios -1 to 1 (OUTPUTS)
    shared_ptr<Variable> task_correlation;    // to what extent are responses consistent with the task
    shared_ptr<Variable> response_bias_correlation; // to what extent are responses consistent with a response bias
    shared_ptr<Variable> alternation_correlation; // to what extent are responses consistent with alternation

    int task_corr_index;
    int response_corr_index;
    int alternation_corr_index;

	unsigned int		  history;			// how many observations of watch to keep
	unsigned int		  grace_period;		// don't do anything unless you've been
                                // updated at least this many times
	
	unsigned int count;
	
    deque<Data> success_buffer;
    deque<Data> response_index_buffer;
	
public:
	
	BiasMonitor(shared_ptr<Variable> _success,
							  shared_ptr<Variable> _response_index,
							  shared_ptr<Variable> _task_correlation,
							  shared_ptr<Variable> _response_bias_correlation,
                              shared_ptr<Variable> _alternation_correlation,
							  unsigned int _grace_period,
							  unsigned int _history);
	
	virtual ~BiasMonitor(){ }
	
    
    virtual void newDataReceived(int inputIndex, const Data& data, 
								 MonkeyWorksTime timeUS);		
	virtual void reset();
	
};

class BiasMonitorFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};
}
#endif

