#ifndef	SIMPLE_STAIRCASE_OPTIMIZER_H_
#define SIMPLE_STAIRCASE_OPTIMIZER_H_


/*
 *  SimpleStaircase.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 2/28/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "VariableTransformAdaptors.h"
#include "FilterTransforms.h"

namespace mw {
class SimpleStaircaseOptimizer : public VarTransformAdaptor{
	
protected:
	
	shared_ptr<Variable> watch;			// variable to watch for correct/incorrect
	shared_ptr<Variable> output;			// variable to vary continuously
	shared_ptr<Variable> lower_limit;		// lower bound for output
	shared_ptr<Variable> upper_limit;		// upper bound for output
	shared_ptr<Variable> step_size;			// step size for changing output
	shared_ptr<Variable> up_criterion;	// percentage above which the var steps up
	shared_ptr<Variable> down_criterion;	// percentage below which the var steps down
	
	
	int		  history;			// how many observations of watch to keep
	int		  grace_period;		// don't step anywhere unless you've been
	// updated at least this many times
	
	int count;
	
	bool clear_history_after_step;
	
	
	BoxcarFilter1D *averager;
	
public:
	
	SimpleStaircaseOptimizer(shared_ptr<Variable> _watch,
							  shared_ptr<Variable> _output,
							  shared_ptr<Variable> _lower_limit,
							  shared_ptr<Variable> _upper_limit,
							  shared_ptr<Variable> _step_size,
							  shared_ptr<Variable> _up_criterion,
							  shared_ptr<Variable> _down_criterion,
							  int _grace_period,
							  int _history);
	
	virtual ~SimpleStaircaseOptimizer(){ }
	void stepUp();
	
	void stepDown();		
	virtual void newDataReceived(int inputIndex, const Data& data, 
								 MonkeyWorksTime timeUS);		
	virtual void reset();
	
};

class SimpleStaircaseOptimizerFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};
}
#endif
