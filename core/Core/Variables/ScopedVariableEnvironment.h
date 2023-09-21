/*
 *  ScopedVariableEnvironment.h
 *  MWorksCore
 *
 *  Created by David Cox on 3/26/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	VARIABLE_ENVIRONMENT_H_
#define VARIABLE_ENVIRONMENT_H_

#include "ScopedVariable.h"


BEGIN_NAMESPACE_MW


class ScopedVariableContext;


class ScopedVariableEnvironment {
    
private:
    std::vector<boost::shared_ptr<ScopedVariable>> variables;
    boost::shared_ptr<ScopedVariableContext> current_context;
    
public:
    // Contexts should be created *after* all variables are added to the environment
    static boost::shared_ptr<ScopedVariableContext> createNewDefaultContext(const boost::shared_ptr<ScopedVariableEnvironment> &env);
    
    virtual ~ScopedVariableEnvironment() { }
    
    // Add a new scoped variable to this environment
    int addVariable(const boost::shared_ptr<ScopedVariable> &var);
    
    // Set the current variable context
    void setCurrentContext(const boost::shared_ptr<ScopedVariableContext> &new_context) {
        current_context = new_context;
    }
    
    int getNVariables() const {
        return variables.size();
    }
    
    // ScopedVariable delegate methods
    Datum getValue(int index);
    void setValue(int index, Datum value);
    
    // Announcing
    void announceAll();
    
};


END_NAMESPACE_MW


#endif
