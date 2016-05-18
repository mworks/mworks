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


BEGIN_NAMESPACE_MW


class ScheduledActions : public Action, boost::noncopyable {
    
public:
    ScheduledActions(const boost::shared_ptr<Variable> &n_repeats,
                     const boost::shared_ptr<Variable> &delay,
                     const boost::shared_ptr<Variable> &interval,
                     const boost::shared_ptr<Variable> &cancel);
    
    ~ScheduledActions();
    
    void addAction(const boost::shared_ptr<Action> &action);
    void addChild(std::map<std::string, std::string> parameters,
                  ComponentRegistry *reg,
                  boost::shared_ptr<mw::Component> child) override;
    bool execute() override;
    
private:
    void executeOnce();
    
    const boost::shared_ptr<Variable> delay;
    const boost::shared_ptr<Variable> n_repeats;
    const boost::shared_ptr<Variable> interval;
    const boost::shared_ptr<Variable> cancel;
    
    const boost::shared_ptr<Clock> clock;
    boost::shared_ptr<VariableCallbackNotification> cancelCallbackNotification;
    std::vector<boost::shared_ptr<Action>> action_list;
    
    boost::shared_ptr<ScheduleTask> node;
    MWTime timeScheduled;
    MWTime scheduledDelay;
    MWTime scheduledInterval;
    unsigned int nRepeated;
    
};


class ScheduledActionsFactory : public ComponentFactory{
    virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                   ComponentRegistry *reg);
};


END_NAMESPACE_MW


#endif



























