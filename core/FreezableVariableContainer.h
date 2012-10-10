/*
 *  FreezableVariable.h
 *  MWorksCore
 *
 *  Created by David Cox on 4/13/10.
 *  Copyright 2010 Harvard University. All rights reserved.
 *
 */

#ifndef FREEZABLE_VARIABLE_H_
#define FREEZABLE_VARIABLE_H_

#include "GenericVariable.h"
#include <boost/thread/mutex.hpp>

namespace mw{

// A wrapper around a variable object that supports "freezing" --
// i.e. transiently taking on a constant value
// whenever the "freeze" method is called, and then returning
// to normal when "thaw" is called.  An important use case
// for these variables is in stimulus presentation.  When a 
// stimulus is queued for display, users have an expectation
// that variables controlling its display behavior will reflect 
// the value at the time of queueing, and will not change if the
// variable's value is subsequently changed.  This is implemented
// by "freezing" the variables using the "FreezableCollection"
// class implemented below.  From the stimulus object's perspective,
// the variables are replaced by constant-valued variables.
    
// TODO: this object needs to be thread-safe
class FreezableVariableContainer : public Variable {
    
protected:
    
    bool frozen;
    Datum frozen_value;
    shared_ptr<Variable> variable;
    
    boost::mutex lock;
    
public:
    
    
    FreezableVariableContainer(shared_ptr<Variable> _var);
    FreezableVariableContainer(const FreezableVariableContainer& tocopy);
    
    Variable * clone();
    
    // If should_freeze is true, make this variable behave as 
    // if it were a constant variable
    void freeze(bool should_freeze = true);
    void thaw();
    
    // Return the wrapped variable's value if not frozen,
    // otherwise return the stored (constant) value
    Datum getValue();
    
    // Pass-through methods
    void setValue(Datum value);
    void setValue(Datum value, MWTime time);
    void setSilentValue(Datum value);

};
    

// A class for storing and mass-freezing/thawing a collection
// of variables.  Objects inheriting from this one can register
// whatever variables they like with the registerVariable method
// and can freeze/thaw this state information on demand
// All other aspects of interacting with these variables is the
// same as if they were not freezable.
    
class FreezableCollection {
    
protected:
    
    vector< shared_ptr<FreezableVariableContainer> > variables;

public:
    
    
    // Register a given variable for "freezing" (see definition above)
    // Takes any kind of variable as an argument, and then returns a "freezable"
    // version of that variable.
    virtual shared_ptr<Variable> registerVariable(shared_ptr<Variable> var, bool check_for_duplicates = false){
        shared_ptr<FreezableVariableContainer> freezable(new FreezableVariableContainer(var));
        variables.push_back(freezable);
        
        return boost::dynamic_pointer_cast<FreezableVariableContainer, Variable>(freezable);
    }
    
    // Individually "freeze" 
    virtual void freeze(bool should_freeze = true){
        
        vector< shared_ptr<FreezableVariableContainer> >::iterator iter;
        for(iter = variables.begin(); iter != variables.end(); ++iter){
            if(should_freeze){
                (*iter)->freeze();
            } else {
                (*iter)->thaw();
            }
        }
    }
        
    virtual void thaw(){
        freeze(false);
    }
    
};
    
}

#endif

