/*
 *  ExpressionVariable.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 2/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "ExpressionVariable.h"
#include "boost/algorithm/string/trim.hpp"
using namespace mw;



ExpressionVariable::ExpressionVariable(Variable *_v1, 
										 Variable *_v2, 
										 Operator _op):
											Variable(){
	
	
	if(_v1 != NULL){
		v1 = _v1;
		//v1 = (Variable *)(_v1->clone());
	} else {
		v1 = NULL; // shouldn't happen TODO
	}
	
	if(_v2 != NULL){
		v2 = _v2;
		//v2 = (Variable *)(_v2->clone());
	} else {
		v2 = NULL;
	}
	op = _op;
}



Data ExpressionVariable::getExpressionValue(){

	switch(op){
	
		case M_PLUS:
			return v1->operator Data() + v2->operator Data();
		case M_MINUS:
			return v1->operator Data() - v2->operator Data();
		case M_TIMES:
			return v1->operator Data() * v2->operator Data();
		case M_DIVIDE:
			return v1->operator Data() / v2->operator Data();
		case M_MOD:
			return v1->operator Data() % v2->operator Data();
		
		case M_INCREMENT:
			// TODO: side effect?
			return v1->operator Data() + Data((long)1);
		case M_DECREMENT:
			// TODO: side effect?
			return v1->operator Data() - Data((long)1);

		case M_IS_EQUAL:
			return v1->operator Data() == v2->operator Data();					
		case M_IS_NOT_EQUAL:
			return (v1->operator Data() != v2->operator Data());						
		case M_IS_GREATER_THAN:
			return v1->operator Data() > v2->operator Data();
		case M_IS_LESS_THAN:
			return v1->operator Data() < v2->operator Data();	
		case M_IS_GREATER_THAN_OR_EQUAL:
			return v1->operator Data() >= v2->operator Data();
		case M_IS_LESS_THAN_OR_EQUAL:
			return v1->operator Data() <= v2->operator Data();
			
		case M_AND:
			return v1->operator Data() && v2->operator Data();
		case M_OR:
			return v1->operator Data() || v2->operator Data();
		case M_NOT:
			return !(v1->operator Data());	
		case M_UNARY_MINUS:
			return (v1->operator Data()) * Data(-1L);	
		case M_UNARY_PLUS:
			return (v1->operator Data());	
		// TODO: casts? (will require parser magic)
		default:
			return Data();
	}
}



ParsedExpressionVariable::ParsedExpressionVariable(string _expression_string) : Variable(),
												original_expression(_expression_string){
	
	string expression_string = _expression_string;
	
	// substitute #GT, #LT, et al.
	boost::replace_all(expression_string, "#GT", ">");
	boost::replace_all(expression_string, "#LT", "<");
	boost::replace_all(expression_string, "#GE", ">=");
	boost::replace_all(expression_string, "#LE", "<=");
	
	boost::replace_all(expression_string, "#AND", "&&");
	boost::replace_all(expression_string, "#OR", "||");
	
	std::string dest_string;

	boost::regex_replace(std::back_inserter(dest_string),
				expression_string.begin(), expression_string.end(),
				boost::regex("(.*?[^a-zA-Z])us(\\s*$)", boost::regex::perl|boost::regex::icase),
				"$1 * 1");
	expression_string = dest_string; dest_string = "";

	boost::regex_replace(std::back_inserter(dest_string),
				expression_string.begin(), expression_string.end(),
				boost::regex("(.*?[^a-zA-Z])ms(\\s*$)", boost::regex::perl|boost::regex::icase),
				"$1 * 1000");
	expression_string = dest_string; dest_string = "";

	boost::regex_replace(std::back_inserter(dest_string),
				expression_string.begin(), expression_string.end(),
				boost::regex("(.*?[^a-zA-Z])s(\\s*$)", boost::regex::perl|boost::regex::icase),
				"$1 * 1000000");
	expression_string = dest_string; dest_string = "";
	
				boost::regex_replace(std::back_inserter(dest_string),
				expression_string.begin(), expression_string.end(),
				boost::regex("(^|[^a-zA-Z])YES($|[^a-zA-Z])", boost::regex::perl|boost::regex::icase),
				"$1 1 $2");
	expression_string = dest_string; dest_string = "";
	
	
	boost::regex_replace(std::back_inserter(dest_string),
				expression_string.begin(), expression_string.end(),
				boost::regex("(^|[^a-zA-Z])NO($|[^a-zA-Z])", boost::regex::perl|boost::regex::icase),
				"$1 0 $2");
	expression_string = dest_string; dest_string = "";
	
	boost::regex_replace(std::back_inserter(dest_string),
				expression_string.begin(), expression_string.end(),
				boost::regex("(^|[^a-zA-Z])true($|[^a-zA-Z])", boost::regex::perl|boost::regex::icase),
				"$1 1 $2");
	expression_string = dest_string; dest_string = "";
	
	boost::regex_replace(std::back_inserter(dest_string),
				expression_string.begin(), expression_string.end(),
				boost::regex("(^|[^a-zA-Z])false($|[^a-zA-Z])", boost::regex::perl|boost::regex::icase),
				"$1 0 $2");
	expression_string = boost::algorithm::trim_copy(dest_string); 
	dest_string = "";
	
	cerr << "expression: " << expression_string << endl;
	//cerr << "dest string: " << dest_string << endl;
	
	try {
		expression_tree = stx::parseExpression(expression_string);
		std::cout << "parsed: " << expression_tree.toString() << "\n";
	} catch (stx::ExpressionParserException &e){
		std::cout << "ExpressionParserException: " << e.what() << "\n";
		throw e;
	}
													
	getValue();												
}
