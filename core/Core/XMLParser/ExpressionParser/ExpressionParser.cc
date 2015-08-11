// $Id: ExpressionParser.cc 59 2007-07-17 14:43:23Z tb $

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

/** \file ExpressionParser.cc
 * Implementation of the parser using a boost::spirit::classic grammar and a different
 * specializations of ParseNode.
 */

#include "ExpressionParser.h"
#include "Clock.h"

#include <boost/spirit/include/classic_core.hpp>

#include <boost/spirit/include/classic_ast.hpp>
#include <boost/spirit/include/classic_tree_to_xml.hpp>

#include <boost/spirit/include/classic_lists.hpp>
#include <boost/spirit/include/classic_distinct.hpp>
#include <boost/spirit/include/classic_escape_char.hpp>
#include <boost/spirit/include/classic_grammar_def.hpp> 

#ifdef seed
#undef seed
#endif
#include <boost/random.hpp>

#include <boost/math/special_functions/round.hpp>
#include <boost/format.hpp>

#include "Utilities.h"

#include <iostream>
#include <sstream>
#include <cmath>

#include "ComponentRegistry.h"
#include "Selectable.h"
#include "StimulusDisplay.h"

// ugly/tricky.  I wish the STL guys would get their acts together
char _to_lower (const char c) { return tolower(c); }
char _to_upper (const char c) { return toupper(c); }


// #define STX_DEBUG_PARSER

namespace stx MW_SYMBOL_PUBLIC {
	
	/// Enclosure for the spirit parser grammar and hidden parse node
	/// implementation classes.
	namespace Grammar {
		
		using namespace boost::spirit::classic;
		
		/// This enum specifies ids for the parse tree nodes created for each rule.
		enum parser_ids
		{
			boolean_const_id = 1,
			integer_const_id,
			long_const_id,
			double_const_id,
			string_const_id,
			constant_id,
            
            list_literal_id,
            dict_literal_id,
			
			function_call_id,
			function_identifier_id,
			
			varname_id,
			
			atom_expr_id,
			
			units_expr_id,
            subscript_expr_id,
			unary_expr_id,
			mul_expr_id,
			add_expr_id,
			
			cast_expr_id,
			cast_spec_id,
			
			comp_expr_id,
			and_expr_id,
			or_expr_id,
			
			expr_id,
			
			range_expr_id,
			
			exprlist_id,
		};
		
		/// Keyword parser used for matching words with () and spaces as separators.
		distinct_parser<> keyword_p("a-zA-Z0-9_");
		
		/// The boost::spirit::classic expression parser grammar
		struct ExpressionGrammar : public grammar<ExpressionGrammar>
		{
			/// The boost::spirit::classic expression parser grammar definition (for a specific
			/// scanner) with two entry points.
			template <typename ScannerT>
			struct definition : public grammar_def<rule<ScannerT, parser_context<>, parser_tag<expr_id> >,
			rule<ScannerT, parser_context<>, parser_tag<exprlist_id> > >
			{
				/// Real definition function of the grammar.
				definition(ExpressionGrammar const& /*self*/)
				{
					// *** Constants
					
					constant
					= double_const
					| integer_const
					| long_const
					| boolean_const
					| string_const
					;
					
					boolean_const
					= as_lower_d[keyword_p("true") | keyword_p("false") | keyword_p("yes") | keyword_p("no")]
					;
					
					integer_const
					= int_p
					;
					
					// this is needed because spirit's int_parsers don't work with
					// these long numbers
					long_const
					= token_node_d[ lexeme_d[ !( ch_p('+') | ch_p('-' ) ) >> +( range_p('0','9') ) ] ]
					;
					
					double_const
					= strict_real_p
					;
					
					string_const
					= lexeme_d[
							   token_node_d[ ('"'  >> *(c_escape_ch_p - '"' )  >> '"' ) |
                                             ('\'' >> *(c_escape_ch_p - '\'')  >> '\'') ]
							   ]
					;
                    
                    // *** List literal
                    
                    list_literal
                    = root_node_d[ ch_p('[') ] >> exprlist >> discard_node_d[ ch_p(']') ]
                    ;
                    
                    // *** Dictionary literal
                    
                    dict_literal
                    = root_node_d[ ch_p('{') ]
                    >> infix_node_d[ !list_p((expr >> ch_p(':') >> expr), ch_p(',')) ]
                    >> discard_node_d[ ch_p('}') ]
                    ;
					
					// *** Function call and function identifier
					
					function_call
					= root_node_d[function_identifier]
					>> discard_node_d[ ch_p('(') ] >> exprlist >> discard_node_d[ ch_p(')') ]
					;
					
					function_identifier
					= lexeme_d[ 
							   token_node_d[ alpha_p >> *(alnum_p | ch_p('_')) ]
							   ]
					;
					
					// *** Variable names
					
					varname
					= lexeme_d[
                               token_node_d[ alpha_p >> *(alnum_p | ch_p('_')) ]
                               ]
					;
					
					// *** Valid Expressions, from small to large
					
					atom_expr
					= constant
					| inner_node_d[ ch_p('(') >> expr >> ch_p(')') ]
                    | list_literal
                    | dict_literal
					| function_call
					| varname
					;
					
					units_expr
					= atom_expr
                    >> !( root_node_d[ as_lower_d[keyword_p("us") | keyword_p("ms") | keyword_p("s")] ] )
					;
                    
                    subscript_expr
                    = units_expr
                    >> *( root_node_d[ ch_p('[') ] >> expr >> discard_node_d[ ch_p(']') ] )
                    ;
					
					unary_expr
					= !( root_node_d[ ch_p('+') | ch_p('-') | ch_p('!') | as_lower_d[keyword_p("not")] ] )
					>> subscript_expr
					;
					
					cast_spec
					= discard_node_d[ ch_p('(') ]
					>> (
						keyword_p("bool") |
						keyword_p("char") | keyword_p("short") | keyword_p("int") | keyword_p("integer") | keyword_p("long") |
						keyword_p("byte") | keyword_p("word") | keyword_p("dword") | keyword_p("qword") |
						keyword_p("float") | keyword_p("double") |
						keyword_p("string")
						)
					>> discard_node_d[ ch_p(')') ]
					;
					
					cast_expr
					= root_node_d[ !cast_spec ] >> unary_expr
					;
					
					mul_expr
					= cast_expr
					>> *( root_node_d[ch_p('*') | ch_p('/') | ch_p('%')] >> cast_expr )
					;
					
					add_expr
					= mul_expr
					>> *( root_node_d[ch_p('+') | ch_p('-')] >> mul_expr )
					;
					
					comp_expr
					= add_expr
					>> *( root_node_d[str_p("==") | ch_p('=') |
                                      str_p("!=") |
                                      str_p("<=") | str_p("=<") | str_p("#LE") |
                                      ch_p('<') | str_p("#LT") |
                                      str_p(">=") | str_p("=>") | str_p("#GE") |
                                      ch_p('>') | str_p("#GT")] >> add_expr )
					;
					
					and_expr
					= comp_expr
					>> *( root_node_d[ as_lower_d[keyword_p("and")] | str_p("&&") | str_p("#AND") ] >> comp_expr )
					;
					
					or_expr
					= and_expr
					>> *( root_node_d[ as_lower_d[keyword_p("or")] | str_p("||") | str_p("#OR") ] >> and_expr )
					;
					
					// *** Base Expression and List
					
					expr
					= or_expr
					;
					
					range_expr
					= expr >> root_node_d[ch_p(':')] >> expr
					;
					
					exprlist
					= infix_node_d[ !list_p((range_expr | expr), ch_p(',')) ]
					;
					
					// Special spirit feature to declare multiple grammar entry points
					this->start_parsers(expr, exprlist); 
					
#ifdef STX_DEBUG_PARSER
					BOOST_SPIRIT_DEBUG_RULE(constant);
					
					BOOST_SPIRIT_DEBUG_RULE(boolean_const);
					BOOST_SPIRIT_DEBUG_RULE(integer_const);
					BOOST_SPIRIT_DEBUG_RULE(long_const);
					BOOST_SPIRIT_DEBUG_RULE(double_const);
					BOOST_SPIRIT_DEBUG_RULE(string_const);
                    
                    BOOST_SPIRIT_DEBUG_RULE(list_literal);
                    BOOST_SPIRIT_DEBUG_RULE(dict_literal);
					
					BOOST_SPIRIT_DEBUG_RULE(function_call);
					BOOST_SPIRIT_DEBUG_RULE(function_identifier);
					
					BOOST_SPIRIT_DEBUG_RULE(varname);
					
					BOOST_SPIRIT_DEBUG_RULE(atom_expr);
					
					BOOST_SPIRIT_DEBUG_RULE(units_expr);
                    BOOST_SPIRIT_DEBUG_RULE(subscript_expr);
					BOOST_SPIRIT_DEBUG_RULE(unary_expr);
					BOOST_SPIRIT_DEBUG_RULE(mul_expr);
					BOOST_SPIRIT_DEBUG_RULE(add_expr);
					
					BOOST_SPIRIT_DEBUG_RULE(cast_spec);
					BOOST_SPIRIT_DEBUG_RULE(cast_expr);
					
					BOOST_SPIRIT_DEBUG_RULE(comp_expr);
					BOOST_SPIRIT_DEBUG_RULE(and_expr);
					BOOST_SPIRIT_DEBUG_RULE(or_expr);
					
					BOOST_SPIRIT_DEBUG_RULE(expr);
					
					BOOST_SPIRIT_DEBUG_RULE(range_expr);
					
					BOOST_SPIRIT_DEBUG_RULE(exprlist);
#endif
				}
				
				/// Rule for a constant: one of the three scalar types integer_const,
				/// double_const or string_const
				rule<ScannerT, parser_context<>, parser_tag<constant_id> > 		constant;
				
				/// Boolean value constant rule: "true" or "false"
				rule<ScannerT, parser_context<>, parser_tag<boolean_const_id> > 	boolean_const;
				/// Integer constant rule: "1234"
				rule<ScannerT, parser_context<>, parser_tag<integer_const_id> > 	integer_const;
				/// Long integer constant rule: "12345452154"
				rule<ScannerT, parser_context<>, parser_tag<long_const_id> > 		long_const;
				/// Float constant rule: "1234.3"
				rule<ScannerT, parser_context<>, parser_tag<double_const_id> > 		double_const;
				/// String constant rule: with quotes "abc"
				rule<ScannerT, parser_context<>, parser_tag<string_const_id> > 		string_const;
                
                /// List literal rule: [a,b,c] where a,b,c is a list of exprs
                rule<ScannerT, parser_context<>, parser_tag<list_literal_id> >      list_literal;
                /// Dictionary literal rule
                rule<ScannerT, parser_context<>, parser_tag<dict_literal_id> >      dict_literal;
				
				/// Function call rule: func1(a,b,c) where a,b,c is a list of exprs
				rule<ScannerT, parser_context<>, parser_tag<function_call_id> > 	function_call;
				/// Function rule to match a function identifier: alphanumeric and _
				/// are allowed.
				rule<ScannerT, parser_context<>, parser_tag<function_identifier_id> > 	function_identifier;
				
				/// Rule to match a variable name: alphanumeric with _
				rule<ScannerT, parser_context<>, parser_tag<varname_id> > 		varname;
				
				/// Helper rule which implements () bracket grouping.
				rule<ScannerT, parser_context<>, parser_tag<atom_expr_id> > 		atom_expr;
				
				/// Units operator rule: recognizes "us" "ms" and "s".
				rule<ScannerT, parser_context<>, parser_tag<units_expr_id> > 		units_expr;
                /// Subscript operator rule
                rule<ScannerT, parser_context<>, parser_tag<subscript_expr_id> > 	subscript_expr;
				/// Unary operator rule: recognizes + - ! and "not".
				rule<ScannerT, parser_context<>, parser_tag<unary_expr_id> > 		unary_expr;
				/// Binary operator rule taking precedent before add_expr:
				/// recognizes * and /
				rule<ScannerT, parser_context<>, parser_tag<mul_expr_id> > 		mul_expr;
				/// Binary operator rule: recognizes + and -
				rule<ScannerT, parser_context<>, parser_tag<add_expr_id> > 		add_expr;
				
				/// Match all the allowed cast types: short, double, etc.
				rule<ScannerT, parser_context<>, parser_tag<cast_spec_id> >        	cast_spec;
				/// Cast operator written like in C: (short)
				rule<ScannerT, parser_context<>, parser_tag<cast_expr_id> >        	cast_expr;
				
				/// Comparison operator: recognizes == = != <= >= < > => =<
				rule<ScannerT, parser_context<>, parser_tag<comp_expr_id> >        	comp_expr;
				/// Boolean operator: recognizes && and "and" and works only on boolean
				/// values
				rule<ScannerT, parser_context<>, parser_tag<and_expr_id> >        	and_expr;
				/// Boolean operator: recognizes || and "or" and works only on boolean
				/// values
				rule<ScannerT, parser_context<>, parser_tag<or_expr_id> >        	or_expr;
				
				/// Base rule to match an expression 
				rule<ScannerT, parser_context<>, parser_tag<expr_id> >        		expr;
				
				/// Range expression 
				rule<ScannerT, parser_context<>, parser_tag<range_expr_id> >        range_expr;
				
				/// Base rule to match a comma-separated list of expressions (used for
				/// function arguments and lists of expressions)
				rule<ScannerT, parser_context<>, parser_tag<exprlist_id> >    		exprlist;
			};
		};
		
