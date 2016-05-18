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


ScheduledActions::ScheduledActions(const boost::shared_ptr<Variable> &n_repeats,
                                   const boost::shared_ptr<Variable> &delay,
                                   const boost::shared_ptr<Variable> &interval,
                                   const boost::shared_ptr<Variable> &cancel) :
    delay(delay),
    n_repeats(n_repeats),
    interval(interval),
    cancel(cancel),
    clock(Clock::instance())
{
    setName("ScheduledActions");
    
    if (cancel) {
        auto callback = [this](const Datum &data, MWTime time) {
            if (node && data.getBool()) {
                node->cancel();
            }
        };
        cancelCallbackNotification = boost::make_shared<VariableCallbackNotification>(callback);
        cancel->addNotification(cancelCallbackNotification);
    }
}


ScheduledActions::~ScheduledActions() {
    if (cancelCallbackNotification) {
        cancelCallbackNotification->remove();
    }
}


void ScheduledActions::addAction(const boost::shared_ptr<Action> &action) {
    action_list.push_back(action);
}


void ScheduledActions::addChild(std::map<std::string, std::string> parameters,
                                ComponentRegistry *reg,
                                boost::shared_ptr<mw::Component> child)
{
    auto act = boost::dynamic_pointer_cast<Action>(child);
    if (!act) {
        throw SimpleException("Attempting to add illegal action (" + child->getTag() + ") to scheduled action (" + this->getTag() + ")");
    }
    addAction(act);
    act->setParent(component_shared_from_this<State>());
}


bool ScheduledActions::execute() {
    if (node) {
        node->cancel();
    }
    
    timeScheduled = clock->getCurrentTimeUS();
    scheduledDelay = MWTime(*delay);
    scheduledInterval = MWTime(*interval);
    nRepeated = 0;
    
    boost::weak_ptr<ScheduledActions> weakThis(component_shared_from_this<ScheduledActions>());
    node = Scheduler::instance()->scheduleUS(FILELINE,
                                             scheduledDelay,
                                             scheduledInterval,
                                             int(*n_repeats),
                                             [weakThis]() {
                                                 if (auto sharedThis = weakThis.lock()) {
                                                     sharedThis->executeOnce();
                                                 }
                                                 return nullptr;
                                             },
                                             M_DEFAULT_PRIORITY,
                                             M_DEFAULT_WARN_SLOP_US,
                                             M_DEFAULT_FAIL_SLOP_US,
                                             M_MISSED_EXECUTION_CATCH_UP);
    
    return true;
}


void ScheduledActions::executeOnce() {
    MWTime delta = clock->getCurrentTimeUS() - ((timeScheduled + scheduledDelay) + nRepeated * scheduledInterval);
    
    if (delta > 2000) {
        merror(M_SCHEDULER_MESSAGE_DOMAIN, "Scheduled action is starting %lldus behind intended schedule", delta);
    } else if (delta > 500) {
        mwarning(M_SCHEDULER_MESSAGE_DOMAIN, "Scheduled action is starting %lldus behind intended schedule", delta);
    }
    
    for (auto &action : action_list) {
        action->announceEntry();
        action->execute();
    }
    
    nRepeated++;
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
    
    return boost::make_shared<ScheduledActions>(repeats, delay, duration, cancel);
}


END_NAMESPACE_MW



























