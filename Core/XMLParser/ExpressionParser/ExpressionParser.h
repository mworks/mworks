// $Id: ExpressionParser.h 59 2007-07-17 14:43:23Z tb $

/*
 * STX Expression Parser C++ Framework v0.7
 * Copyright (C) 2007 Timo Bingmann
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/** \file ExpressionParser.h
 * Definition of a the public interface of the STX Expression Parser. It
 * exports the abstract interface to a ParseNode tree root and the parse
 * functions themselves.
 */

#ifndef _STX_ExpressionParser_H_
#define _STX_ExpressionParser_H_

#include <string>
#include <vector>
#include <map>
#include <assert.h>
#include <boost/smart_ptr.hpp>
#include "AnyScalar.h"

/// STX - Some Template Extensions namespace
namespace stx {

/** Base class for other exceptions of the expression parser and
 * evaluators. \ingroup Exception */
class ExpressionParserException : public std::runtime_error
{
public:
    /// Construct with a description string.
    inline ExpressionParserException(const std::string &s) throw()
	: std::runtime_error(s)
    { }
};

/** ConversionException is an exception class thrown by some combinations of
 * get and set in AnyScalar. \ingroup Exception */

class ConversionException : public ExpressionParserException
{
public:
    /// Constructor of the exception takes the description string s.
    inline ConversionException(const std::string &s) throw()
	: ExpressionParserException(s)
    { }
};

/** ArithmeticException is an exception class thrown upon some arithmetic
 * errors like integer divison by zero. \ingroup Exception */

class ArithmeticException : public ExpressionParserException
{
public:
    /// Constructor of the exception takes the description string s.
    inline ArithmeticException(const std::string &s) throw()
	: ExpressionParserException(s)
    { }
};

/** Exception class thrown when the parser recognizes a syntax error.
 * \ingroup Exception */

class BadSyntaxException : public ExpressionParserException
{
public:
    /// Construct with a description string.
    inline BadSyntaxException(const std::string &s) throw()
	: ExpressionParserException(s)
    { }
};

/** Exception class thrown when the symbol table cannot find a variable or
 * function. \ingroup Exception */

class UnknownSymbolException : public ExpressionParserException
{
public:
    /// Construct with a description string.
    inline UnknownSymbolException(const std::string &s) throw()
	: ExpressionParserException(s)
    { }
};

/** Exception class thrown when the symbol table cannot correctly execute a
 * function. \ingroup Exception */

class BadFunctionCallException : public ExpressionParserException
{
public:
    /// Construct with a description string.
    inline BadFunctionCallException(const std::string &s) throw()
	: ExpressionParserException(s)
    { }
};

/** Abstract class used for evaluation of variables and function placeholders
 * within an expression. If you wish some standard mathematic function, then
 * derive your SymbolTable class from BasicSymbolTable instead of directly from
 * this one. */
class SymbolTable
{
public:
    /// STL container type used for parameter lists: a vector
    typedef std::vector<AnyScalar>	paramlist_type;

    /// Required for virtual functions.
    virtual ~SymbolTable();

    /// Return the (constant) value of a variable.
    virtual AnyScalar	lookupVariable(const std::string &varname) const = 0;

    /// Called when a program-defined function needs to be evaluated within an
    /// expression.
    virtual AnyScalar	processFunction(const std::string &funcname,
					const paramlist_type &paramlist) const = 0;
};

/** Concrete class used for evaluation of variables and function placeholders
 * within an expression. This class will always throw an UnknownSymbolException
 * for both lookup functions. It can be used as a base class for symbol tables
 * without the standard functions included in BasicSymbolTable. */
class EmptySymbolTable : public SymbolTable
{
public:
    /// STL container type used for parameter lists: a vector
    typedef std::vector<AnyScalar>	paramlist_type;

    /// Required for virtual functions.
    virtual ~EmptySymbolTable();

    /// Return the (constant) value of a variable. In this dummy implementation
    /// no variables are defined, it always throws an UnknownSymbolException.
    virtual AnyScalar	lookupVariable(const std::string &varname) const;

