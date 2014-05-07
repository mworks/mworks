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


class SelectionVariable : public Selectable, public Variable {
    
protected:
    static const int NO_SELECTION = -1;
    
    std::vector<Datum> values;
    int selected_index;
    bool advanceOnAccept;
    
public:
	SelectionVariable(VariableProperties *props, shared_ptr<Selection> _sel = shared_ptr<Selection>());
    
    virtual ~SelectionVariable() { }
    
    void setAdvanceOnAccept(bool val) {
        advanceOnAccept = val;
    }
    
    virtual void addValue(const Datum &val) {
        values.push_back(val);
		if (selection != NULL) {
			resetSelections();
		}
    }
    
	virtual void addValue(shared_ptr<Variable> var) {
        if (var) {
            addValue(var->getValue());
        }
	}
	
    virtual Datum getTentativeSelection(int index);
	virtual void nextValue();
	
    // Variable overrides
	virtual Variable *clone();
	virtual Datum getValue();
	virtual void setValue(Datum data) { }
	virtual void setValue(Datum data, MWTime time) { }
	virtual void setSilentValue(Datum data) { }
    bool isWritable() const MW_OVERRIDE { return false; }

    //
    // Selectable overrides
    //
    
    int getNItems() MW_OVERRIDE { return values.size(); }
    
    void resetSelections() MW_OVERRIDE {
        Selectable::resetSelections();
		selected_index = NO_SELECTION;
	}
	
	void rejectSelections() MW_OVERRIDE {
        Selectable::rejectSelections();
		nextValue();
	}

    void acceptSelections() MW_OVERRIDE {
        Selectable::acceptSelections();
        if (advanceOnAccept && (getNLeft() > 0)) {
            nextValue();
        }
    }

};


class SelectionVariableFactory : public ComponentFactory {
    
	virtual shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                                   ComponentRegistry *reg);
    
};


END_NAMESPACE_MW


#endif


























