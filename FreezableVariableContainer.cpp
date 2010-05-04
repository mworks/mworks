/*
 *  FreezableVariable.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 4/13/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#include "FreezableVariableContainer.h"

using namespace mw;

FreezableVariableContainer::FreezableVariableContainer(shared_ptr<Variable> _var){
    variable = _var;
}

FreezableVariableContainer::FreezableVariableContainer(const FreezableVariableContainer& tocopy){
    variable = tocopy.variable;
}

Variable * FreezableVariableContainer::clone(){
    return (Variable *)new FreezableVariableContainer(*this);
}

void FreezableVariableContainer::freeze(bool should_freeze){
    
    boost::mutex::scoped_lock(lock);
    
    frozen = should_freeze;
    
    if(frozen){
        frozen_value = variable->getValue();
    }
}


void FreezableVariableContainer::thaw(){
    freeze(false);
}

Datum FreezableVariableContainer::getValue(){
    boost::mutex::scoped_lock(lock);
    
    if(frozen){
        return frozen_value;
    } else {
        // the contained variable may have it own
        // mutex/locking, but this is not a problem
        return variable->getValue();
    }
}

// Pass-through methods
void FreezableVariableContainer::setValue(Datum value){
    // this can rely on the contained object's locking
    // for thread safety
    variable->setValue(value);
}

void FreezableVariableContainer::setValue(Datum value, MWTime time){
    // this can rely on the contained object's locking
    // for thread safety
    variable->setValue(value, time);
}

void FreezableVariableContainer::setSilentValue(Datum value){
    // this can rely on the contained object's locking
    // for thread safety
    variable->setSilentValue(value);
}