		// *** Classes representing the nodes in the resulting parse tree, these need
		// *** not be publicly available via the header file.
		
		/// Constant value nodes of the parse tree. This class holds any of the three
		/// constant types in the enclosed Datum object.
		class PNConstant : public ParseNode
			{
			private:
				/// The constant parsed value.
				Datum	value;
				
			public:
				/// constructor for folded constant values.
				PNConstant(const Datum &_value)
				: value(_value)
				{
				}
				
				/// Easiest evaluation: return the constant.
				virtual Datum evaluate(const class SymbolTable &) const
				{
					return value;
				}
				
				/// Returns true, because value is constant
				virtual bool evaluate_const(Datum *dest) const
				{
					if (dest) *dest = value;
					return true;
				}
				
				/// String representation of the constant Datum value.
				virtual std::string toString() const
				{
					return value.toString(true);
				}
			};
		
		/// Parse tree node representing a variable place-holder. It is filled when
		/// parameterized by a symbol table.
		class PNVariable : public ParseNode
			{
			private:
				/// String name of the variable
				std::string		varname;
				
			public:
				/// Constructor from the string received from the parser.
				PNVariable(std::string _varname)
				: ParseNode(), varname(_varname)
				{
				}
				
				/// Recursively delete the parse tree.
				virtual ~PNVariable()
				{
				}
				
				/// Check the given symbol table for the actual value of this variable.
				virtual Datum evaluate(const class SymbolTable &st) const
				{
                    return st.lookupVariable(varname);
				}
				
				/// Returns false, because value isn't constant.
				virtual bool evaluate_const(Datum *) const
				{
					return false;
				}
				
				/// Nothing but the variable name.
				virtual std::string toString() const
				{
                    return varname;
				}
			};
        
        /// Parse tree node representing a list literal.
        class PNListLiteral : public ParseNode
        {
        public:
            /// Type of sequence of subtrees to evaluate as list items.
            typedef std::vector<const class ParseNode*> itemlist_type;
            
        private:
            /// The array of list item subtrees
            itemlist_type	itemlist;
            
        public:
            /// Constructor from the string received from the parser.
            PNListLiteral(const itemlist_type& _itemlist)
            : ParseNode(), itemlist(_itemlist)
            {
            }
            
            /// Delete the itemlist
            ~PNListLiteral()
            {
                for(unsigned int i = 0; i < itemlist.size(); ++i)
                    delete itemlist[i];
            }
            
            virtual Datum evaluate(const class SymbolTable &st) const
            {
                std::vector<Datum> itemvalues;
                
                for(unsigned int i = 0; i < itemlist.size(); ++i)
                {
                    itemlist[i]->evaluate(itemvalues, st);
                }
                
                Datum value(mw::M_LIST, int(itemvalues.size()));
                
                for (int i = 0; i < itemvalues.size(); i++) {
                    value.setElement(i, itemvalues.at(i));
                }
                
                return value;
            }
            
            /// Returns false, because value isn't constant.
            virtual bool evaluate_const(Datum *) const
            {
                return false;
            }
            
            virtual std::string toString() const
            {
                std::string str = "[";
                for(unsigned int i = 0; i < itemlist.size(); ++i)
                {
                    if (i != 0) str += ",";
                    str += itemlist[i]->toString();
                }
                return str + "]";
            }
        };
        
        /// Parse tree node representing a dictionary literal.
        class PNDictLiteral : public ParseNode
        {
        public:
            /// Type of sequence of subtrees to evaluate as dict items.
            typedef std::vector<std::pair<const class ParseNode*, const class ParseNode*>> itemlist_type;
            
        private:
            /// The array of dict item subtrees
            itemlist_type	itemlist;
            
        public:
            /// Constructor from the string received from the parser.
            PNDictLiteral(const itemlist_type& _itemlist)
            : ParseNode(), itemlist(_itemlist)
            {
            }
            
            /// Delete the itemlist
            ~PNDictLiteral()
            {
                for(unsigned int i = 0; i < itemlist.size(); ++i) {
                    delete itemlist[i].first;
                    delete itemlist[i].second;
                }
            }
            
