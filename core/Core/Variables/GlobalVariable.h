/*
 *  GlobalVariable.h
 *  MWorksCore
 *
 *  Created by David Cox on 2/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 */

#ifndef	GLOBAL_VARIABLE_H_
#define GLOBAL_VARIABLE_H_

#include "GenericVariable.h"


BEGIN_NAMESPACE_MW


class GlobalVariable : public ReadWriteVariable {
    
public:
    explicit GlobalVariable(const Datum &value, const VariableProperties &properties = VariableProperties());
    explicit GlobalVariable(const VariableProperties &properties);
    
    Datum getValue() override;
    void setValue(const Datum &value, MWTime when, bool silent) override;
    void setValue(const std::vector<Datum> &indexOrKeyPath, const Datum &value, MWTime when, bool silent) override;
    
private:
    Datum value;
    
};


END_NAMESPACE_MW


#endif
