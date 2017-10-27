//
//  VariableAssignment.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 10/27/17.
//

#ifndef VariableAssignment_hpp
#define VariableAssignment_hpp

#include "ExpressionVariable.h"
#include "ParameterValue.h"


BEGIN_NAMESPACE_MW


class VariableAssignment {
    
public:
    VariableAssignment(const std::string &varStr, ComponentRegistryPtr registry);
    
    explicit VariableAssignment(const ParameterValue &variable) :
        VariableAssignment(variable.str(), variable.getRegistry())
    { }
    
    explicit VariableAssignment(const VariablePtr &var) :
        var(var)
    { }
    
    void assign(const Datum &value) const;
    
private:
    VariablePtr var;
    stx::ParseTreeList indexExprs;

};


END_NAMESPACE_MW


#endif /* VariableAssignment_hpp */




