            virtual Datum evaluate(const class SymbolTable &st) const
            {
                Datum value(mw::M_DICTIONARY, int(itemlist.size()));
                
                for(unsigned int i = 0; i < itemlist.size(); ++i)
                {
                    Datum itemKey = itemlist[i].first->evaluate();
                    Datum itemValue = itemlist[i].second->evaluate();
                    value.addElement(itemKey, itemValue);
                }
                
                return value;
            }
            
            /// Returns false, because value isn't constant.
            virtual bool evaluate_const(Datum *) const
            {
                return false;
            }
            
            virtual std::string toString() const
            {
                std::string str = "{";
                for(unsigned int i = 0; i < itemlist.size(); ++i)
                {
                    if (i != 0) str += ",";
                    str += itemlist[i].first->toString();
                    str += ":";
                    str += itemlist[i].second->toString();
                }
                return str + "}";
            }
        };
		
		/// Parse tree node representing a function place-holder. It is filled when
		/// parameterized by a symbol table.
		class PNFunction : public ParseNode
			{
			public:
				/// Type of sequence of subtrees to evaluate as function parameters.
				typedef std::vector<const class ParseNode*> paramlist_type;
				
			private:
				/// String name of the function
				std::string		funcname;
				
				/// The array of function parameter subtrees
				paramlist_type	paramlist;
				
			public:
				/// Constructor from the string received from the parser.
				PNFunction(std::string _funcname, const paramlist_type& _paramlist)
				: ParseNode(), funcname(_funcname), paramlist(_paramlist)
				{
				}
				
				/// Delete the paramlist
				~PNFunction()
				{
					for(unsigned int i = 0; i < paramlist.size(); ++i)
						delete paramlist[i];
				}
				
				/// Check the given symbol table for the actual value of this variable.
				virtual Datum evaluate(const class SymbolTable &st) const
				{
					std::vector<Datum> paramvalues;
					
					for(unsigned int i = 0; i < paramlist.size(); ++i)
					{
						paramlist[i]->evaluate(paramvalues, st);
					}
					
					return st.processFunction(funcname, paramvalues);
				}
				
				/// Returns false, because value isn't constant.
				virtual bool evaluate_const(Datum *) const
				{
					return false;
				}
				
				/// Nothing but the function and its parameters
				virtual std::string toString() const
				{
					std::string str = funcname + "(";
					for(unsigned int i = 0; i < paramlist.size(); ++i)
					{
						if (i != 0) str += ",";
						str += paramlist[i]->toString();
					}
					return str + ")";
				}
			};
		
		/// Parse tree node representing a units operator: "us", "ms", or "s".
		/// This node has one child.
		class PNUnitsArithmExpr : public ParseNode
        {
        private:
            /// Pointer to the single operand
            const ParseNode 	*operand;
            
            /// Units to apply: "us", "ms", or "s".
            std::string units;
            
        public:
            /// Constructor from the parser: operand subnode and operator id.
            PNUnitsArithmExpr(const ParseNode* _operand, const std::string &_units)
            : ParseNode(), operand(_operand), units(_units)
            { }
            
            /// Recursively delete the parse tree.
            virtual ~PNUnitsArithmExpr()
            {
                delete operand;
            }
            
            /// Applies the operator to the recursively calculated value.
            virtual Datum evaluate(const class SymbolTable &st) const
            {
                Datum dest = operand->evaluate(st);
                
                if (units == "s") {
                    dest = dest * Datum(1000000);
                }
                else if (units == "ms")
                {
                    dest = dest * Datum(1000);
                }
                else {
                    // No change for "us"
                    assert(units == "us");
                }
                
                return dest;
            }
            
            /// Calculates subnodes and returns result if the operator can be applied.
            virtual bool evaluate_const(Datum *dest) const
            {
                if (!dest) return false;
                
                bool b = operand->evaluate_const(dest);
                
                if (units == "s") {
                    *dest = (*dest) * Datum(1000000);
                }
                else if (units == "ms")
                {
                    *dest = (*dest) * Datum(1000);
                }
                else {
                    // No change for "us"
                    assert(units == "us");
                }
                
                return b;
            }
            
            /// Return the subnode's string with this operator prepended.
            virtual std::string toString() const
            {
                return std::string("(") + operand->toString() + " " + units + ")";
            }
        };
        
        /// Parse tree node representing subscripting. This node has two children.
        class PNSubscriptExpr : public ParseNode
            {
            private:
                const ParseNode *target;
                const ParseNode *subscript;
                
            public:
                PNSubscriptExpr(const ParseNode* _target,
                                const ParseNode* _subscript)
                : ParseNode(),
                target(_target), subscript(_subscript)
                { }
                
                /// Recursively delete parse tree.
                virtual ~PNSubscriptExpr()
                {
                    delete target;
                    delete subscript;
                }
                
                virtual Datum evaluate(const class SymbolTable &st) const
                {
                    Datum vt = target->evaluate(st);
                    Datum vs = subscript->evaluate(st);
                    
                    Datum value = vt[vs];
                    
                    if (value.isUndefined()) {
                        mw::mwarning(mw::M_SYSTEM_MESSAGE_DOMAIN, "Subscript evaluation failed.  Returning 0 instead.");
                        value.setInteger(0);
                    }
                    
                    return value;
                }
                
                /// Returns false, because value isn't constant.
                virtual bool evaluate_const(Datum *dest) const
                {
                    return false;
                }
                
                virtual std::string toString() const
                {
                    return std::string("(") + target->toString() + "[" + subscript->toString() + "])";
                }
            };
		
		/// Parse tree node representing an unary operator: '+', '-', '!' or
		/// "not". This node has one child.
		class PNUnaryArithmExpr : public ParseNode
			{
			private:
				/// Pointer to the single operand
				const ParseNode 	*operand;
				
				/// Arithmetic operation to perform: either '+', '-' or '!'. Further
				/// optimization would be to create an extra class for each op
				char	op;
				
			public:
				/// Constructor from the parser: operand subnode and operator id.
				PNUnaryArithmExpr(const ParseNode* _operand, char _op)
				: ParseNode(), operand(_operand), op(_op)
				{
					if (op == 'n' || op == 'N') op = '!';
				}
				
				/// Recursively delete the parse tree.
				virtual ~PNUnaryArithmExpr()
				{
					delete operand;
				}
				
				/// Applies the operator to the recursively calculated value.
				virtual Datum evaluate(const class SymbolTable &st) const
				{
					Datum dest = operand->evaluate(st);
					
					if (op == '-') {
						dest = -dest;
					}
					else if (op == '!')
					{
						if (dest.getBool()) {
							dest = false;
						} else {
							dest = true;
						}
					}
					else {
						assert(op == '+');
					}
					
					return dest;
				}
				
				/// Calculates subnodes and returns result if the operator can be applied.
				virtual bool evaluate_const(Datum *dest) const
				{
					if (!dest) return false;
					
					bool b = operand->evaluate_const(dest);
					
					if (op == '-') {
						*dest = -(*dest);
					}
					else if (op == '!')
					{
						if(dest->getBool()) {
							*dest = false;
						} else {
							*dest = true;
						}
					}
					else {
						assert(op == '+');
					}
					
					return b;
				}
				
				/// Return the subnode's string with this operator prepended.
				virtual std::string toString() const
				{
					return std::string("(") + op + " " + operand->toString() + ")";
				}
			};
		
		/// Parse tree node representing a binary operators: +, -, * and / for numeric
		/// values. This node has two children.
		class PNBinaryArithmExpr : public ParseNode
			{
			private:
				/// Pointers to the left of the two child parse trees.
				const ParseNode 	*left;
				
				/// Pointers to the right of the two child parse trees.
				const ParseNode	*right;
				
				/// Arithmetic operation to perform: left op right.
				/// Further optimization would be to create an extra class for each op
				char	op;
				
			public:
				/// Constructor from the parser: both operand subnodes and the operator id.
				PNBinaryArithmExpr(const ParseNode* _left,
								   const ParseNode* _right,
								   char _op)
				: ParseNode(),
				left(_left), right(_right), op(_op)
				{ }
				
				/// Recursively delete parse tree.
				virtual ~PNBinaryArithmExpr()
				{
					delete left;
					delete right;
				}
				
				/// Applies the operator to the two recursive calculated values. The actual
				/// switching between types is handled by Datum's operators.
				virtual Datum evaluate(const class SymbolTable &st) const
				{
					Datum vl = left->evaluate(st);
					Datum vr = right->evaluate(st);
					
					if (op == '+') {
						return (vl + vr);
					}
					else if (op == '-') {
						return (vl - vr);
					}
					else if (op == '*') {
						return (vl * vr);
					}
					else if (op == '/') {
						return (vl / vr);
					}
					else if (op == '%') {
						return (vl % vr);
					}
					
					assert(0);
					return Datum();
				}
				
