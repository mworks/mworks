/*
 *  CancelScheduledActionsAction.cpp
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 11/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "CancelScheduledActionsAction.h"
using namespace mw;

CancelScheduledAction::CancelScheduledAction(shared_ptr<ScheduledActions> _tc){
	to_cancel = _tc;
}

bool CancelScheduledAction::execute(){
	if(to_cancel == NULL){
		mwarning(M_PARADIGM_MESSAGE_DOMAIN,"Cannot cancel NULL action");
		return false;
		setName("CancelScheduledAction");
	}
	
	shared_ptr<ScheduleTask> node = to_cancel->getNode();
	if(node != NULL){
		node->cancel();
	}
	
	return true;
}


shared_ptr<mw::Component> CancelScheduledActionFactory::createObject(std::map<std::string, std::string> parameters,
																   mwComponentRegistry *reg) {		
	
	// TODO is "action" correct?
	REQUIRE_ATTRIBUTES(parameters, "action");
	
	shared_ptr<ScheduledActions> sa = reg->getObject<ScheduledActions>(parameters.find("action")->second);
	
	checkAttribute(sa, parameters.find("reference_id")->second, "action", parameters.find("action")->second);
	
	
	shared_ptr <mw::Component> newCancelScheduledAction = shared_ptr<mw::Component>(new CancelScheduledAction(sa));
	return newCancelScheduledAction;		
}


