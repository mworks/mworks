/*
 *  FreezableVariable.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 4/13/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#include "FreezableVariableContainer.h"

#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


FreezableVariableContainer::FreezableVariableContainer(shared_ptr<Variable> _var){
    frozen = false;
    variable = _var;
    if(variable == NULL){
        // TODO: make a better exception here; this is only effective for debugging as stands
        throw FatalParserException("Registering an empty variable within a FreezableVariableContainer "
                                   "(is there an empty variable passed into a Stimulus object?)");
    }
}

FreezableVariableContainer::FreezableVariableContainer(const FreezableVariableContainer& tocopy){
    frozen = false;
    variable = tocopy.variable;
    frozen_value = tocopy.frozen_value;
}

void FreezableVariableContainer::freeze(bool should_freeze){
    
    boost::mutex::scoped_lock locker(lock);
    
    frozen = should_freeze;
    
    if(frozen){
        frozen_value = variable->getValue();
    }
}


void FreezableVariableContainer::thaw(){
    freeze(false);
}

Datum FreezableVariableContainer::getValue(){
    boost::mutex::scoped_lock locker(lock);
    
    if(frozen){
        return frozen_value;
    } else {
        // the contained variable may have it own
        // mutex/locking, but this is not a problem
        return variable->getValue();
    }
}

void FreezableVariableContainer::setValue(Datum value, MWTime time){
    // this can rely on the contained object's locking
    // for thread safety
    variable->setValue(value, time);
}

void FreezableVariableContainer::setSilentValue(Datum value, MWTime time) {
    // this can rely on the contained object's locking
    // for thread safety
    variable->setSilentValue(value, time);
}

bool FreezableVariableContainer::isWritable() const {
    return variable->isWritable();
}


VariablePtr FreezableCollection::registerVariable(const ParameterValue &param, bool check_for_duplicates) {
    return registerVariable(VariablePtr(param), check_for_duplicates);
}


END_NAMESPACE_MW


