				/// Returns false because this node isn't always constant. Tries to
				/// calculate a constant subtree's value.
				virtual bool evaluate_const(Datum *dest) const
				{
					if (!dest) return false;
					
					Datum vl, vr;
					
					bool bl = left->evaluate_const(&vl);
					bool br = right->evaluate_const(&vr);
					
					if (op == '+') {
						*dest = vl + vr;
					}
					else if (op == '-') {
						*dest = vl - vr;
					}
					else if (op == '*') {
						*dest = vl * vr;
					}
					else if (op == '/') {
						*dest = vl / vr;
					}
					else if (op == '%') {
						*dest = vl % vr;
					}
					
					return (bl && br);
				}
				
				/// String representing (operandA op operandB)
				virtual std::string toString() const
				{
					return std::string("(") + left->toString() + " " + op + " " + right->toString() + ")";
				}
			};
		
		/// Parse tree node handling type conversions within the tree.
		class PNCastExpr : public ParseNode
			{
			private:
				/// Child tree of which the return value should be casted.
				const ParseNode*	operand;
				
				/// Datum type to cast the value to.
                mw::GenericDataType	type;
				
			public:
				/// Constructor from the parser: operand subnode and the cast type as
				/// recognized by Datum.
				PNCastExpr(const ParseNode* _operand, mw::GenericDataType _type)
				: ParseNode(),
				operand(_operand), type(_type)
				{ }
				
				/// Recursively delete parse tree.
				virtual ~PNCastExpr()
				{
					delete operand;
				}
                
                static Datum convert(const Datum &value, mw::GenericDataType type) {
                    switch (type) {
                        case mw::M_BOOLEAN:
                            return value.getBool();
                            
                        case mw::M_FLOAT:
                            return value.getFloat();
                            
                        case mw::M_STRING:
                            return value.toString();
                            
                        default:
                            return value.getInteger();
                    }
                }
				
				/// Recursive calculation of the value and subsequent casting
				virtual Datum evaluate(const class SymbolTable &st) const
				{
					Datum val = operand->evaluate(st);
					return convert(val, type);
				}
				
				virtual bool evaluate_const(Datum *dest) const
				{
					if (!dest) return false;
					
					bool b = operand->evaluate_const(dest);
                    *dest = convert(*dest, type);
                    
					return b;
				}
                
                static const char * getTypeString(mw::GenericDataType type) {
                    switch (type) {
                        case mw::M_BOOLEAN:
                            return "bool";
                            
                        case mw::M_FLOAT:
                            return "float";
                            
                        case mw::M_STRING:
                            return "string";
                            
                        default:
                            return "integer";
                    }
                }
				
				/// c-like representation of the cast
				virtual std::string toString() const
				{
					return std::string("((") + getTypeString(type) + ")" + operand->toString() + ")";
				}
			};
		
		/// Parse tree node representing a binary comparison operator: ==, =, !=, <, >,
		/// >=, <=, =>, =<. This node has two children.
		class PNBinaryComparisonExpr : public ParseNode
			{
			private:
				/// Pointers to the left of the two child parse trees.
				const ParseNode 	*left;
				
				/// Pointers to the right of the two child parse trees.
				const ParseNode	*right;
				
				/// Comparison operation to perform: left op right
				enum { EQUAL, NOTEQUAL, LESS, GREATER, LESSEQUAL, GREATEREQUAL } op;
				
				/// String saved for toString()
				std::string		opstr;
				
			public:
				/// Constructor from the parser: both operand subnodes and the operator id.
				PNBinaryComparisonExpr(const ParseNode* _left,
									   const ParseNode* _right,
									   std::string _op)
				: ParseNode(),
				left(_left), right(_right), opstr(_op)
				{
					if (_op == "==" || _op == "=")
						op = EQUAL;
					else if (_op == "!=")
						op = NOTEQUAL;
					else if (_op == "<" || _op == "#LT")
						op = LESS;
					else if (_op == ">" || _op == "#GT")
						op = GREATER;
					else if (_op == "<=" || _op == "=<" || _op == "#LE")
						op = LESSEQUAL;
					else if (_op == ">=" || _op == "=>" || _op == "#GE")
						op = GREATEREQUAL;
					else
						throw(BadSyntaxException("Program Error: invalid binary comparision operator."));
				}
				
				/// Recursively delete parse tree.
				virtual ~PNBinaryComparisonExpr()
				{
					delete left;
					delete right;
				}
				
				/// Applies the operator to the two recursive calculated values. The actual
				/// switching between types is handled by Datum's operators. This
				/// result type of this processing node is always bool.
				virtual Datum evaluate(const class SymbolTable &st) const
				{
					Datum vl = left->evaluate(st);
					Datum vr = right->evaluate(st);
					
					Datum dest;
					
					switch(op)
					{
						case EQUAL:
							dest = (vl == vr);
							break;
							
						case NOTEQUAL:
							dest = (vl != vr);
							break;
							
						case LESS:
							dest = (vl < vr);
							break;
							
						case GREATER:
							dest = (vl > vr);
							break;
							
						case LESSEQUAL:
							dest = (vl <= vr);
							break;
							
						case GREATEREQUAL:
							dest = (vl >= vr);
							break;
							
						default:
							assert(0);
					}
					
					return dest;
				}
				
				/// Returns false because this node isn't always constant.
				virtual bool evaluate_const(Datum *dest) const
				{
					if (!dest) return false;
					
					Datum vl, vr;
					
					bool bl = left->evaluate_const(&vl);
					bool br = right->evaluate_const(&vr);
					
					switch(op)
					{
						case EQUAL:
							*dest = (vl == vr);
							break;
							
						case NOTEQUAL:
							*dest = (vl != vr);
							break;
							
						case LESS:
							*dest = (vl < vr);
							break;
							
						case GREATER:
							*dest = (vl > vr);
							break;
							
						case LESSEQUAL:
							*dest = (vl <= vr);
							break;
							
						case GREATEREQUAL:
							*dest = (vl >= vr);
							break;
							
						default:
							assert(0);
					}
					
					return (bl && br);
				}
				
				/// String (operandA op operandB)
				virtual std::string toString() const
				{
					return std::string("(") + left->toString() + " " + opstr + " " + right->toString() + ")";
				}
			};
		
		/// Parse tree node representing a binary logic operator: and, or, &&, ||. This
		/// node has two children.
		class PNBinaryLogicExpr : public ParseNode
			{
			private:
				/// Pointers to the left of the two child parse trees.
				ParseNode*		left;
				
				/// Pointers to the right of the two child parse trees.
				ParseNode*		right;
				
				/// Comparison operation to perform: left op right
				enum { OP_AND, OP_OR } op;
				
			public:
				/// Constructor from the parser: both operand subnodes and the operator id.
				PNBinaryLogicExpr(ParseNode* _left,
								  ParseNode* _right,
								  std::string _op)
				: ParseNode(),
				left(_left), right(_right)
				{
					if (_op == "and" || _op == "&&" || _op == "#and")
						op = OP_AND;
					else if (_op == "or" || _op == "||" || _op == "#or")
						op = OP_OR;
					else
						throw(BadSyntaxException("Program Error: invalid binary logic operator: " + _op));
				}
				
				/// Recursively delete parse tree.
				virtual ~PNBinaryLogicExpr()
				{
					if (left) delete left;
					if (right) delete right;
				}
				
				/// Calculate the operator
				inline bool do_operator(bool left, bool right) const
				{
					if (op == OP_AND)
						return left && right;
					else if (op == OP_OR)
						return left || right;
					else
						return false;
				}
				
				/// Return the string of this operator
				inline std::string get_opstr() const
				{
					return (op == OP_AND) ? "&&" : "||";
				}
				
				/// Applies the operator to the two recursive calculated values. The actual
				/// switching between types is handled by Datum's operators.
				virtual Datum evaluate(const class SymbolTable &st) const
				{
					Datum vl = left->evaluate(st);
					Datum vr = right->evaluate(st);
					
					int bvl = vl.getInteger();
					int bvr = vr.getInteger();
					
					return Datum( do_operator(bvl, bvr) );
				}
				
				/// Applies the operator to the two recursive calculated const
				/// values. Determining if this node is constant is somewhat more tricky
				/// than with the other parse nodes: AND with a false operand is always
				/// false. OR with a true operand is always true.
				virtual bool evaluate_const(Datum *dest) const
				{
					if (!dest) return false; // returns false because this node isn't always constant
					
					Datum vl, vr;
					
					bool bl = left->evaluate_const(&vl);
					bool br = right->evaluate_const(&vr);
					
					int bvl = 0;
                    if (bl) bvl = vl.getInteger();
					int bvr = 0;
                    if (br) bvr = vr.getInteger();
					
                    if (bl && br) {
                        *dest = do_operator(bvl, bvr);
                        return true;
                    }
					else if (op == OP_AND)
					{
						// true if either of the ops is constant and evaluates to false.
                        if ((bl && !bvl) || (br && !bvr)) {
                            *dest = false;
                            return true;
                        }
					}
					else if (op == OP_OR)
					{
						// true if either of the ops is constant and evaluates to true.
                        if ((bl && bvl) || (br && bvr)) {
                            *dest = true;
                            return true;
                        }
					}
                    
                    return false;
				}
				
