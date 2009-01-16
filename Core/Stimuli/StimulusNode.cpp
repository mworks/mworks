/*
 *  StimulusNode.cpp
 *  MonkeyWorksCore
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
	frozen = false;
}

StimulusNode::StimulusNode(shared_ptr<Stimulus> _stim) : 
								Lockable(), LinkedListNode<StimulusNode>() {
    stim = _stim;

    visible = false;
    visible_on_last_update = false;
	frozen = false;
}
    
/*void StimulusNode::setStimulus(Stimulus *_stim) {
    stim = _stim;
}*/
        
shared_ptr<Stimulus> StimulusNode::getStimulus() {
    return stim;
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

void StimulusNode::draw(StimulusDisplay *stimulus_display) {
	// TODO: error check
	if(frozen){
		frozen_stim->draw(stimulus_display);
	} else {
		stim->draw(stimulus_display);
	}
}

// Freeze the stimulus as needed, and set the frozen flag
void StimulusNode::setFrozen(bool _frozen){
	lock();
	frozen = _frozen;
	if(frozen){
		
		frozen_stim = shared_ptr<Stimulus>(stim->frozenClone());
	}
	unlock();
}

bool StimulusNode::isFrozen(){
	return frozen;
}


void StimulusNode::announceStimulusDraw(MonkeyWorksTime now) {
	if((!frozen && stim == NULL) || (frozen && frozen_stim == NULL)) {
		// TODO: warn
		return;
	}
	
	if(frozen) {
		frozen_stim->announceStimulusDraw(now);
	} else {
		stim->announceStimulusDraw(now);		
	}
}

Data StimulusNode::getCurrentAnnounceDrawData() {
	if((!frozen && stim == NULL) || (frozen && frozen_stim == NULL)) {
		// TODO: warn
		Data undef;
		return undef;
	}
	if(frozen) {
		return frozen_stim->getCurrentAnnounceDrawData();
	} else {
		return stim->getCurrentAnnounceDrawData();			
	}
}

void StimulusNode::addToDisplay(shared_ptr<StimulusNode> stimnode, 
								 shared_ptr<StimulusDisplay> display){
	display->addStimulusNode(stimnode);
}

