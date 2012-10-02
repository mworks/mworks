/*
 *  ExpressionVariable.h
 *  MWorksCore
 *
 *  Created by David Cox on 2/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 *
 *	The expression variable object hierarchically represents arbitrary
 *  expressions made up of other variable objects (which might be "regular"
 *  variables, constants, or other expressions).  The base Variable
 *  object overrides arithmetic operators to generate ExpressionVariable
 *  objects, and one can create arbitrarily complex trees of variables
 *  and operation by simply performing arithmetic on the variables themselves
 *  (e.g. "a + 4 / b" would evaluate to two layer hierarchy of variable
 *  objects contained within an expression variable, and this composite
 *  variable would evaluate to the current value of the expression when 
 *  cast to a built-in type or evaluated with the "get" method.  Since this
 *  operator overloading can be exposed in scripting targets as well, very 
 *  expression creation can be done within the XML parser in an almost 
 *  entirely seemless way, with no explicit "expression parsing" code required
 */
 
#ifndef	EXPRESSION_VARIABLE_H_
#define	EXPRESSION_VARIABLE_H_

#include <boost/algorithm/string/trim.hpp>
 
#include "GenericVariable.h"
#include "VariableRegistry.h"
#include "ExpressionParser.h"


BEGIN_NAMESPACE_MW


class ExpressionVariable : public Variable {
    
protected:
	
    Variable *v1;
    Variable *v2;
	
    Operator op;
	
public:
	
    ExpressionVariable(Variable *_v1 = NULL, Variable *_v2 = NULL, Operator _op = M_PLUS);
    
    Operator getOperator() { return op; };
    
    virtual Datum getValue(){
        return getExpressionValue();
    }
    
    virtual void setValue(Datum val){ return; }
    virtual void setValue(Datum val, MWTime time){ return; }
    virtual void setSilentValue(Datum _value){ return; }
    
    // TODO: remove
    Datum getExpressionValue();
    
    Variable *getFirstOperand(){ return v1; }
    Variable *getSecondOperand(){ return v2; }
    
    
    /**
     *  A polymorphic copy constructor (inherited from Clonable)
     */
    virtual Variable *clone(){
        ExpressionVariable *returned = new ExpressionVariable((const ExpressionVariable&)(*this));
        return (Variable *)returned;
    }
    
};


class ParsedExpressionVariable : public Variable {
    
protected:
    const stx::ParseTree expression_tree;
    
public:
    static stx::ParseTree parseExpression(const std::string &expr) {
        try {
            return stx::parseExpression(boost::algorithm::trim_copy(expr));
        } catch (stx::ExpressionParserException &e) {
            throw FatalParserException("Expression parser error", e.what());
        }
    }
    
    static stx::AnyScalar evaluateParseTree(const stx::ParseTree &tree) {
        if (tree.isEmpty()) {
            throw FatalParserException("Internal error", "Expression parse tree is empty");
        }
        
        try {
            return tree.evaluate((stx::SymbolTable&)(*(global_variable_registry.get())));
        } catch (stx::ExpressionParserException &e){
            throw FatalParserException("Expression parser error", e.what());
        }
    }
    
    static stx::AnyScalar evaluateExpression(const std::string &expr) {
        return evaluateParseTree(parseExpression(expr));
    }
    
    static stx::ParseTreeList parseExpressionList(const std::string &exprList) {
        try {
            return stx::parseExpressionList(boost::algorithm::trim_copy(exprList));
        } catch (stx::ExpressionParserException &e) {
            throw FatalParserException("Expression parser error", e.what());
        }
    }
    
    static void evaluateParseTreeList(const stx::ParseTreeList &treeList, std::vector<stx::AnyScalar> &values) {
        try {
            treeList.evaluate(values, (stx::SymbolTable&)(*(global_variable_registry.get())));
        } catch (stx::ExpressionParserException &e){
            throw FatalParserException("Expression parser error", e.what());
        }
    }
    
    static void evaluateExpressionList(const std::string &exprList, std::vector<stx::AnyScalar> &values) {
        evaluateParseTreeList(parseExpressionList(exprList), values);
    }
	
    ParsedExpressionVariable(const std::string &expression_string) :
        Variable(),
        expression_tree(parseExpression(expression_string))
    {
        getValue();
    }
	
    ParsedExpressionVariable(const stx::ParseTree &expression_tree) :
        Variable(),
        expression_tree(expression_tree)
    {
        getValue();
    }
    
    virtual Datum getValue() {
        return Datum(evaluateParseTree(expression_tree));
    }
    
    virtual void setValue(Datum val){ return; }
    virtual void setValue(Datum val, MWTime time){ return; }
    virtual void setSilentValue(Datum _value){ return; }
    
    /**
     *  A polymorphic copy constructor (inherited from Clonable)
     */
    virtual Variable *clone(){
        ParsedExpressionVariable *returned = 
        new ParsedExpressionVariable((const ParsedExpressionVariable&)(*this));
        return (Variable *)returned;
    }
    
};


END_NAMESPACE_MW


#endif

