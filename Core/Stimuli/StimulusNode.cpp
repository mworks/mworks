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



StimulusNodeGroup::StimulusNodeGroup(shared_ptr<StimulusGroup> group){
	for(int i = 0; i < group->getNElements(); i++){
		shared_ptr<StimulusNode> p(new StimulusNode(group->getElement(i)));
		addReference(p);
	} // TODO leaks
}

StimulusGroupReferenceNode::StimulusGroupReferenceNode(
                                                       shared_ptr<StimulusNodeGroup> _group,
                                                       shared_ptr<Variable> _index) : StimulusNode(){
    
	stimulus_nodes = _group;
	index = _index;
}


int StimulusGroupReferenceNode::getIndexValue(){
	int the_index = (*index);
	int nelements = stimulus_nodes->getNElements();
	if(the_index >= nelements ||
	   the_index < 0){
		merror(M_DISPLAY_MESSAGE_DOMAIN, 
			   "Attempt to access illegal index (%d) in stimulus group (nelements = %d)", 
			   the_index, 
			   nelements);
		return -1;
	}
	
	return the_index;
	
}

void StimulusGroupReferenceNode::addToDisplay(shared_ptr<StimulusNode> stimnode, shared_ptr<StimulusDisplay> display){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		display->addStimulusNode(stimulus_nodes->getElement(index_value));
	}
}

void StimulusGroupReferenceNode::freeze(){
    setFrozen(true);
}

void StimulusGroupReferenceNode::thaw(){
    setFrozen(false);
}

// set the "frozen" state of the node
void StimulusGroupReferenceNode::setFrozen(bool _frozen){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
        shared_ptr<StimulusNode> node = stimulus_nodes->getElement(index_value);
        if(_frozen){
            node->freeze();
        } else {
            node->thaw();
        }
	}
}

bool StimulusGroupReferenceNode::isFrozen(){
    
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		return (stimulus_nodes->getElement(index_value))->isFrozen();
	} else {
		return false;
	}
}


void StimulusGroupReferenceNode::load(shared_ptr<StimulusDisplay> display){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->load(display);
	}  
}

bool StimulusGroupReferenceNode::isLoaded(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		return (stimulus_nodes->getElement(index_value))->isLoaded();
	}
    
    return false;
}

int StimulusGroupReferenceNode::getDeferred(){
    int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		return (stimulus_nodes->getElement(index_value))->getDeferred();
	}
    return Stimulus::nondeferred_load;
}

// Passthrough to the referenced node
void StimulusGroupReferenceNode::draw(StimulusDisplay * display){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->draw(display);
	}
}


void StimulusGroupReferenceNode::setVisible(bool _vis){ 
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->setVisible(_vis);
	}
}

bool StimulusGroupReferenceNode::isVisible(){          
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		return (stimulus_nodes->getElement(index_value))->isVisible();
	} else {
		return false;
	}
}	



//void StimulusGroupReferenceNode::clearPending(){
//    int index_value = getIndexValue();
//	int nelements = stimulus_nodes->getNElements();
//	if(index_value >=0 && index_value < nelements ){
//		(stimulus_nodes->getElement(index_value))->clearPending();
//	}
//}
//
//bool StimulusGroupReferenceNode::isPending(){
//    int index_value = getIndexValue();
//	int nelements = stimulus_nodes->getNElements();
//	if(index_value >=0 && index_value < nelements ){
//		return (stimulus_nodes->getElement(index_value))->isPending();
//	} else {
//		return false;
//	}
//}

void StimulusGroupReferenceNode::setPendingVisible(bool _vis){
    int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
        (stimulus_nodes->getElement(index_value))->setPending();
		(stimulus_nodes->getElement(index_value))->setPendingVisible(_vis);
	}
}

bool StimulusGroupReferenceNode::isPendingVisible(){
    int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		return (stimulus_nodes->getElement(index_value))->isPendingVisible();
	} else {
		return false;
	}
}

/*void StimulusGroupReferenceNode::setPendingRemoval(){
 int index_value = getIndexValue();
 int nelements = stimulus_nodes->getNElements();
 if(index_value >=0 && index_value < nelements ){
 (stimulus_nodes->getElement(index_value))->setPending();
 (stimulus_nodes->getElement(index_value))->setPendingRemoval();
 }
 }
 
 void StimulusGroupReferenceNode::clearPendingRemoval(){
 int index_value = getIndexValue();
 int nelements = stimulus_nodes->getNElements();
 if(index_value >=0 && index_value < nelements ){
 (stimulus_nodes->getElement(index_value))->clearPendingRemoval();
 }
 }
 
 bool StimulusGroupReferenceNode::isPendingRemoval(){
 int index_value = getIndexValue();
 int nelements = stimulus_nodes->getNElements();
 if(index_value >=0 && index_value < nelements ){
 return (stimulus_nodes->getElement(index_value))->isPendingRemoval();
 } else {
 return false;
 }
 }*/


/*void StimulusGroupReferenceNode::setVisibleOnLastUpdate(bool _vis){ // JJD add
 (stimulus_nodes->getElement((int)(*index)))->setVisibleOnLastUpdate(_vis);
 }
 
 bool StimulusGroupReferenceNode::wasVisibleOnLastUpdate(){          // JJD add
 return (stimulus_nodes->getElement((int)(*index)))->wasVisibleOnLastUpdate();
 }*/

void StimulusGroupReferenceNode::announceStimulusDraw(MWTime time){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->announceStimulusDraw(time);
	}
}

Datum StimulusGroupReferenceNode::getCurrentAnnounceDrawData(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		return (stimulus_nodes->getElement(index_value))->getCurrentAnnounceDrawData();
	} else {
        Datum undef;
		return undef;
	}
}


void StimulusGroupReferenceNode::remove(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->remove();
	}
}

void StimulusGroupReferenceNode::bringToFront(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->bringToFront();
	}
}

void StimulusGroupReferenceNode::sendToBack(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->sendToBack();
	}
}


void StimulusGroupReferenceNode::bringForward(){
	
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->moveForward();
	}
}

void StimulusGroupReferenceNode::sendBackward(){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->moveBackward();
	}
}

