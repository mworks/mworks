/*
 *  PulseAction.h
 *  MWorksCore
 *
 *  Created by bkennedy on 11/24/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef PULSE_ACTION_H
#define PULSE_ACTION_H

#include "TrialBuildingBlocks.h"


BEGIN_NAMESPACE_MW


class Pulse: public Action {
    
public:
    static const std::string VARIABLE;
    static const std::string DURATION;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit Pulse(const ParameterValueMap &parameters);
    
    bool execute() override;
    
private:
    const VariablePtr var;
    const VariablePtr duration;
    boost::shared_ptr<ScheduleTask> task;
    
};	


END_NAMESPACE_MW


#endif



