				/// String (operandA op operandB)
				virtual std::string toString() const
				{
					return std::string("(") + left->toString() + " " + get_opstr() + " " + right->toString() + ")";
				}
				
				/// Detach left node
				inline ParseNode* detach_left()
				{
					ParseNode *n = left;
					left = NULL;
					return n;
				}
				
				/// Detach right node
				inline ParseNode* detach_right()
				{
					ParseNode *n = right;
					right = NULL;
					return n;
				}
			};
		
		/// Parse tree node representing a range expression. This node has two children.
		class PNRangeExpr : public ParseNode
			{
			private:
				/// Pointer to the first of the two child parse trees.
				const ParseNode *start;
				
				/// Pointer to the second of the two child parse trees.
				const ParseNode	*stop;
				
			public:
				/// Constructor from the parser
				PNRangeExpr(const ParseNode* _start,
                            const ParseNode* _stop)
				: ParseNode(),
				start(_start), stop(_stop)
				{ }
				
				/// Recursively delete parse tree.
				virtual ~PNRangeExpr()
				{
					delete start;
					delete stop;
				}
				
				/// Always throws, because a range expression can't be treated as a single scalar value
				Datum evaluate(const class SymbolTable &st) const override
				{
                    throw ExpressionParserException("Internal error: range expression cannot be evaluated as a scalar");
				}
				
				/// Evaluates and returns the full range of values
				void evaluate(std::vector<Datum> &values, const class SymbolTable &st) const override
				{
					Datum first = start->evaluate(st);
					Datum last = stop->evaluate(st);
                    
                    if (!(first.isInteger() && last.isInteger())) {
                        throw ExpressionParserException("start and stop values of range expression must be integers");
                    }
                    
                    const long long firstValue = first.getInteger();
                    const long long lastValue = last.getInteger();
                    const long long delta = ((firstValue <= lastValue) ? 1 : -1);
                    
                    for (long long currentValue = firstValue;
                         currentValue != (lastValue + delta);
                         currentValue += delta)
                    {
                        values.emplace_back(currentValue);
                    }
				}
				
				/// Returns false, because value isn't constant.
				virtual bool evaluate_const(Datum *) const
				{
					return false;
				}
				
				/// String representing (operandA : operandB)
				virtual std::string toString() const
				{
					return start->toString() + " : " + stop->toString();
				}
			};
		
		// *** Functions which translate the resulting parse tree into our expression
		// *** tree, simultaneously folding constants.
		
		/// Iterator type used by spirit's parsers.
		typedef std::string::const_iterator InputIterT;
		
		/// Resulting match tree after parsing
		typedef tree_match<InputIterT> ParseTreeMatchT;
		
		/// The iterator of the match tree used in build_expr()
		typedef ParseTreeMatchT::const_tree_iterator TreeIterT;
		
		/// Build_expr is the constructor method to create a parse tree from the
		/// AST-tree returned by the spirit parser.
		static ParseNode* build_expr(TreeIterT const& i)
		{
#ifdef STX_DEBUG_PARSER
			std::cout << "In build_expr. i->value = " <<
			std::string(i->value.begin(), i->value.end()) <<
			" i->children.size() = " << i->children.size() << 
			" i->value.id = " << i->value.id().to_long() << std::endl;
#endif
			
			switch(i->value.id().to_long())
			{
					// *** Constant node cases
					
				case boolean_const_id:
				{
                    std::string boolconst(i->value.begin(), i->value.end());
					std::transform(boolconst.begin(), boolconst.end(), boolconst.begin(), &_to_lower);
					return new PNConstant(Datum((boolconst == "true") || (boolconst == "yes")));
				}
					
				case integer_const_id:
				case long_const_id:
				{
                    std::string longconst(i->value.begin(), i->value.end());
					return new PNConstant(Datum(std::strtoll(longconst.c_str(), nullptr, 10)));
				}
					
				case double_const_id:
				{
                    std::string doubleconst(i->value.begin(), i->value.end());
					return new PNConstant(Datum(std::strtod(doubleconst.c_str(), nullptr)));
				}
					
				case string_const_id:
				{
                    std::string stringconst(i->value.begin(), i->value.end());
                    Datum val;
                    val.setStringQuoted(stringconst);
					return new PNConstant(val);
				}
					
					// *** Arithmetic node cases
					
				case units_expr_id:
				{
					assert(i->children.size() == 1);
                    
					std::string units(i->value.begin(), i->value.end());
					std::transform(units.begin(), units.end(), units.begin(), &_to_lower);
					
					const ParseNode *val = build_expr(i->children.begin());
					
					if (val->evaluate_const(NULL))
					{
						// construct a constant node
						PNUnitsArithmExpr tmpnode(val, units);
						Datum constval;
						
						tmpnode.evaluate_const(&constval);
						
						return new PNConstant(constval);
					}
					else
					{
						// calculation node
						return new PNUnitsArithmExpr(val, units);
					}
				}
                    
                case subscript_expr_id:
                {
                    assert(i->children.size() == 2);
                    
                    // auto_ptr needed because of possible parse exceptions in build_expr.
                    std::auto_ptr<const ParseNode> target( build_expr(i->children.begin()) );
                    std::auto_ptr<const ParseNode> subscript( build_expr(i->children.begin()+1) );
                    
                    return new PNSubscriptExpr(target.release(), subscript.release());
                }
					
				case unary_expr_id:
				{
					char arithop = *i->value.begin();
					assert(i->children.size() == 1);
					
					const ParseNode *val = build_expr(i->children.begin());
					
					if (val->evaluate_const(NULL))
					{
						// construct a constant node
						PNUnaryArithmExpr tmpnode(val, arithop);
						Datum constval;
						
						tmpnode.evaluate_const(&constval);
						
						return new PNConstant(constval);
					}
					else
					{
						// calculation node
						return new PNUnaryArithmExpr(val, arithop);
					}
				}
					
				case add_expr_id:
				case mul_expr_id:
				{
					char arithop = *i->value.begin();
					assert(i->children.size() == 2);
					
					// auto_ptr needed because of possible parse exceptions in build_expr.
					
					std::auto_ptr<const ParseNode> left( build_expr(i->children.begin()) );
					std::auto_ptr<const ParseNode> right( build_expr(i->children.begin()+1) );
					
					if (left->evaluate_const(NULL) && right->evaluate_const(NULL))
					{
						// construct a constant node
						PNBinaryArithmExpr tmpnode(left.release(), right.release(), arithop);
						Datum both;
						
						tmpnode.evaluate_const(&both);
						
						// left and right are deleted by tmpnode's deconstructor
						
						return new PNConstant(both);
					}
					else
					{
						// calculation node
						return new PNBinaryArithmExpr(left.release(), right.release(), arithop);
					}
				}
					
					// *** Cast node case
					
				case cast_spec_id:
				{
					assert(i->children.size() == 1);
					
					std::string tname(i->value.begin(), i->value.end());
                    mw::GenericDataType type;
                    
                    if (tname == "bool") {
                        type = mw::M_BOOLEAN;
                    } else if (tname == "float" || tname == "double") {
                        type = mw::M_FLOAT;
                    } else if (tname == "string") {
                        type = mw::M_STRING;
                    } else {
                        // All other types are integral
                        type = mw::M_INTEGER;
                    }

					const ParseNode *val = build_expr(i->children.begin());
					
					if (val->evaluate_const(NULL))
					{
						// construct a constant node
						PNCastExpr tmpnode(val, type);
						
						Datum constval;
						
						tmpnode.evaluate_const(&constval);
						
						return new PNConstant(constval);
					}
					else
					{
						return new PNCastExpr(val, type);
					}
				}
					
					// *** Binary Comparison Operator
					
				case comp_expr_id:
				{
					assert(i->children.size() == 2);
					
					std::string arithop(i->value.begin(), i->value.end());
					
					// we need auto_ptr because of possible parse exceptions in build_expr.
					
					std::auto_ptr<const ParseNode> left( build_expr(i->children.begin()) );
					std::auto_ptr<const ParseNode> right( build_expr(i->children.begin()+1) );
					
					if (left->evaluate_const(NULL) && right->evaluate_const(NULL))
					{
						// construct a constant node
						PNBinaryComparisonExpr tmpnode(left.release(), right.release(), arithop);
						Datum both;
						
						tmpnode.evaluate_const(&both);
						
						// left and right are deleted by tmpnode's deconstructor
						
						return new PNConstant(both);
					}
					else
					{
						// calculation node
						return new PNBinaryComparisonExpr(left.release(), right.release(), arithop);
					}
				}
					
					// *** Binary Logic Operator
					
				case and_expr_id:
				case or_expr_id:
				{
					assert(i->children.size() == 2);
					
					std::string logicop(i->value.begin(), i->value.end());
					std::transform(logicop.begin(), logicop.end(), logicop.begin(), &_to_lower);
					
					// auto_ptr needed because of possible parse exceptions in build_expr.
					
					std::auto_ptr<ParseNode> left( build_expr(i->children.begin()) );
					std::auto_ptr<ParseNode> right( build_expr(i->children.begin()+1) );
					
					bool constleft = left->evaluate_const(NULL);
					bool constright = right->evaluate_const(NULL);
					
					// a logical node is constant if one of the two ops is constant. so we
					// construct a calculation node and check later.
					std::auto_ptr<PNBinaryLogicExpr> node( new PNBinaryLogicExpr(left.release(), right.release(), logicop) );
					
					if (constleft || constright)
					{
						Datum both;
						
						// test if the node is really const.
						if (node->evaluate_const(&both))
						{
							// return a constant node instead, node will be deleted by
							// auto_ptr, left,right by node's destructor.
							return new PNConstant(both);
						}
					}
					if (constleft)
					{
						// left node is constant, but the evaluation is not
						// -> only right node is meaningful.
						return node->detach_right();
					}
					if (constright)
					{
						// right node is constant, but the evaluation is not
						// -> only left node is meaningful.
						return node->detach_left();
					}
					
					return node.release();
				}
					
					// *** Variable and Function name place-holder
					
				case varname_id:
				{
					assert(i->children.size() == 0);
					
					std::string varname(i->value.begin(), i->value.end());
					
					return new PNVariable(varname);
				}
                    
                case list_literal_id:
                {
                    std::vector<const class ParseNode*> itemlist;
                    
                    if (i->children.size() > 0)
                    {
                        TreeIterT const& itemlistchild = i->children.begin();
                        
                        if (itemlistchild->value.id().to_long() == exprlist_id)
                        {
                            try
                            {
                                for(TreeIterT ci = itemlistchild->children.begin(); ci != itemlistchild->children.end(); ++ci)
                                {
                                    const ParseNode *pas = build_expr(ci);
                                    itemlist.push_back(pas);
                                }
                            }
                            catch (...) // need to clean-up
                            {
                                for(unsigned int i = 0; i < itemlist.size(); ++i)
                                    delete itemlist[i];
                                throw;
                            }
                        }
                        else
                        {
                            // just one subnode and its not a full expression list
                            itemlist.push_back( build_expr(itemlistchild) );
                        }
                    }
                    
                    return new PNListLiteral(itemlist);
                }
                    
                case dict_literal_id:
                {
                    PNDictLiteral::itemlist_type itemlist;
                    
                    for (TreeIterT ci = i->children.begin(); ci != i->children.end(); ++ci) {
                        try {
                            const ParseNode *keyNode = build_expr(ci);
                            const ParseNode *valueNode = build_expr(++ci);
                            itemlist.emplace_back(keyNode, valueNode);
                        } catch (...)  {
                            for (unsigned int i = 0; i < itemlist.size(); ++i) {
                                delete itemlist[i].first;
                                delete itemlist[i].second;
                            }
                            throw;
                        }
                    }
                    
                    return new PNDictLiteral(itemlist);
                }
					
				case function_identifier_id:
				{
					std::string funcname(i->value.begin(), i->value.end());
					std::vector<const class ParseNode*> paramlist;
					
					if (i->children.size() > 0)
					{
						TreeIterT const& paramlistchild = i->children.begin();
						
						if (paramlistchild->value.id().to_long() == exprlist_id)
						{
							try
							{
								for(TreeIterT ci = paramlistchild->children.begin(); ci != paramlistchild->children.end(); ++ci)
								{
									const ParseNode *pas = build_expr(ci);
									paramlist.push_back(pas);
								}
							}
							catch (...) // need to clean-up
							{
								for(unsigned int i = 0; i < paramlist.size(); ++i)
									delete paramlist[i];
								throw;
							}
						}
						else
						{
							// just one subnode and its not a full expression list
							paramlist.push_back( build_expr(paramlistchild) );
						}
					}
					
					return new PNFunction(funcname, paramlist);
				}

				case range_expr_id:
				{
					assert(i->children.size() == 2);
					
					// auto_ptr needed because of possible parse exceptions in build_expr.
					std::auto_ptr<const ParseNode> start( build_expr(i->children.begin()) );
					std::auto_ptr<const ParseNode> stop( build_expr(i->children.begin()+1) );
                    
                    return new PNRangeExpr(start.release(), stop.release());
				}
					
				default:
					throw(ExpressionParserException("Unknown AST parse tree node found. This should never happen."));
			}
		}
		
