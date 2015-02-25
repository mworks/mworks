/*
 *  ScheduledActions.h
 *  MWorksCore
 *
 *  Created by bkennedy on 11/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef SCHEDULED_ACTIONS_H
#define SCHEDULED_ACTIONS_H

#include "TrialBuildingBlocks.h"
#include "boost/enable_shared_from_this.hpp"


BEGIN_NAMESPACE_MW


	class ScheduledActions : public Action {//, public enable_shared_from_this<ScheduledActions> {
	protected:
		ExpandableList<Action> action_list;
		shared_ptr<Variable> delay;
		shared_ptr<Variable> n_repeats;
		shared_ptr<Variable> interval;
		shared_ptr<Variable> cancel;
		
		shared_ptr<ScheduleTask> node;
		MWTime timeScheduled;
		unsigned int nRepeated;
		
	public:
		ScheduledActions(shared_ptr<Variable> _n_repeats, 
                         shared_ptr<Variable> _delay, 
                         shared_ptr<Variable> _interval,
                         shared_ptr<Variable> _cancel);
		void addAction(shared_ptr<Action> _action);
		void addChild(std::map<std::string, std::string> parameters,
					  ComponentRegistry *reg,
					  shared_ptr<mw::Component> child);
		virtual bool execute();
		shared_ptr<ScheduleTask> getNode();
		MWTime getDelay() const;
		MWTime getInterval() const;
		MWTime getTimeScheduled() const;
		unsigned int getNRepeated() const;
		void executeActions();
        
        class CancelNotification : public VariableNotification {
        public:
            CancelNotification(const boost::shared_ptr<ScheduledActions> &sa) :
                saWeak(sa)
            { }
            
            void notify(const Datum &data, MWTime time) override;
            
        private:
            boost::weak_ptr<ScheduledActions> saWeak;
        };
	};
	
	class ScheduledActionsFactory : public ComponentFactory{
		virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
													ComponentRegistry *reg);
	};
	
	extern void *scheduled_action_runner(const shared_ptr<ScheduledActions> &scheduled_actions);


END_NAMESPACE_MW


#endif
