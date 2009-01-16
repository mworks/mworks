#ifndef SELECTION_VARIABLE_H_
#define SELECTION_VARIABLE_H_

/*
 *  SelectionVariable.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/1/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "Selection.h"
#include "Selectable.h"
#include "GenericVariable.h"
#include "ComponentFactory.h"
namespace mw {

class SelectionVariable :  public Selectable, public Variable {

protected:

	ExpandableList<Variable> values;
	shared_ptr<Variable> selected_value;
public:

	SelectionVariable(VariableProperties *props);
	SelectionVariable(VariableProperties *props, shared_ptr<Selection> _sel);
	
		
	//mSelectionVariable(const SelectionVariable& tocopy);
		
				
	virtual ~SelectionVariable(){
		values.releaseElements();
		
	}
	

	virtual void addValue(shared_ptr<Variable> _var){
		values.addReference(_var);
		if(selection != NULL){
			selection->reset();
		}
	}
	
	virtual shared_ptr<Variable> getValue(int i){
		return values[i];
	}
	
	
	// A polymorphic copy constructor
	virtual Variable *clone();
	
	virtual void nextValue();
	
	virtual Data getValue();
	virtual void setValue(Data data){  return; }
	virtual void setValue(Data data, MonkeyWorksTime time){ return; }
	virtual void setSilentValue(Data data){  return; }
	
	virtual int getNChildren(){ return values.getNElements();  }
	
			
	// From Selectable
	virtual int getNItems(){ return getNChildren(); }
	virtual void resetSelections() {
		selected_value = shared_ptr<Variable>();
		selection->reset();
	}
	
	
	void rejectSelections() {
		selection->rejectSelections();
		this->nextValue();
	}

};


class SelectionVariableFactory : public ComponentFactory {
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
												mwComponentRegistry *reg);
};
}
#endif
