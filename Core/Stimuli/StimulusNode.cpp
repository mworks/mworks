/*
 *  StimulusNode.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 6/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "StimulusNode.h"
using namespace mw;

/**********************************************************************
 *                  StimulusNode Methods
 **********************************************************************/
StimulusNode::StimulusNode() : Lockable() {
    //stim = NULL;
	//frozen_stim = NULL;
//    previous = NULL;
 //   next = NULL;
    visible = false;
    visible_on_last_update = false;
    
    pending = false;
    pending_visible = false;
    pending_removal = false;
    
	frozen = false;
}

StimulusNode::StimulusNode(shared_ptr<Stimulus> _stim) : 
								Lockable(), LinkedListNode<StimulusNode>() {
    stim = _stim;

    visible = false;
    visible_on_last_update = false;
	
    pending = false;
    pending_visible = false;
    pending_removal = false;
    
    frozen = false;
}
    
/*void StimulusNode::setStimulus(Stimulus *_stim) {
    stim = _stim;
}*/
        
shared_ptr<Stimulus> StimulusNode::getStimulus() {
    return stim;
}
 
void StimulusNode::setPendingVisible(bool _vis){
    lock();
    pending = true;
    pending_visible = _vis;

    unlock();
}

bool StimulusNode::isPendingVisible(){
    return pending_visible;
}



void StimulusNode::setVisible(bool _vis) {
    lock();
            
    visible = _vis;
    if(stim) {
        // in case the stimulus object needs to know...
        stim->setVisible(visible);  
    }
    unlock();
}
        
bool StimulusNode::isVisible() {
    return visible;
}

void StimulusNode::load(shared_ptr<StimulusDisplay> display){
  stim->load(display.get());
}

bool StimulusNode::isLoaded(){
  return stim->isLoaded();
}

int StimulusNode::getDeferred(){  
  return stim->getDeferred(); 
}

void StimulusNode::draw(StimulusDisplay *stimulus_display) {
	// TODO: error check
	live_stim->draw(stimulus_display);
}

// Freeze the stimulus as needed, and set the frozen flag
bool StimulusNode::isFrozen(){
	return frozen;
}


void StimulusNode::freeze(){
    lock();
	
    // set the "live" version of the stimulus to the
    // a frozen (variable values fixed) version of stim
    live_stim = shared_ptr<Stimulus>(stim->frozenClone());
    frozen = true;

	unlock();
}

void StimulusNode::thaw(){
    lock();
    live_stim = stim;  // replace the "live" version of the
                       // stimulus with the raw stim
    frozen = false;
	unlock();
}

void StimulusNode::clearPending(){
    lock();
    pending = false;
    unlock();
}

bool StimulusNode::isPending(){
    return pending;
}

void StimulusNode::setPendingRemoval(){
    lock();
    pending = true;
    pending_removal = true;
    unlock();
}

void StimulusNode::clearPendingRemoval(){
    lock();
    pending_removal = false;
    unlock();
}

bool StimulusNode::isPendingRemoval(){
    return pending_removal;
}

void StimulusNode::announceStimulusDraw(MWTime now) {
	if(live_stim == NULL) {
		// TODO: warn
		return;
	}
	
	live_stim->announceStimulusDraw(now);
	
}

Datum StimulusNode::getCurrentAnnounceDrawData() {
	if(live_stim == NULL) {
		// TODO: warn
        Datum undef;
		return undef;
	}

    return live_stim->getCurrentAnnounceDrawData();
	
}

void StimulusNode::addToDisplay(shared_ptr<StimulusNode> stimnode, 
								 shared_ptr<StimulusDisplay> display){
	display->addStimulusNode(stimnode);
}

