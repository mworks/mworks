/*
 *  ScheduledActions.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 11/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "ScheduledActions.h"


BEGIN_NAMESPACE_MW


const std::string ScheduledActions::DELAY{"delay"};
const std::string ScheduledActions::DURATION("duration");
const std::string ScheduledActions::REPEATS("repeats");
const std::string ScheduledActions::CANCEL("cancel");


void ScheduledActions::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    
    info.setSignature("action/schedule");
    
    info.addParameter(DELAY);
    info.addParameter(DURATION);
    info.addParameter(REPEATS);
    info.addParameter(CANCEL, "NO");
}


ScheduledActions::ScheduledActions(const ParameterValueMap &parameters) :
    Action(parameters),
    clock(Clock::instance()),
    delay(parameters[DELAY]),
    n_repeats(parameters[REPEATS]),
    interval(parameters[DURATION])
{
    if (!parameters[CANCEL].empty()) {
        cancel = VariablePtr(parameters[CANCEL]);
    }
    
    init();
}


ScheduledActions::ScheduledActions(const boost::shared_ptr<Variable> &n_repeats,
                                   const boost::shared_ptr<Variable> &delay,
                                   const boost::shared_ptr<Variable> &interval,
                                   const boost::shared_ptr<Variable> &cancel) :
    clock(Clock::instance()),
    delay(delay),
    n_repeats(n_repeats),
    interval(interval),
    cancel(cancel)
{
    init();
}


void ScheduledActions::init() {
    // Add state_system_mode callback
    {
        auto callback = [this](const Datum &data, MWTime time) {
            if (node && data.getInteger() == IDLE) {
                node->cancel();
            }
        };
        stateSystemCallbackNotification = boost::make_shared<VariableCallbackNotification>(callback);
        state_system_mode->addNotification(stateSystemCallbackNotification);
    }
    
    // Add cancel callback
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
    stateSystemCallbackNotification->remove();
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
    auto stateSystem = StateSystem::instance();
    for (auto &action : action_list) {
        stateSystem->executeAction(action);
    }
    
    nRepeated++;
}


END_NAMESPACE_MW
