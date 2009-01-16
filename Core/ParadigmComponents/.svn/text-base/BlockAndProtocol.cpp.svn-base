/*
 *  BlockAndProtocol.cpp
 *  ExperimentControlCocoa
 *
 *  Created by David Cox on Wed Mar 24 2004.
 *  Copyright (c) 2004 __MyCompanyName__. All rights reserved.
 *
 */

#include "BlockAndProtocol.h"

using namespace mw;


////////////////////////////////////////////////
// ->mBlock Functions
////////////////////////////////////////////////


Block::Block() : ListState(){
	setName("mBlock");
}


void Block::action(){
	ListState::action();
	if(!accessed) {
		blockAnnounce->setValue(blockAnnounce->getValue().getInteger() + 1);
		accessed = true;
	}
}

weak_ptr<State> Block::next(){
	if(!has_more_children_to_run) {
		blockAnnounce->setValue(blockAnnounce->getValue().getInteger() - 1);
	}
	return ListState::next();
}


shared_ptr<mw::Component> Block::createInstanceObject(){
//void *Block::scopedClone(){
	
	ListState *new_state = new Block();
	
	new_state->setExperiment(getExperiment());
	new_state->setScopedVariableEnvironment(getScopedVariableEnvironment());
	new_state->setDescription(getDescription());
	new_state->setName(getName());
	
	new_state->setList(list);
	
	// DDC TODO 9/07: Should there be a copy here?
	shared_ptr<Selection> sel = getSelectionClone();
	if(sel != NULL){
		new_state->attachSelection(sel);
	}
	
	// TODO: was this safe to remove?
	/*if(new_state->getSelection() != NULL){
		(new_state->getSelection())->setNItems(list->getNElements());
	}*/
	
	new_state->setIsAClone(true);	// in case we care for memory-freeing purposes
	
	shared_ptr<mw::Component> clone_ptr(new_state);
	return clone_ptr;
}


		
mw::Protocol::Protocol() : ListState(){
	setName("mProtocol");
}
		

shared_ptr<mw::Component> mw::Protocol::createInstanceObject(){
//void *mw::Protocol::scopedClone(){
	
	ListState *new_state = new mw::Protocol();

	new_state->setExperiment(getExperiment());
	new_state->setScopedVariableEnvironment(getScopedVariableEnvironment());
	new_state->setDescription(getDescription());
	new_state->setName(getName());
		
	new_state->setList(list);
	
	shared_ptr<Selection> sel = getSelectionClone();
	if(sel != NULL){
		new_state->attachSelection(sel);
	}
	
	
	new_state->setIsAClone(true);	// in case we care for memory-freeing purposes
	
	shared_ptr<mw::Component> clone_ptr(new_state);
	return clone_ptr;
}



GenericListState::GenericListState() : ListState(){
	setName("mList");
}


shared_ptr<mw::Component> GenericListState::createInstanceObject(){
//void *GenericListState::scopedClone(){
	
	ListState *new_state = new GenericListState();
	//new_state->setLocalVariableContext(new VariableContext());
	new_state->setExperiment(getExperiment());
	new_state->setScopedVariableEnvironment(getScopedVariableEnvironment());
	new_state->setDescription(getDescription());
	new_state->setName(getName());
	
	//new_state->setN(getN());
	//new_state->setSamplingMethod(getSamplingMethod());
	
	
	new_state->setList(list);
	
	shared_ptr<Selection> sel = getSelectionClone();
	if(sel != NULL){
		new_state->attachSelection(sel);
		//new_state->setSelection((Selection *)(sel->clone()));
	}
	
	// TODO: was this safe to remove?
	/*if(new_state->getSelection() != NULL){
		(new_state->getSelection())->setNItems(list->getNElements());
	}*/
	
	new_state->setIsAClone(true);	// in case we care for memory-freeing purposes
	
	shared_ptr<mw::Component> copy_ptr(new_state);
	return copy_ptr;
}


//////////////

Trial::Trial() :ListState(){
	setName("mTrial");
}


void Trial::action(){
	ListState::action();
	if(!accessed) {
		trialAnnounce->setValue(trialAnnounce->getValue().getInteger() + 1);
		accessed = true;
	}
}

weak_ptr<State> Trial::next(){
	if(!has_more_children_to_run) {
		trialAnnounce->setValue(trialAnnounce->getValue().getInteger() - 1);
	}
	return ListState::next();
}


shared_ptr<mw::Component> Trial::createInstanceObject(){
//void *Trial::scopedClone(){
	
	ListState *new_state = new Trial();
	//new_state->setLocalVariableContext(new VariableContext());
	new_state->setExperiment(getExperiment());
	new_state->setScopedVariableEnvironment(getScopedVariableEnvironment());
	new_state->setDescription(getDescription());
	new_state->setName(getName());
	
	//new_state->setN(getN());
	//new_state->setSamplingMethod(getSamplingMethod());
	
	
	new_state->setList(list);
	
	shared_ptr<Selection> sel = getSelectionClone();
	if(sel != NULL){
		new_state->attachSelection(sel);
		//new_state->setSelection((Selection *)(sel->clone()));
	}
	
	// TODO: was this safe to remove?
	/*if(new_state->getSelection() != NULL){
		(new_state->getSelection())->setNItems(list->getNElements());
	}*/
	
	new_state->setIsAClone(true);	// in case we care for memory-freeing purposes
	
	shared_ptr<mw::Component> clone_ptr(new_state);
	return clone_ptr;
}


