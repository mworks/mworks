#ifndef SELECTION_VARIABLE_H_
#define SELECTION_VARIABLE_H_

/*
 *  SelectionVariable.h
 *  MWorksCore
 *
 *  Created by David Cox on 3/1/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include <vector>

#include "Selection.h"
#include "Selectable.h"
#include "GenericVariable.h"
#include "ComponentFactory.h"


BEGIN_NAMESPACE_MW


class SelectionVariable :  public Selectable, public Variable {
    
private:
    static const int NO_SELECTION = -1;
    
    std::vector< shared_ptr<Variable> > values;
    int selected_index;
    
public:
	SelectionVariable(VariableProperties *props);
	SelectionVariable(VariableProperties *props, shared_ptr<Selection> _sel);
    
    virtual ~SelectionVariable() { }
    
	virtual void addValue(shared_ptr<Variable> _var) {
		values.push_back(_var);
		if (selection != NULL) {
			selection->reset();
		}
	}
	
	virtual shared_ptr<Variable> getValue(int i) {
		return values[i];
	}
	
	virtual void nextValue();
	
    // Variable overrides
	virtual Variable *clone();
	virtual Datum getValue();
	virtual void setValue(Datum data) { }
	virtual void setValue(Datum data, MWTime time) { }
	virtual void setSilentValue(Datum data) { }
    
    //
    // Selectable overrides
    //
    
	virtual int getNItems() { return values.size(); }
    
	virtual void resetSelections() {
        Selectable::resetSelections();
		selected_index = NO_SELECTION;
	}
	
	void rejectSelections() {
        Selectable::rejectSelections();
		nextValue();
	}
    
};


class SelectionVariableFactory : public ComponentFactory {
    
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                   ComponentRegistry *reg);
    
};


END_NAMESPACE_MW


#endif


























