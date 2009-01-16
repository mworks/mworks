/*
 *  BiasMonitor.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 7/21/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "BiasMonitor.h"

#include "ComponentRegistry_new.h"
#include <boost/lexical_cast.hpp>
using namespace mw;

BiasMonitor::BiasMonitor(shared_ptr<Variable> _success,
							  shared_ptr<Variable> _response_index,
							  shared_ptr<Variable> _task_correlation,
							  shared_ptr<Variable> _response_bias_correlation,
                              shared_ptr<Variable> _alternation_correlation,
							  unsigned int _grace_period,
							  unsigned int _history){
	
	success = _success;
    response_index = _response_index;
    task_correlation = _task_correlation;
    response_bias_correlation = _response_bias_correlation;
    alternation_correlation = _alternation_correlation;
    
    history = _history;
    grace_period = _grace_period;
    
	if(_grace_period > history) {
		grace_period = _grace_period;
	} else {
		grace_period = _history;
	}
	
	
    count = 0;
	
	registerInput(success);
	task_corr_index = registerOutput(task_correlation);
    response_corr_index = registerOutput(response_bias_correlation);
    alternation_corr_index = registerOutput(alternation_correlation);
    

        
}

void BiasMonitor::newDataReceived(int inputIndex, const Data& data, 
                                                MonkeyWorksTime timeUS){
	
	if(inputIndex != 0){
		// error
		return;
	}
	
	count++;
	
    success_buffer.push_back(data);
    response_index_buffer.push_back(response_index->getValue());
    
		
    if(success_buffer.size() > history){
        success_buffer.pop_front();
        response_index_buffer.pop_front();
    }
    
    if(count < grace_period){
		return;
	}
    
    
    // compute task correlation (e.g. normalized cross correlation of responses 
    // with correct answers
    
    deque<Data>::iterator success_iterator;
    deque<Data>::iterator response_iterator;
    
    double task_corr = 0.;
    double resp_corr = 0.;
    double alt_corr = 0.;
    
    success_iterator = success_buffer.begin();
    response_iterator = response_index_buffer.begin();
    
    int alternation = -1;
    while(success_iterator != success_buffer.end()){
        
        int success = (int)(*success_iterator);
        int response = (int)(*response_iterator);
        
        alternation *= -1;
        if(success == 0) success = -1;
        if(response == 0) response = -1;
        
        task_corr += success;
        resp_corr += response;
        alt_corr += alternation * response;
        
        success_iterator++;
        response_iterator++;
    }
    
    double divisor = history;
    
    postResults(task_corr_index, Data(task_corr / divisor));
    postResults(response_corr_index, Data(resp_corr / divisor));
    postResults(alternation_corr_index, Data(alt_corr / divisor));
}

void BiasMonitor::reset(){
	success_buffer.clear();
    response_index_buffer.clear();
    
	count = 0;
}

shared_ptr<mw::Component> BiasMonitorFactory::createObject(std::map<std::string, std::string> parameters,
															mwComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters,
					   "success",
						"response_index",
						"task_correlation",
						"response_bias_correlation",
                        "alternation_correlation",
						"grace_period",
						"history");
	
	shared_ptr <Variable> success = reg->getVariable(parameters.find("success")->second);                                                 
	shared_ptr <Variable> response_index = reg->getVariable(parameters.find("response_index")->second);
    shared_ptr <Variable> task_correlation = reg->getVariable(parameters.find("task_correlation")->second);  
    shared_ptr <Variable> response_bias_correlation = reg->getVariable(parameters.find("response_bias_correlation")->second);                                                 
	shared_ptr <Variable> alternation_correlation = reg->getVariable(parameters.find("alternation_correlation")->second);
    
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
	
	checkAttribute(success, parameters.find("reference_id")->second, "success", parameters.find("success")->second);
    checkAttribute(response_index, parameters.find("reference_id")->second, "response_index", parameters.find("response_index")->second);
    checkAttribute(task_correlation, parameters.find("reference_id")->second, "task_correlation", parameters.find("task_correlation")->second);
    checkAttribute(response_bias_correlation, parameters.find("reference_id")->second, "response_bias_correlation", parameters.find("response_bias_correlation")->second);
    checkAttribute(alternation_correlation, parameters.find("reference_id")->second, "alternation_correlation", parameters.find("alternation_correlation")->second);
    

	shared_ptr <mw::Component> newBiasMonitor = 
                shared_ptr<mw::Component>(new BiasMonitor(success,
                                                        response_index,
                                                        task_correlation,
                                                        response_bias_correlation,
                                                        alternation_correlation,
                                                        grace_period,
                                                        history));
                                                                                  
	return newBiasMonitor;
}
