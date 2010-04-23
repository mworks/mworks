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

namespace mw{

class FreezableVariableContainer : public Variable {
    
protected:
    
    bool frozen;
    Datum frozen_value;
    shared_ptr<Variable> variable;
    
public:
    
    
    FreezableVariableContainer(shared_ptr<Variable> _var){
        variable = _var;
    }
    
    // use default copy constructor
    
    Variable * clone(){
        return (Variable *)new FreezableVariableContainer(*this);
    }
    
    void freeze(bool should_freeze = true){
        frozen = should_freeze;
        
        if(frozen){
            frozen_value = variable->getValue();
        }
    }
    
    void thaw(){
        freeze(false);
    }
    
    Datum getValue(){
        if(frozen){
            return frozen_value;
        } else {
            return variable->getValue();
        }
    }
    
    
    // Pass-through methods
    void setValue(Datum value){
        variable->setValue(value);
    }
    
    void setValue(Datum value, MWTime time){
        variable->setValue(value, time);
    }
    
    void setSilentValue(Datum value){
        variable->setSilentValue(value);
    }
};

    
class FreezableCollection {
    
protected:
    
    vector< shared_ptr<FreezableVariableContainer> > variables;

public:
    
    shared_ptr<Variable> registerVariable(shared_ptr<Variable> var, bool check_for_duplicates = false){
        shared_ptr<FreezableVariableContainer> freezable(new FreezableVariableContainer(var));
        variables.push_back(freezable);
        
        return dynamic_pointer_cast<FreezableVariableContainer, Variable>(freezable);
    }
    
    void freeze(bool should_freeze = true){
        
        vector< shared_ptr<FreezableVariableContainer> >::iterator iter;
        for(iter = variables.begin(); iter != variables.end(); ++iter){
            if(should_freeze){
                (*iter)->freeze();
            } else {
                (*iter)->thaw();
            }
        }
    }
        
    void thaw(){
        freeze(false);
    }
    
};
    
}

#endif

