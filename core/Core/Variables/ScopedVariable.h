/*
 *  ScopedVariable.h
 *  MWorksCore
 *
 *  Created by David Cox on 3/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef	SCOPED_VARIABLE_H_
#define SCOPED_VARIABLE_H_

#include "GenericVariable.h"


BEGIN_NAMESPACE_MW


class ScopedVariableEnvironment;


class ScopedVariable : public Variable {
    
private:
    boost::weak_ptr<ScopedVariableEnvironment> weakEnvironment;  // delegate for get and set methods
    int context_index;
    
public:
    explicit ScopedVariable(const VariableProperties &properties);
    
    boost::shared_ptr<ScopedVariableEnvironment> getEnvironment() const { return weakEnvironment.lock(); }
    void setEnvironment(const boost::shared_ptr<ScopedVariableEnvironment> &env) { weakEnvironment = env; }
    
    int getContextIndex() const { return context_index; }
    void setContextIndex(int i) { context_index = i; }
    
    // The scoped variable delegates its get and set methods
    Datum getValue() override;
    void setSilentValue(Datum _value, MWTime _when) override;
    void setSilentValue(const std::vector<Datum> &indexOrKeyPath, Datum value, MWTime when) override;
    
    bool isWritable() const override { return true; }
    
};


END_NAMESPACE_MW


#endif