    /// Called when a program-defined function needs to be evaluated within an
    /// expression. In this dummy implementation no functions are defined, it
    /// always throws an UnknownSymbolException.
    virtual AnyScalar	processFunction(const std::string &funcname,
					const paramlist_type &paramlist) const;
};

/** Class representing variables and functions placeholders within an
 * expression. This base class contain two tables of variables and
 * functions. Variables may be filled into the STL map by the program. The
 * class also contains a set of basic mathematic functions.
 */
class BasicSymbolTable : public SymbolTable
{
public:
    /// Signature of a function used in the symbol table.
    typedef AnyScalar	(*functionptr_type)(const paramlist_type& paramlist);

protected:

    /// Container used to save a map of variable names
    typedef std::map<std::string, AnyScalar>	variablemap_type;

    /// Extra info about a function: the valid arguments.
    struct FunctionInfo
    {
	/// Number of arguments this function takes: either >= 0 for a fixed
	/// number of -1 for no checking.
	int		arguments;

	/// Function pointer to call.
	functionptr_type func;

	/// Initializing Constructor
	FunctionInfo(int _arguments = 0, functionptr_type _func = NULL)
	    : arguments(_arguments), func(_func)
	{
	}
    };

    /// Container used to save a map of function names
    typedef std::map<std::string, struct FunctionInfo>	functionmap_type;

private:
    /// Variable map which can be filled by the user-application
    variablemap_type	variablemap;

    /// Function map used to lookup standard or user-added function
    functionmap_type	functionmap;

protected:
    // *** Lots of Standard Functions

    /// Return the value of PI as a double AnyScalar
    static AnyScalar	funcPI(const paramlist_type& paramlist);

    /// Return the value of sin(x) as a double AnyScalar
    static AnyScalar	funcSIN(const paramlist_type& paramlist);

    /// Return the value of cos(x) as a double AnyScalar
    static AnyScalar	funcCOS(const paramlist_type& paramlist);

    /// Return the value of tan(x) as a double AnyScalar
    static AnyScalar	funcTAN(const paramlist_type& paramlist);

    /// Return the value of abs(x) or fabs(f)
    static AnyScalar	funcABS(const paramlist_type& paramlist);

    /// Return the value of exp(x) as a double AnyScalar
    static AnyScalar	funcEXP(const paramlist_type& paramlist);

    /// Return the value of log(x) as a double AnyScalar
    static AnyScalar	funcLOGN(const paramlist_type& paramlist);

    /// Return the value of pow(x,y) as a double AnyScalar
    static AnyScalar	funcPOW(const paramlist_type& paramlist);

    /// Return the value of sqrt(x) as a double AnyScalar
    static AnyScalar	funcSQRT(const paramlist_type& paramlist);

    static AnyScalar	funcUNIFORM_RAND(const paramlist_type& paramlist);
    static AnyScalar	funcDISC_UNIFORM_RAND(const paramlist_type& paramlist);
	static AnyScalar	funcGEOM_RAND(const paramlist_type& paramlist);
	static AnyScalar	funcNOW(const paramlist_type& paramlist);
	static AnyScalar	funcTIMER_EXPIRED(const paramlist_type& paramlist);
	
public:
    /// Fills in the functionmap with the standard functions.
    BasicSymbolTable();

    /// Required for virtual functions.
    virtual ~BasicSymbolTable();

    /// Return the (constant) value of a variable. In this basic implementation
    /// no variables are defined, it always throws an UnknownSymbolException.
    virtual AnyScalar	lookupVariable(const std::string &varname) const;

    /// Called when a program-defined function needs to be evaluated within an
    /// expression.
    virtual AnyScalar	processFunction(const std::string &funcname,
					const paramlist_type &paramlist) const;

    /// Add or replace a variable to the symbol table
    void	setVariable(const std::string& varname, const AnyScalar &value);

    /// Add or replace a function to the symbol table
    void	setFunction(const std::string& funcname, int arguments, functionptr_type funcptr);

    /// Clear variables table
    void	clearVariables();

