/*
 *  ExpressionVariable.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 2/20/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "ExpressionVariable.h"


BEGIN_NAMESPACE_MW


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



Datum ExpressionVariable::getExpressionValue(){

	switch(op){
	
		case M_PLUS:
			return v1->operator Datum() + v2->operator Datum();
		case M_MINUS:
			return v1->operator Datum() - v2->operator Datum();
		case M_TIMES:
			return v1->operator Datum() * v2->operator Datum();
		case M_DIVIDE:
			return v1->operator Datum() / v2->operator Datum();
		case M_MOD:
			return v1->operator Datum() % v2->operator Datum();
		
		case M_INCREMENT:
			// TODO: side effect?
			return v1->operator Datum() + Datum((long)1);
		case M_DECREMENT:
			// TODO: side effect?
			return v1->operator Datum() - Datum((long)1);

		case M_IS_EQUAL:
			return v1->operator Datum() == v2->operator Datum();					
		case M_IS_NOT_EQUAL:
			return (v1->operator Datum() != v2->operator Datum());						
		case M_IS_GREATER_THAN:
			return v1->operator Datum() > v2->operator Datum();
		case M_IS_LESS_THAN:
			return v1->operator Datum() < v2->operator Datum();	
		case M_IS_GREATER_THAN_OR_EQUAL:
			return v1->operator Datum() >= v2->operator Datum();
		case M_IS_LESS_THAN_OR_EQUAL:
			return v1->operator Datum() <= v2->operator Datum();
			
		case M_AND:
			return v1->operator Datum() && v2->operator Datum();
		case M_OR:
			return v1->operator Datum() || v2->operator Datum();
		case M_NOT:
			return !(v1->operator Datum());	
		case M_UNARY_MINUS:
			return (v1->operator Datum()) * Datum(-1L);	
		case M_UNARY_PLUS:
			return (v1->operator Datum());	
		// TODO: casts? (will require parser magic)
		default:
			return Datum();
	}
}


END_NAMESPACE_MW
