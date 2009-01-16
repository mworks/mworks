/**
 * Selection.cpp
 *
 * History:
 * David Cox on 12/28/05 - Created.
 * Paul Jankunas on 05/13/04 - added getSelectionType to Selection type to  
 *      give information about what kind of selection an object is.
 * Paul Jankunas on 06/01/05 - Removed getSelectionType() because it is unused.
 *      fixed spacing.
 *
 * Copyright (c) 2002 MIT. All rights reserved.
 */

#include "Selection.h"
#include "Selectable.h"
#include "Experiment.h"
#include "ConstantVariable.h"
using namespace mw;

// for random seeds


Selection::Selection(int _n_draws) {
	selectable = NULL;
	n_draws = _n_draws;
}


int Selection::getNItems(){
	if(selectable != NULL){
		return selectable->getNItems();
	}
	
	return 0;
}




// Reset the object to start over
void Selection::reset() {
	done_so_far = 0;	
	tentative_selections = std::vector<int>();
}


// Accept all of the tentative selections
void Selection::acceptSelections(){
	
	tentative_selections = std::vector<int>();
}








