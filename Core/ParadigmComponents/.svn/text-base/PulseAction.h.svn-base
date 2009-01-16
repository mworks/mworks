/*
 *  PulseAction.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 11/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef PULSE_ACTION_H
#define PULSE_ACTION_H

#include "ScheduledActions.h"
namespace mw {
class Pulse: public Action {
protected:
	shared_ptr<Variable> var;
	shared_ptr<ScheduledActions> actions;
	shared_ptr<Assignment> assignment;
	shared_ptr<ConstantVariable> one_variable; 
	shared_ptr<ConstantVariable> zero_variable;
	
	
public:
	Pulse(shared_ptr<Variable> _var, shared_ptr<Variable> _duration);
	~Pulse();
	virtual bool execute();
};	

class PulseFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};
}

#endif
