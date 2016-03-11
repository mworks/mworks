/*
 *  PulseAction.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 11/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "PulseAction.h"


BEGIN_NAMESPACE_MW


const std::string Pulse::VARIABLE("variable");
const std::string Pulse::DURATION("duration");


void Pulse::describeComponent(ComponentInfo &info) {
    Action::describeComponent(info);
    info.setSignature("action/pulse");
    info.addParameter(VARIABLE);
    info.addParameter(DURATION);
}


Pulse::Pulse(const ParameterValueMap &parameters) :
    Action(parameters),
    var(parameters[VARIABLE]),
    duration(parameters[DURATION])
{
    setName("Pulse");
}


bool Pulse::execute() {
    var->setValue(Datum(1));
    
    boost::weak_ptr<Variable> weakVar(var);
    task = Scheduler::instance()->scheduleUS(FILELINE,
                                             MWTime(duration->getValue()),
                                             0,
                                             1,
                                             [weakVar]() {
                                                 if (auto sharedVar = weakVar.lock()) {
                                                     sharedVar->setValue(Datum(0));
                                                 }
                                                 return nullptr;
                                             },
                                             M_DEFAULT_PRIORITY,
                                             M_DEFAULT_WARN_SLOP_US,
                                             M_DEFAULT_FAIL_SLOP_US,
                                             M_MISSED_EXECUTION_CATCH_UP);
    
    return true;
}


END_NAMESPACE_MW



























