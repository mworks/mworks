/*
 *  SequentialSelection.cpp
 *  MonkeyWorksCore
 *
 *  Created by labuser on 9/25/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#include "SequentialSelection.h"
using namespace mw;

SequentialSelection::SequentialSelection(int _n_draws, bool ascendingp) : 
													Selection(_n_draws) {
	ascending = ascendingp;
	reset();
}

SequentialSelection::SequentialSelection(SequentialSelection& tocopy) : 
Selection(tocopy){
	ascending = tocopy.ascending;
	current = tocopy.current;
	tentative_selections = tocopy.tentative_selections;
}


int SequentialSelection::draw() {

	// Throw an exception if we are already done
	if(done_so_far >= n_draws){
		SelectionOffEdgeException e;
		throw e;
	}

	// Advance the counter
	if(ascending) {
		current++;
		if(current >= getNItems()) {
			current = 0;
		}
	} else {
		current--;
		if(current < 0) {
			current = getNItems()-1;
		}
	}

	int returnval = current;

	if(returnval < 0 || returnval > selectable->getNItems()){
		InvalidSelectionException e;
		throw e;
	}

	// Log the returnval
	tentative_selections.push_back(returnval);
	
	// Advance the done_so_far counter
	done_so_far++;
	
	// return the selected value
	return returnval;
}

void SequentialSelection::rejectSelections() {
	
	std::vector<int>::iterator	i;
	
	for(i = tentative_selections.begin(); i != tentative_selections.end(); i++){
		
		done_so_far--;
		if(ascending){
			current--;
			if(current < 0) {
				current += getNItems();
			}
		} else {
			current++;
			if(current >= getNItems()) {
				current -= getNItems();
			}
		}
	}
	
	tentative_selections = std::vector<int>();
	
}


// Reset the object to start over
void SequentialSelection::reset() {
		
	if(ascending) {
		current = -1;
	} else {
		current = getNItems();
	}
	
	Selection::reset();
}