		/// build_exprlist constructs the vector holding the ParseNode parse tree for
		/// each parse tree.
		ParseTreeList build_exprlist(TreeIterT const &i)
		{
#ifdef STX_DEBUG_PARSER
			std::cout << "In build_exprlist. i->value = " <<
			std::string(i->value.begin(), i->value.end()) <<
			" i->children.size() = " << i->children.size() << 
			" i->value.id = " << i->value.id().to_long() << std::endl;
#endif
			
			ParseTreeList ptlist;
            
            if (i->value.id().to_long() == exprlist_id) {
                for(TreeIterT ci = i->children.begin(); ci != i->children.end(); ++ci)
                {
                    ParseNode *vas = build_expr(ci);
                    
                    ptlist.push_back( ParseTree(vas) );
                }
            } else {
                ptlist.push_back( ParseTree(build_expr(i)) );
            }
			
			return ptlist;
		}
		
		/// Uses boost::spirit::classic function to convert the parse tree into a XML document.
		static inline void tree_dump_xml(std::ostream &os, const std::string &input, const tree_parse_info<InputIterT> &info)
		{
			// map used by the xml dumper to label the nodes
			
			std::map<parser_id, std::string> rule_names;
			
			rule_names[boolean_const_id] = "boolean_const";
			rule_names[integer_const_id] = "integer_const";
			rule_names[long_const_id] = "long_const";
			rule_names[double_const_id] = "double_const";
			rule_names[string_const_id] = "string_const";
			rule_names[constant_id] = "constant";
            
            rule_names[list_literal_id] = "list_literal";
            rule_names[dict_literal_id] = "dict_literal";
			
			rule_names[function_call_id] = "function_call";
			rule_names[function_identifier_id] = "function_identifier";
			
			rule_names[varname_id] = "varname";
			
			rule_names[units_expr_id] = "units_expr";
            rule_names[subscript_expr_id] = "subscript_expr";
			rule_names[unary_expr_id] = "unary_expr";
			rule_names[mul_expr_id] = "mul_expr";
			rule_names[add_expr_id] = "add_expr";
			
			rule_names[cast_expr_id] = "cast_expr";
			rule_names[cast_spec_id] = "cast_spec";
			
			rule_names[comp_expr_id] = "comp_expr";
			rule_names[and_expr_id] = "and_expr";
			rule_names[or_expr_id] = "or_expr";
			
			rule_names[expr_id] = "expr";
			
			rule_names[range_expr_id] = "range_expr";
			
			rule_names[exprlist_id] = "exprlist";
			
			tree_to_xml(os, info.trees, input.c_str(), rule_names);
		}
		
	} // namespace Grammar
	
	const ParseTree parseExpression(const std::string &input)
	{
		// instance of the grammar
		Grammar::ExpressionGrammar g;
		
#ifdef STX_DEBUG_PARSER
		BOOST_SPIRIT_DEBUG_GRAMMAR(g);
#endif
		
		Grammar::tree_parse_info<Grammar::InputIterT> info =
		boost::spirit::classic::ast_parse(input.begin(), input.end(),
								 g.use_parser<0>(),	// use first entry point: expr
								 boost::spirit::classic::space_p);
		
		if (!info.full)
		{
			std::ostringstream oss;
			oss << "Syntax error at position "
			<< static_cast<int>(info.stop - input.begin())
			<< " near " 
			<< std::string(info.stop, input.end());
			
			throw(BadSyntaxException(oss.str()));
		}
		
		return ParseTree( Grammar::build_expr(info.trees.begin()) );
	}
	
	std::string parseExpressionXML(const std::string &input)
	{
		// instance of the grammar
		Grammar::ExpressionGrammar g;
		
#ifdef STX_DEBUG_PARSER
		BOOST_SPIRIT_DEBUG_GRAMMAR(g);
#endif
		
		Grammar::tree_parse_info<Grammar::InputIterT> info =
		boost::spirit::classic::ast_parse(input.begin(), input.end(),
								 g.use_parser<0>(),	// use first entry point: expr
								 boost::spirit::classic::space_p);
		
		if (!info.full)
		{
			std::ostringstream oss;
			oss << "Syntax error at position "
			<< static_cast<int>(info.stop - input.begin())
			<< " near " 
			<< std::string(info.stop, input.end());
			
			throw(BadSyntaxException(oss.str()));
		}
		
		std::ostringstream oss;
		Grammar::tree_dump_xml(oss, input, info);
		return oss.str();
	}
	
