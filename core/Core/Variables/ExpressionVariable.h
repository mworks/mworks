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


class ParsedExpressionVariable : public ReadOnlyVariable {
    
public:
    static stx::ParseTree parseExpression(const std::string &expr) {
        try {
            return stx::parseExpression(boost::algorithm::trim_copy(expr));
        } catch (stx::ExpressionParserException &e) {
            throw FatalParserException("Expression parser error", e.what());
        }
    }
    
    static stx::ParseTree parseUnquotedStringLiteral(const std::string &input) {
        try {
            return stx::parseUnquotedStringLiteral(input);  // Don't trim the input!
        } catch (stx::ExpressionParserException &e) {
            throw FatalParserException("Expression parser error", e.what());
        }
    }
    
    static Datum evaluateParseTree(const stx::ParseTree &tree, const stx::SymbolTable &symbolTable) {
        if (tree.isEmpty()) {
            throw FatalParserException("Internal error", "Expression parse tree is empty");
        }
        
        try {
            return tree.evaluate(symbolTable);
        } catch (stx::ExpressionParserException &e){
            throw FatalParserException("Expression parser error", e.what());
        }
    }
    
    static Datum evaluateParseTree(const stx::ParseTree &tree) {
        return evaluateParseTree(tree, *global_variable_registry);
    }
    
    static Datum evaluateExpression(const std::string &expr, const stx::SymbolTable &symbolTable) {
        return evaluateParseTree(parseExpression(expr), symbolTable);
    }

    static Datum evaluateExpression(const std::string &expr) {
        return evaluateExpression(expr, *global_variable_registry);
    }
    
    static stx::ParseTreeList parseExpressionList(const std::string &exprList) {
        try {
            return stx::parseExpressionList(boost::algorithm::trim_copy(exprList));
        } catch (stx::ExpressionParserException &e) {
            throw FatalParserException("Expression parser error", e.what());
        }
    }
    
    static stx::ParseTreeList parseVarnameWithSubscripts(const std::string &input, std::string &varname) {
        try {
            return stx::parseVarnameWithSubscripts(boost::algorithm::trim_copy(input), varname);
        } catch (stx::ExpressionParserException &e) {
            throw FatalParserException("Expression parser error", e.what());
        }
    }
    
    static void evaluateParseTreeList(const stx::ParseTreeList &treeList,
                                      const stx::SymbolTable &symbolTable,
                                      std::vector<Datum> &values)
    {
        try {
            treeList.evaluate(values, symbolTable);
        } catch (stx::ExpressionParserException &e){
            throw FatalParserException("Expression parser error", e.what());
        }
    }
    
    static void evaluateParseTreeList(const stx::ParseTreeList &treeList, std::vector<Datum> &values) {
        return evaluateParseTreeList(treeList, *global_variable_registry, values);
    }
    
    static void evaluateExpressionList(const std::string &exprList,
                                       const stx::SymbolTable &symbolTable,
                                       std::vector<Datum> &values)
    {
        evaluateParseTreeList(parseExpressionList(exprList), symbolTable, values);
    }
    
    static void evaluateExpressionList(const std::string &exprList, std::vector<Datum> &values) {
        evaluateExpressionList(exprList, *global_variable_registry, values);
    }
    
    ParsedExpressionVariable(const std::string &expression_string) :
        expression_tree(parseExpression(expression_string))
    {
        getValue();
    }
	
    ParsedExpressionVariable(const stx::ParseTree &expression_tree) :
        expression_tree(expression_tree)
    {
        getValue();
    }
    
    Datum getValue() override {
        return Datum(evaluateParseTree(expression_tree));
    }
    
private:
    const stx::ParseTree expression_tree;
    
};


END_NAMESPACE_MW


#endif

