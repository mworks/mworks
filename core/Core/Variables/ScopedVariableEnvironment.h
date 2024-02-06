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
    
protected:
    const std::vector<boost::shared_ptr<ScopedVariable>> & getVariables() const { return variables; }
    
public:
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
    void setValue(int index, const Datum &value);
    
    // Announcing
    void announceAll();
    
};


END_NAMESPACE_MW


#endif
