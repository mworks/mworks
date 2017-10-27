//
//  VariableAssignment.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 10/27/17.
//

#include "VariableAssignment.hpp"


BEGIN_NAMESPACE_MW


VariableAssignment::VariableAssignment(const std::string &varStr, ComponentRegistryPtr registry) {
    std::string varName;
    
    if (varStr.find_first_of('[') == std::string::npos) {
        varName = varStr;
    } else {
        indexExprs = ParsedExpressionVariable::parseVarnameWithSubscripts(varStr, varName);
    }
    
    var = registry->getVariable(varName);
    if (!var->isWritable()) {
        throw ComponentFactoryException("Assignment target is not writable");
    }
    
    if (!indexExprs.empty()) {
        // Try evaluating the index expressions, hopefully catching any critical errors
        std::vector<Datum> indexOrKeyPath;
        ParsedExpressionVariable::evaluateParseTreeList(indexExprs, indexOrKeyPath);
    }
}


void VariableAssignment::assign(const Datum &value) const {
    if (indexExprs.empty()) {
        var->setValue(value);
    } else {
        std::vector<Datum> indexOrKeyPath;
        ParsedExpressionVariable::evaluateParseTreeList(indexExprs, indexOrKeyPath);
        var->setValue(indexOrKeyPath, value);
    }
}


END_NAMESPACE_MW




















