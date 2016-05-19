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
    static const std::string DELAY;
    static const std::string DURATION;
    static const std::string REPEATS;
    static const std::string CANCEL;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit ScheduledActions(const ParameterValueMap &parameters);
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
    void init();
    void executeOnce();
    
    const boost::shared_ptr<Clock> clock;
    
    const boost::shared_ptr<Variable> delay;
    const boost::shared_ptr<Variable> n_repeats;
    const boost::shared_ptr<Variable> interval;
    boost::shared_ptr<Variable> cancel;
    
    boost::shared_ptr<VariableCallbackNotification> cancelCallbackNotification;
    std::vector<boost::shared_ptr<Action>> action_list;
    
    boost::shared_ptr<ScheduleTask> node;
    MWTime timeScheduled;
    MWTime scheduledDelay;
    MWTime scheduledInterval;
    unsigned int nRepeated;
    
};


END_NAMESPACE_MW


#endif



























