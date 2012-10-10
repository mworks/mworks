/*
 *  ScheduledActions.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 11/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ScheduledActions.h"
#include "boost/bind.hpp"


BEGIN_NAMESPACE_MW


ScheduledActions::ScheduledActions(shared_ptr<Variable> _n_repeats, 
                                   shared_ptr<Variable> _delay, 
                                   shared_ptr<Variable> _interval,
                                   shared_ptr<Variable> _cancel)
{						  
	n_repeats = _n_repeats;
	delay = _delay;
	interval = _interval;
    cancel = _cancel;
	setName("ScheduledActions");
}

void ScheduledActions::addAction(shared_ptr<Action> _action){
	action_list.addReference(_action);
}

void ScheduledActions::addChild(std::map<std::string, std::string> parameters,
								 ComponentRegistry *reg,
								 shared_ptr<mw::Component> child){
	
	shared_ptr<Action> act = boost::dynamic_pointer_cast<Action,mw::Component>(child);
	if(act == 0) {
		throw SimpleException("Attempting to add illegal action (" + child->getTag() + ") to scheduled action (" + this->getTag() + ")");
	}
	addAction(act);
	act->setParent(component_shared_from_this<State>());
}

bool ScheduledActions::execute(){
	shared_ptr <Clock> clock = Clock::instance();
	timeScheduled = clock->getCurrentTimeUS();
	nRepeated = 0;
	
	shared_ptr<Scheduler> scheduler = Scheduler::instance();	
	shared_ptr<ScheduledActions> this_one = component_shared_from_this<ScheduledActions>();
	node = scheduler->scheduleUS(FILELINE, 
								 (MWTime)(*delay),
								 (MWTime)(*interval),
								 (long)(*n_repeats),
								 boost::bind(scheduled_action_runner, this_one),
								 M_DEFAULT_PRIORITY,
								 M_DEFAULT_WARN_SLOP_US,
								 M_DEFAULT_FAIL_SLOP_US,
								 M_MISSED_EXECUTION_CATCH_UP);
	return true;
}

shared_ptr<ScheduleTask> ScheduledActions::getNode(){ 
	return node; 
}

MWTime ScheduledActions::getDelay() const {
	return (MWTime)(*delay);
}

MWTime ScheduledActions::getInterval() const {
	return (MWTime)(*interval);
}

MWTime ScheduledActions::getTimeScheduled() const {
	return timeScheduled;
}

unsigned int ScheduledActions::getNRepeated() const {
	return nRepeated;
}

bool ScheduledActions::shouldCancel() const {
    return bool(*cancel);
}

void ScheduledActions::executeActions() {
	for(int i = 0; i < action_list.getNElements(); i++){
		action_list[i]->announceEntry();
		action_list[i]->execute();
	}
	++nRepeated;
}

/****************************************************************
 *                       ScheduledActions Methods
 ****************************************************************/

	void *scheduled_action_runner(const shared_ptr<ScheduledActions> &sa){
        if (sa->shouldCancel()) {
            shared_ptr<ScheduleTask> node(sa->getNode());
            if (node) {
                node->cancel();
                return NULL;
            }
        }
        
		shared_ptr <Clock> clock = Clock::instance();
		MWTime delta = clock->getCurrentTimeUS() - ((sa->getTimeScheduled() + sa->getDelay()) + sa->getNRepeated()*sa->getInterval());
		
		if(delta > 2000) {
			merror(M_SCHEDULER_MESSAGE_DOMAIN, "Scheduled action is starting %lldus behind intended schedule", delta); 
		} else if (delta > 500) {
			mwarning(M_SCHEDULER_MESSAGE_DOMAIN, "Scheduled action is starting %lldus behind intended schedule", delta); 
		}
		
		sa->executeActions();
		return 0;
	}


shared_ptr<Component> ScheduledActionsFactory::createObject(std::map<std::string, std::string> parameters,
															  ComponentRegistry *reg) {
	REQUIRE_ATTRIBUTES(parameters, "delay", "repeats", "duration");
	
	shared_ptr<Variable> delay = reg->getVariable(parameters["delay"]);
	shared_ptr<Variable> duration = reg->getVariable(parameters["duration"]);
	shared_ptr<Variable> repeats = reg->getVariable(parameters["repeats"]);
	shared_ptr<Variable> cancel = reg->getVariable(parameters["cancel"], "0");
	
	checkAttribute(duration, parameters["reference_id"], "duration", parameters["duration"]);
	
	checkAttribute(delay, parameters["reference_id"], "delay", parameters["delay"]);
	
	checkAttribute(repeats, parameters["reference_id"], "repeats", parameters["repeats"]);
	
	checkAttribute(repeats, parameters["reference_id"], "cancel", parameters["cancel"]);
	
	
	// TODO .. needs more work, the actual actions aren't included here
	
	shared_ptr <Component> newScheduledActions = shared_ptr<Component>(new ScheduledActions(repeats, delay, duration, cancel));
	return newScheduledActions;		
}


END_NAMESPACE_MW