	ParseTreeList parseExpressionList(const std::string &input)
	{
		// instance of the grammar
		Grammar::ExpressionGrammar g;
		
#ifdef STX_DEBUG_PARSER
		BOOST_SPIRIT_DEBUG_GRAMMAR(g);
#endif
		
		Grammar::tree_parse_info<Grammar::InputIterT> info =
		boost::spirit::classic::ast_parse(input.begin(), input.end(),
								 g.use_parser<1>(),	// use second entry point: exprlist
								 boost::spirit::classic::space_p);
		
		if (!info.full)
		{
			std::ostringstream oss;
			oss << "Syntax error at position "
			<< static_cast<int>(info.stop - input.begin())
			<< " near " 
			<< std::string(info.stop, input.end());
			
			throw(BadSyntaxException(oss.str()));
		}
		
		return Grammar::build_exprlist(info.trees.begin());
	}
	
	void ParseTreeList::evaluate(std::vector<Datum> &values, const class SymbolTable &st) const
	{
		for(parent_type::const_iterator i = parent_type::begin(); i != parent_type::end(); i++)
		{
			i->evaluate(values, st);
		}
	}
	
	std::string ParseTreeList::toString() const
	{
		std::string sl;
		
		for(parent_type::const_iterator i = parent_type::begin(); i != parent_type::end(); i++)
		{
			if (i != parent_type::begin()) {
				sl += ", ";
			}
			
			sl += i->toString();
		}
		
		return sl;
	}
	
	/// *** SymbolTable, EmptySymbolTable and BasicSymbolTable implementation
	
	SymbolTable::~SymbolTable()
	{
	}
	
	EmptySymbolTable::~EmptySymbolTable()
	{
	}
	
	Datum EmptySymbolTable::lookupVariable(const std::string &varname) const
	{
		throw(UnknownSymbolException(std::string("Unknown variable ") + varname));
	}
	
	Datum EmptySymbolTable::processFunction(const std::string &funcname,
												const paramlist_type &) const
	{
		throw(UnknownSymbolException(std::string("Unknown function ") + funcname + "()"));
	}
	
	BasicSymbolTable::BasicSymbolTable()
	{
		addStandardFunctions();
	}
	
	BasicSymbolTable::~BasicSymbolTable()
	{
	}
	
	void BasicSymbolTable::setVariable(const std::string& varname, const Datum &value)
	{
		std::string vn = varname;
		std::transform(vn.begin(), vn.end(), vn.begin(), _to_lower);
		
		variablemap[vn] = value;
	}
	
	void BasicSymbolTable::setFunction(const std::string& funcname, int arguments, functionptr_type funcptr)
	{
		std::string fn = funcname;
		std::transform(fn.begin(), fn.end(), fn.begin(), _to_upper);
		
		functionmap[fn] = FunctionInfo(arguments, funcptr);
	}
	
	void BasicSymbolTable::clearVariables()
	{
		variablemap.clear();
	}
	
	void BasicSymbolTable::clearFunctions()
	{
		functionmap.clear();
	}
	
	Datum BasicSymbolTable::funcPI(const paramlist_type &)
	{
		return Datum(3.14159265358979323846);
	}
	
	Datum BasicSymbolTable::funcSIN(const paramlist_type &paramlist)
	{
		return Datum( std::sin(paramlist[0].getFloat()) );
	}
	
	Datum BasicSymbolTable::funcCOS(const paramlist_type &paramlist)
	{
		return Datum( std::cos(paramlist[0].getFloat()) );
	}
	
	Datum BasicSymbolTable::funcTAN(const paramlist_type &paramlist)
	{
		return Datum( std::tan(paramlist[0].getFloat()) );
	}
	
	Datum BasicSymbolTable::funcABS(const paramlist_type &paramlist)
	{
		if (paramlist[0].isInteger()) {
			return Datum( std::abs(paramlist[0].getInteger()) );
		}
		else if (paramlist[0].isFloat()) {
			return Datum( std::fabs(paramlist[0].getFloat()) );
		}
		else {
			throw(BadFunctionCallException("Function ABS() requires a numeric parameter"));
		}
	}
	
	Datum BasicSymbolTable::funcEXP(const paramlist_type &paramlist)
	{
		return Datum( std::exp(paramlist[0].getFloat()) );
	}
	
	Datum BasicSymbolTable::funcLOGN(const paramlist_type &paramlist)
	{
		return Datum( std::log(paramlist[0].getFloat()) );
	}
	
	Datum BasicSymbolTable::funcPOW(const paramlist_type &paramlist)
	{
		return Datum( std::pow(paramlist[0].getFloat(), paramlist[1].getFloat()) );
	}
	
	Datum BasicSymbolTable::funcSQRT(const paramlist_type &paramlist)
	{
		return Datum( std::sqrt(paramlist[0].getFloat()) );
	}
	
	Datum BasicSymbolTable::funcCEIL(const paramlist_type &paramlist)
	{
		return Datum( std::ceil(paramlist[0].getFloat()) );
	}
	
	Datum BasicSymbolTable::funcFLOOR(const paramlist_type &paramlist)
	{
		return Datum( std::floor(paramlist[0].getFloat()) );
	}
	
	Datum BasicSymbolTable::funcROUND(const paramlist_type &paramlist)
	{
		return Datum( boost::math::round(paramlist[0].getFloat()) );
	}
	
	Datum BasicSymbolTable::funcMIN(const paramlist_type &paramlist)
	{
        auto &first = paramlist[0];
        auto &second = paramlist[1];
        return ((first < second) ? first : second);
	}
	
	Datum BasicSymbolTable::funcMAX(const paramlist_type &paramlist)
	{
        auto &first = paramlist[0];
        auto &second = paramlist[1];
        return ((first > second) ? first : second);
	}
	
	static boost::mt19937 rng;
	static bool seeded;
	static void seed_rng(){
		boost::shared_ptr <mw::Clock> clock = mw::Clock::instance();
		rng.seed((int)clock->getCurrentTimeNS());
		seeded = true;
	}
	
	Datum BasicSymbolTable::funcNOW(const paramlist_type &paramlist)
	{
		boost::shared_ptr <mw::Clock> clock = mw::Clock::instance();
		return Datum( clock->getCurrentTimeUS() );
	}
	
	Datum BasicSymbolTable::funcTIMER_EXPIRED(const paramlist_type &paramlist)
	{
		return Datum( paramlist[0].getInteger() );
	}
	
	Datum BasicSymbolTable::funcREFRESH_RATE(const paramlist_type &paramlist)
	{
        boost::shared_ptr<mw::StimulusDisplay> display(mw::StimulusDisplay::getCurrentStimulusDisplay());
		return Datum( display->getMainDisplayRefreshRate() );
	}
	
	Datum BasicSymbolTable::funcNEXT_FRAME_TIME(const paramlist_type &paramlist)
    {
        boost::shared_ptr<mw::StimulusDisplay> display(mw::StimulusDisplay::getCurrentStimulusDisplay());
        return Datum( display->getCurrentOutputTimeUS() );
    }
	
	Datum BasicSymbolTable::funcFORMAT(const paramlist_type &paramlist)
	{
        if (paramlist.size() < 1) {
            throw BadFunctionCallException("Function FORMAT() requires at least one parameter");
        }
        
        if (!(paramlist[0].isString())) {
            throw BadFunctionCallException("First parameter to function FORMAT() must be a string");
        }
        
        try {
            boost::format fmt(paramlist[0].getString());
            for (paramlist_type::size_type i = 1; i < paramlist.size(); i++) {
                fmt % paramlist[i];
            }
            return Datum( fmt.str() );
        } catch (boost::io::format_error &e) {
            throw BadFunctionCallException(std::string("Error in function FORMAT(): ") + e.what());
        }
	}
    
    Datum BasicSymbolTable::funcSELECTION(const paramlist_type& paramlist)
    {
        if (!(paramlist[0].isString())) {
            throw BadFunctionCallException("First argument to function SELECTION() must be a string");
        }
        
        auto reg = mw::ComponentRegistry::getSharedRegistry();
        auto sel = reg->getObject<mw::SelectionVariable>(paramlist[0].getString());
        
        if (!sel) {
            throw BadFunctionCallException("First argument to function SELECTION() must be the name of a selection variable");
        }
        
        return sel->getTentativeSelection(paramlist[1].getInteger());
    }
    
    Datum BasicSymbolTable::funcNUMACCEPTED(const paramlist_type& paramlist)
    {
        if (!(paramlist[0].isString())) {
            throw BadFunctionCallException("Argument to function NUMACCEPTED() must be a string");
        }
        
        boost::shared_ptr<mw::ComponentRegistry> reg = mw::ComponentRegistry::getSharedRegistry();
        boost::shared_ptr<mw::Selectable> selectable = reg->getObject<mw::Selectable>(paramlist[0].getString());
        
        if (!selectable) {
            throw BadFunctionCallException("Argument to function NUMACCEPTED() must be the name of a selectable object");
        }
        
        return Datum( selectable->getNAccepted() );
    }
	
