/*
 *  CancelScheduledActionsAction.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 11/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef CANCEL_SCHEDULED_ACTIONS_ACTION_H
#define CANCEL_SCHEDULED_ACTIONS_ACTION_H

#include "ScheduledActions.h"
namespace mw {
class CancelScheduledAction : public Action {
protected: 
	shared_ptr<ScheduledActions> to_cancel;
	
public:
	CancelScheduledAction(shared_ptr<ScheduledActions> _tc);
	bool execute();
};

class CancelScheduledActionFactory : public ComponentFactory{
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};	
}
#endif
