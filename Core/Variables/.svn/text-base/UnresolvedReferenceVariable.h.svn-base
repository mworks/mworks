/*
 *  UnresolvedReferenceVariable.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 1/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _UNRESOLVED_REFERENCE_VARIABLE
#define _UNRESOLVED_REFERENCE_VARIABLE

#include "GenericVariable.h"
#include "ConstantVariable.h"
#include "ExpressionVariable.h"
#include "ComponentRegistry_new.h"
#include <string>
#include <sstream>
namespace mw {
using namespace std;


class UnresolvedVariableString {

	protected:
		std::vector <shared_ptr<Variable> >stringFragments;

	public:
	
		UnresolvedVariableString(const std::string &unresolved_string) {
		
			std::string outStr(unresolved_string);	
			
			stringFragments.clear();
			
			while(outStr.find_first_of("$") != std::string::npos) {
				std::string stringSegment;
				std::string varName;
				istringstream parser(outStr);
				
				getline(parser, stringSegment, '$');
				shared_ptr<ConstantVariable> c(new ConstantVariable(Data(stringSegment)));
				stringFragments.push_back(c);
				
				getline(parser, varName, ' ');
				shared_ptr<Variable> var = GlobalVariableRegistry->getVariable(varName);
				if(!var) {
					var = shared_ptr<ConstantVariable>(new ConstantVariable(Data(std::string("UNKNOWNVAR"))));			
				}
				
				stringFragments.push_back(var);
				
				outStr.erase(0, stringSegment.size() + varName.size() + 1);
			}
			
			// add any remainder
			shared_ptr<ConstantVariable> remainder(new ConstantVariable(Data(outStr)));
			stringFragments.push_back(remainder);
		}


		string resolve() {
			std::string outStr("");
			
			for(std::vector<shared_ptr<Variable> >::iterator i=stringFragments.begin();
				i != stringFragments.end();
				++i) {
				
				shared_ptr<Variable> var = *i;
				
				outStr.append(var->getValue().toString());		
			}
			
			return outStr;
		}


};

class UnresolvedReferenceVariable : public Variable {

	protected:
	
		weak_ptr<mwComponentRegistry> registry;
		string expression;
		UnresolvedVariableString unresolved_string;
		
	
	public:

		UnresolvedReferenceVariable (string _expression, shared_ptr<mwComponentRegistry> _registry) :
										expression(_expression),
										unresolved_string(_expression){
										
			registry = weak_ptr<mwComponentRegistry>(_registry);
		}
		
		
		virtual Data getValue(){
			
			string resolved_string = unresolved_string.resolve();
			
			if(!registry.expired()){
				shared_ptr<mwComponentRegistry> registry_shared(registry);
				shared_ptr<Variable> resolved_variable = registry_shared->getVariable(resolved_string);
				
				if(resolved_variable != NULL){
					return resolved_variable->getValue();
				}
			}
			
			return Data();
		}
		
		
		virtual void setValue(Data val){ return; }
		virtual void setValue(Data val, MonkeyWorksTime time){ return; }
		virtual void setSilentValue(Data _value){ return; }
		
		/**
		 *  A polymorphic copy constructor (inherited from Clonable)
		 */
		virtual Variable *clone(){
			UnresolvedReferenceVariable *returned = 
					new UnresolvedReferenceVariable((const UnresolvedReferenceVariable&)(*this));
			return (Variable *)returned;
		}

		

};
}
#endif