	Datum BasicSymbolTable::funcUNIFORM_RAND(const paramlist_type &paramlist)
	{
		double hi, lo;
		
		if(!seeded){
			seed_rng();	
		}
		
		if(paramlist.size() == 0){
			hi = 1.0;
			lo = 0.0;
		} else {
			lo = paramlist[0].getFloat();
			hi = paramlist[1].getFloat();
			
			if(hi == lo) {
				return Datum(hi);
			}
			
			if(hi < lo) {
				mw::merror(mw::M_GENERIC_MESSAGE_DOMAIN,"Trying to evaluate discrete uniform random variable with high limit <= low limit.  Returning 0.");
				return Datum(0);
			}
		}
		
		boost::uniform_real<> uni_dist(lo, hi);
		boost::variate_generator<boost::mt19937&, boost::uniform_real<> > uni(rng, uni_dist);
		return Datum( uni() );
	}
	
	Datum BasicSymbolTable::funcDISC_UNIFORM_RAND(const paramlist_type &paramlist)
	{
		// go get current values of high and low ranges (these might be variables)
		long lo = paramlist[0].getInteger();
		long hi = paramlist[1].getInteger();
		
		long value;
		
		if(!seeded){
			seed_rng();	
		}
		
		
		if (hi>=lo) {
			boost::uniform_int<> disc_range(lo,hi);
			boost::variate_generator<boost::mt19937&, boost::uniform_int<> > disc_rand(rng, disc_range);             // glues randomness with mapping
			value = disc_rand();
		}
		else {
			mw::merror(mw::M_GENERIC_MESSAGE_DOMAIN,"Trying to evaluate discrete uniform random variable with high limit <= low limit.  Returning 0.");
			value = 0;
		}
		
		return Datum( value );
	}
	
	
	Datum BasicSymbolTable::funcGEOM_RAND(const paramlist_type &paramlist)
	{
		
		long value;
		
		if(!seeded){
			seed_rng();	
		}
		
		// go get current values of input parameters (these might be variables)
		double Bernoulli_prob = paramlist[0].getFloat();
		long high = paramlist[1].getFloat();
		
		// Discrete random number sampled in the interval [0, high] from a GEOMETRICAL DISTRIBUTION with constant Bernoulli probability = Bernoulli_prob
		// (Monte Carlo sampling algorithm taken from Rubinstain).
		// The distribution is truncated at high through the do-while loop, which resample the number of distractors
		// to be presented if the current sample is > high. This is equivalent to renormalize the truncated distribution
		// on the allowed values.
		if (high > 0) {
			
			if( Bernoulli_prob <= 1 && Bernoulli_prob > 0 ) {
				boost::uniform_real<> uni_dist(0,1);
				boost::variate_generator<boost::mt19937&, boost::uniform_real<> > uni(rng, uni_dist);
				do{
					value = floor( log( uni() ) / log( 1-Bernoulli_prob ) ); 	
				} while(  value > high );	
				
			} else if( Bernoulli_prob > 1 ){ 
				mw::mwarning(mw::M_GENERIC_MESSAGE_DOMAIN,"Trying to evaluate discrete geometric random variable with Bernoulli prob > 1.  Returning 0.");
				value = 0;	// As if Bernoulli_prob was = 1
			} else if( Bernoulli_prob <= 0 ){
				value = high;	// As if Bernoulli_prob was = 0; return range axtreme				
				mw::mwarning(mw::M_GENERIC_MESSAGE_DOMAIN,"Trying to evaluate discrete geometric random variable with Bernoulli prob <= 0.  Returning range high limit.");
			}
		} else {
			mw::mwarning(mw::M_GENERIC_MESSAGE_DOMAIN,"Trying to evaluate discrete geometric random variable with high limit <= low limit.  Returning 0.");
			value = 0;
		}
		return Datum( value );
	}
    
    Datum BasicSymbolTable::funcEXP_RAND(const paramlist_type &paramlist)
    {
        if (paramlist.size() < 1 || paramlist.size() > 3) {
            throw BadFunctionCallException("Function EXP_RAND() requires 1 to 3 parameters");
        }
        
        double beta = paramlist[0].getFloat();
        double minVal = 0.0;
        double maxVal = HUGE_VAL;
        
        if (paramlist.size() > 1) {
            minVal = paramlist[1].getFloat();
            if (paramlist.size() > 2) {
                maxVal = paramlist[2].getFloat();
            }
        }
        
        if (beta <= 0.0) {
            throw BadFunctionCallException("First parameter to function EXP_RAND() must be a positive number");
        }
        if (minVal < 0.0) {
            throw BadFunctionCallException("Second parameter to function EXP_RAND() must be a non-negative number");
        }
        if (maxVal <= minVal) {
            throw BadFunctionCallException("Third parameter to function EXP_RAND() must be greater than second parameter");
        }
        
        if (!seeded) {
            seed_rng();
        }
        
        boost::random::uniform_real_distribution<> randDist;  // [0, 1)
        constexpr int maxDraws = 100;
        
        for (int i = 0; i < maxDraws; i++) {
            double result = 1.0 - randDist(rng);  // Include 1, exclude 0
            result = minVal - beta * std::log(result);
            if (result < maxVal) {
                return Datum( result );
            }
        }
        
        mw::merror(mw::M_GENERIC_MESSAGE_DOMAIN,
                   "Failed to generate an exponential random value less than %g after %d tries.  Returning %g.",
                   maxVal,
                   maxDraws,
                   maxVal);
        
        return Datum( maxVal );
    }
	
	
	
	void BasicSymbolTable::addStandardFunctions()
	{
		setFunction("PI", 0, funcPI);
		
		setFunction("SIN", 1, funcSIN);
		setFunction("COS", 1, funcCOS);
		setFunction("TAN", 1, funcTAN);
		
		setFunction("ABS", 1, funcABS);
		setFunction("EXP", 1, funcEXP);
		setFunction("LOGN", 1, funcLOGN);
		setFunction("POW", 2, funcPOW);
		setFunction("SQRT", 1, funcSQRT);
		
		setFunction("CEIL", 1, funcCEIL);
		setFunction("FLOOR", 1, funcFLOOR);
		setFunction("ROUND", 1, funcROUND);

		setFunction("MIN", 2, funcMIN);
		setFunction("MAX", 2, funcMAX);
		
		setFunction("RAND", 0, funcUNIFORM_RAND);
		setFunction("RAND", 2, funcUNIFORM_RAND);
		setFunction("DISC_RAND", 2, funcDISC_UNIFORM_RAND);
		setFunction("GEOM_RAND", 2, funcGEOM_RAND);
        setFunction("EXP_RAND", -1, funcEXP_RAND);
		
		setFunction("NOW", 0, funcNOW);
		setFunction("TIMEREXPIRED", 1, funcTIMER_EXPIRED);
		setFunction("REFRESHRATE", 0, funcREFRESH_RATE);
		setFunction("NEXTFRAMETIME", 0, funcNEXT_FRAME_TIME);
        
        setFunction("SELECTION", 2, funcSELECTION);
        setFunction("NUMACCEPTED", 1, funcNUMACCEPTED);
        
		setFunction("FORMAT", -1, funcFORMAT);
	}
	
	Datum BasicSymbolTable::lookupVariable(const std::string &_varname) const
	{
		std::string varname = _varname;
		std::transform(varname.begin(), varname.end(), varname.begin(), _to_lower);
		
		variablemap_type::const_iterator fi = variablemap.find(varname);
		
		if (fi != variablemap.end())
		{
			return fi->second;
		}
		
		throw(UnknownSymbolException(std::string("Unknown variable ") + varname));
	}
	
	Datum BasicSymbolTable::processFunction(const std::string &_funcname,
												const paramlist_type &paramlist) const
	{
		std::string funcname = _funcname;
		std::transform(funcname.begin(), funcname.end(), funcname.begin(), _to_upper);
		
		functionmap_type::const_iterator fi = functionmap.find(funcname);
		
		if (fi != functionmap.end())
		{
			if (fi->second.arguments >= 0)
			{
				if (fi->second.arguments == 0 && paramlist.size() != 0)
				{
					throw(BadFunctionCallException(std::string("Function ") + funcname + "() does not take any parameter."));
				}
				else if (fi->second.arguments == 1 && paramlist.size() != 1)
				{
					throw(BadFunctionCallException(std::string("Function ") + funcname + "() takes exactly one parameter."));
				}
				else if (static_cast<unsigned int>(fi->second.arguments) != paramlist.size())
				{
					std::ostringstream oss;
					oss << "Function " << funcname << "() takes exactly " << fi->second.arguments << " parameters.";
					throw(BadFunctionCallException(oss.str()));
				}
			}
			return fi->second.func(paramlist);
		}
		
		throw(UnknownSymbolException(std::string("Unknown function ") + funcname + "()"));
	}
	
} // namespace stx
