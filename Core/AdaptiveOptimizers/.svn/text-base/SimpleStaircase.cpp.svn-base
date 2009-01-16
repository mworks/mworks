/*
 *  SimpleStaircase.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 2/28/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SimpleStaircase.h"
#include "ComponentRegistry_new.h"
#include <boost/lexical_cast.hpp>
using namespace mw;

SimpleStaircaseOptimizer::SimpleStaircaseOptimizer(shared_ptr<Variable> _watch,
													 shared_ptr<Variable> _output,
													 shared_ptr<Variable> _lower_limit,
													 shared_ptr<Variable> _upper_limit,
													 shared_ptr<Variable> _step_size,
													 shared_ptr<Variable> _up_criterion,
													 shared_ptr<Variable> _down_criterion,
													 int _grace_period,
													 int _history){
	
	
	watch = _watch;
	output = _output;
	lower_limit = _lower_limit;
	upper_limit = _upper_limit;
	step_size = _step_size;
	history = _history;
	up_criterion = _up_criterion;
	down_criterion = _down_criterion;
	clear_history_after_step = true;
	
	if(_grace_period > history) {
		grace_period = _grace_period;
	} else {
		grace_period = _history;
	}
	
	
    count = 0;
	
	registerInput(watch);
	registerOutput(output);
    
    averager = new BoxcarFilter1D(history, false);
    
    // Enforce the limits to start 
    if((double)(*output) > (double)(*upper_limit)){
        postResults(0, (Data)((double)*upper_limit));
    }

    if((double)(*output) < (double)(*lower_limit)){
        postResults(0, (Data)((double)*lower_limit));
    }
    	
}


void SimpleStaircaseOptimizer::stepUp(){
	double stepped_up = (double)(*output) + (double)(*step_size);
	if((double)(*upper_limit) >= stepped_up && (double)(*lower_limit) <= stepped_up){
		postResults(0, (Data)stepped_up);
	} else {
		if((double)(*upper_limit) < stepped_up) {
			postResults(0, (Data)((double)*upper_limit));
		} else if((double)(*lower_limit) > stepped_up) {
			postResults(0, (Data)((double)*lower_limit));
		}
	}
	
	
	if(clear_history_after_step){
		averager->reset();
		count = 0;
	}
}

void SimpleStaircaseOptimizer::stepDown(){
	double stepped_down = (double)(*output) - (double)(*step_size);
	if((double)(*upper_limit) >= stepped_down && (double)(*lower_limit) <= stepped_down){
		postResults(0, (Data)stepped_down);
	} else {
		if((double)(*upper_limit) < stepped_down) {
			postResults(0, (Data)((double)*upper_limit));
		} else if((double)(*lower_limit) > stepped_down) {
			postResults(0, (Data)((double)*lower_limit));
		}
	}
	
	if(clear_history_after_step){
		averager->reset();
		count = 0;
	}
}

void SimpleStaircaseOptimizer::newDataReceived(int inputIndex, const Data& data, 
                                                MonkeyWorksTime timeUS){
	
	if(inputIndex != 0){
		// error
		return;
	}
	
	count++;
	
	averager->input(data);
	
    if(count < grace_period){
		return;
	}
    
	double val;
	averager->output(&val);
	    
	if(val >= (double)*up_criterion){
		stepUp();
	} else if (val <= (double)*down_criterion){
		stepDown();
	}
}

void SimpleStaircaseOptimizer::reset(){
	averager->reset();
	count = 0;
}

shared_ptr<mw::Component> SimpleStaircaseOptimizerFactory::createObject(std::map<std::string, std::string> parameters,
															mwComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters,
					   "watch",
					   "output",
					   "lower_limit",
					   "upper_limit",
					   "up_criterion",
					   "down_criterion",
					   "history",
					   "grace_period",
					   "step_size");
	
	shared_ptr <Variable> watch = reg->getVariable(parameters.find("watch")->second);                                                 
	shared_ptr <Variable> output = reg->getVariable(parameters.find("output")->second);                                                 
	shared_ptr <Variable> lower_limit = reg->getVariable(parameters.find("lower_limit")->second);                                       
	shared_ptr <Variable> upper_limit = reg->getVariable(parameters.find("upper_limit")->second);                                       
	shared_ptr <Variable> up_criterion = reg->getVariable(parameters.find("up_criterion")->second);                                     
	shared_ptr <Variable> down_criterion = reg->getVariable(parameters.find("down_criterion")->second);                                 
	shared_ptr <Variable> step_size = reg->getVariable(parameters.find("step_size")->second);                                           
	
	unsigned int grace_period = 0;
	try {
		grace_period = boost::lexical_cast< unsigned int >(parameters.find("grace_period")->second);
	} catch(bad_lexical_cast &) {
		throw InvalidReferenceException(parameters.find("reference_id")->second, "grace_period", parameters.find("grace_period")->second);
	}
	
	unsigned int history = 0;
	try {
		history = boost::lexical_cast< unsigned int >(parameters.find("history")->second);
	} catch(bad_lexical_cast &) {
		throw InvalidReferenceException(parameters.find("reference_id")->second, "history", parameters.find("history")->second);
	}
	
	checkAttribute(watch, parameters.find("reference_id")->second, "watch", parameters.find("watch")->second);                                                        
	checkAttribute(output, parameters.find("reference_id")->second, "output", parameters.find("output")->second);                                                        
	checkAttribute(lower_limit, parameters.find("reference_id")->second, "lower_limit", parameters.find("lower_limit")->second);                                         
	checkAttribute(upper_limit, parameters.find("reference_id")->second, "upper_limit", parameters.find("upper_limit")->second);                                         
	checkAttribute(up_criterion, parameters.find("reference_id")->second, "up_criterion", parameters.find("up_criterion")->second);                                      
	checkAttribute(down_criterion, parameters.find("reference_id")->second, "down_criterion", parameters.find("down_criterion")->second);                                
	checkAttribute(step_size, parameters.find("reference_id")->second, "step_size", parameters.find("step_size")->second);                                               
	
	shared_ptr <mw::Component> newSimpleStaircaseOptimizer = shared_ptr<mw::Component>(new SimpleStaircaseOptimizer(watch, 
																											   output,
																											   lower_limit,
																											   upper_limit,
																											   step_size,
																											   up_criterion,
																											   down_criterion,
																											   grace_period,
																											   history));
	return newSimpleStaircaseOptimizer;
}