    /// Clear function table
    void	clearFunctions();

    /// Add set of standard mathematic functions
    void	addStandardFunctions();
};

/** ParseNode is the abstract node interface of different parse nodes. From
 * these parse nodes the the ExpressionParser constructs a tree which can be
 * evaluated using different SymbolTable settings.
 */
class ParseNode
{
protected:
    /// Usual construction
    inline ParseNode()
    { }

    /// Disable copy construction
    ParseNode(const ParseNode &pn);

    /// And disable assignment
    ParseNode& operator=(const ParseNode &pn);
    
public:
    /// Virtual destructor so derived classes can deallocate their children
    /// nodes.
    virtual ~ParseNode()
    {
    }

    /// Function to recursively evaluate the contained parse tree and retrieve
    /// the calculated scalar value based on the given symbol table.
    virtual AnyScalar evaluate(const class SymbolTable &st = BasicSymbolTable()) const = 0;

    /// (Internal) Function to check if the subtree evaluates to a constant
    /// expression. If dest == NULL then do a static check whether the node is
    /// always a constant (ignoring subnodes), if dest != NULL try to calculate
    /// the constant value and type recursively, thus the return value can be
    /// true for a non-constant tree node.
    virtual bool evaluate_const(AnyScalar *dest) const = 0;

    /// Return the parsed expression as a string, which can be parsed again.
    virtual std::string toString() const = 0;
};

/** ParseTree contains the root node of a parse tree. It correctly allocates
 * and deletes parse node, because they themselves are not copy-constructable
 * or assignable. Pimpl class pattern with exposed inner class. */
class ParseTree
{
protected:
    /// Enclosed smart ptr so that the parse tree is not cloned when ParseTree
    /// instances are copied.
    boost::shared_ptr<ParseNode>	rootnode;

public:
    /// Create NULL parse tree object from the root ParseNode. All functions
    /// will assert() or segfault unless the tree is assigned.
    ParseTree()
	: rootnode(static_cast<ParseNode*>(NULL))
    {
    }

    /// Create parse tree object from the root ParseNode.
    ParseTree(ParseNode* pt)
	: rootnode(pt)
    {
    }

    /// Returns true if this object does not contain a parse tree.
    inline bool	isEmpty() const
    {
	return (rootnode.get() == NULL);
    }

    /// Function to recursively evaluate the contained parse tree and retrieve
    /// the calculated scalar value based on the given symbol table.
    AnyScalar	evaluate(const class SymbolTable &st = BasicSymbolTable()) const
    {
		assert(rootnode.get() != NULL);
		return rootnode->evaluate(st);
    }

    /// Return the parsed expression as a string, which can be parsed again.
    std::string	toString() const
    {
	assert(rootnode.get() != NULL);
	return rootnode->toString();
    }
};

/// Parse the given input expression into a parse tree. The parse tree is
/// represented by its root node, which can be evaluated.
const ParseTree parseExpression(const std::string &input);

/// Parse the given input expression into a parse tree. The parse tree is then
/// transformed into a XML tree for better visualisation.
std::string parseExpressionXML(const std::string &input);

/** ParseTreeList contains the root parse nodes of a list of expressions. It
 * correctly allocates and deletes the parse nodes, because they themselves are
 * not copy-constructable or assignable. */
class ParseTreeList : public std::vector<ParseTree>
{
protected:
    /// typedef of our parent class
    typedef std::vector<ParseTree>	parent_type;

public:
    /// Function to recursively evaluate all the contained parse trees and
    /// retrieve each calculated scalar value for the given symbol table.
    std::vector<AnyScalar>	evaluate(const class SymbolTable &st = BasicSymbolTable()) const;

    /// Return the list of parsed expression as a string, which can be parsed
    /// again.
    std::string	toString() const;
};

/// Parse the given input as an expression list "expr1, expr2, ..." into a
/// vector of parse trees. Each parse tree is represented by its root node,
/// which can be evaluated.
ParseTreeList parseExpressionList(const std::string &input);

} // namespace stx

#endif // _STX_ExpressionParser_H_
