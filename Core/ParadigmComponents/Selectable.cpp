/*
 *  Selectable.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 5/1/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */

#include "Selectable.h"
#include "SequentialSelection.h"
#include "RandomWORSelection.h"
#include "RandomWithReplacementSelection.h"
using namespace mw;


Selectable::Selectable(){
	selection = shared_ptr<Selection>();
}

//DAVE TODO DEFINE
Selectable::~Selectable() { }


void Selectable::attachSelection(shared_ptr<Selection> newselect) { 
	selection = newselect; 
	selection->setSelectable(this);
	selection->reset();
}

shared_ptr<Selection> Selectable::getSelectionClone() {
	if (selection != 0) {
		return selection;
//		return selection->clone();
	} else {
		return shared_ptr<Selection>();
	}
}


void Selectable::acceptSelections() {
	if(selection == NULL){
		throw SimpleException("Attempt to \"accept\" on an empty selection object");
	}
	selection->acceptSelections();
}

void Selectable::rejectSelections() {
	if(selection == NULL){
		throw SimpleException("Attempt to \"reject\" on an empty selection object");
	}
	selection->rejectSelections();
}

void Selectable::resetSelections() {
	if(selection == NULL){
		throw SimpleException("Attempt to \"reset\" on an empty selection object");
	}
	selection->reset();
}

int Selectable::getNLeft() {
	return selection->getNLeft();	
}

int Selectable::getNDone() {
	return selection->getNDone();
}
