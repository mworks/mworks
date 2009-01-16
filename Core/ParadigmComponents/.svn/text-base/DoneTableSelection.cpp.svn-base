/*
 *  DoneTableSelection.cpp
 *  MonkeyWorksCore
 *
 *  Created by labuser on 9/25/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "DoneTableSelection.h"
using namespace mw;

DoneTableSelection::DoneTableSelection(Selectable *_selectable) : Selection(_selectable){
	done_table_samples = 0;
	done_table = new ExpandableList<int>(getNItems());
	nelements = getNItems();
	for(int i = 0; i < nelements; i++) {
		done_table->addElement(i, 0);
	}
}

DoneTableSelection::DoneTableSelection() : Selection() {
	done_table_samples = 0;
	done_table = new ExpandableList<int>();
	nelements = getNItems();
	for(int i = 0; i < nelements; i++) {
		done_table->addElement(i, 0);
	}
}

DoneTableSelection::DoneTableSelection(const DoneTableSelection& tocopy) :
Selection(tocopy){
	
	done_table_samples = tocopy.done_table_samples;
	done_table = new ExpandableList<int>(*(tocopy.done_table));
	nelements = tocopy.nelements;
	nper = tocopy.nper;
}

DoneTableSelection::~DoneTableSelection() {
	// 1) is it safe to delte this
	// 2) should change to using new/delete operators
	// TODO
	delete done_table;
}

// default increase in size routine. Just makes sure no memory problems
void DoneTableSelection::incrementNItems() {
	//nitems++;
	done_table->addElement(0);
}

// probably needs overloading...
void DoneTableSelection::resetDoneTable() {
	for(int i = 0; i < nelements; i++){
		done_table->addElement(i,0);
	}
};

void DoneTableSelection::reset() {
	done_so_far = 0;
	resetDoneTable();
	if(tentative_selections != NULL){
		delete tentative_selections;
	}
	tentative_selections = new ExpandableList<int>();
	selectable->resetParams();
}



bool  DoneTableSelection::advance() {
	
	done_so_far++;
	updateDoneTable();
	
	if(done_so_far >= getNSamples()) {
		return false;
	}
	
	return true;
}

void DoneTableSelection::updateDoneTable() { };



// TODO: funniness when reset the donetable?
// Reject all of the tentative selection
// (e.g. rewind those selections, they will be made
// available again for selection)
void DoneTableSelection::rejectSelections(){
	// subclasses will need to override this
	for(int i = 0; i < tentative_selections->getNElements(); i++){
		done_so_far--;
		int index = (*(tentative_selections->getElement(i)));
		int how_many = (*(done_table->getElement(index))) + 1;
		done_table->addElement(index, how_many);
	}
	
	if(tentative_selections != NULL){
		delete tentative_selections;
	}
	tentative_selections = new ExpandableList<int>();
	return;
}



