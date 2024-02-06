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


class ScopedVariable : public ReadWriteVariable {
    
public:
    explicit ScopedVariable(const VariableProperties &properties);
    
    boost::shared_ptr<ScopedVariableEnvironment> getEnvironment() const { return weakEnvironment.lock(); }
    void setEnvironment(const boost::shared_ptr<ScopedVariableEnvironment> &env) { weakEnvironment = env; }
    
    int getContextIndex() const { return context_index; }
    void setContextIndex(int i) { context_index = i; }
    
    Datum getValue() override;
    void setValue(const Datum &value, MWTime when, bool silent) override;
    void setValue(const std::vector<Datum> &indexOrKeyPath, const Datum &value, MWTime when, bool silent) override;
    
private:
    boost::weak_ptr<ScopedVariableEnvironment> weakEnvironment;
    int context_index;
    
};


END_NAMESPACE_MW


#endif
