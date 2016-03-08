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


class SelectionVariable : public Selectable, public ReadOnlyVariable {
    
protected:
    static const int NO_SELECTION = -1;
    
    std::vector<Datum> values;
    int selected_index;
    bool advanceOnAccept;
    
public:
	SelectionVariable(VariableProperties *props, shared_ptr<Selection> _sel = shared_ptr<Selection>());
    
    void setAdvanceOnAccept(bool val) {
        advanceOnAccept = val;
    }
    
    void addValue(const Datum &val) {
        values.push_back(val);
		if (selection != NULL) {
			resetSelections();
		}
    }
    
    void addValue(const shared_ptr<Variable> &var) {
        if (var) {
            addValue(var->getValue());
        }
	}
	
    Datum getTentativeSelection(int index);
    void nextValue();
	
    // Variable overrides
    Datum getValue() override;

    //
    // Selectable overrides
    //
    
    int getNItems() override { return values.size(); }
    
    void resetSelections() override {
        Selectable::resetSelections();
		selected_index = NO_SELECTION;
	}
	
	void rejectSelections() override {
        Selectable::rejectSelections();
		nextValue();
	}

    void acceptSelections() override {
        Selectable::acceptSelections();
        if (advanceOnAccept && (getNLeft() > 0)) {
            nextValue();
        }
    }

};


class SelectionVariableFactory : public ComponentFactory {
    
    shared_ptr<mw::Component> createObject(std::map<std::string, std::string> parameters,
                                           ComponentRegistry *reg) override;
    
};


END_NAMESPACE_MW


#endif


























