/*
 *  StimulusNode.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 6/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "StimulusNode.h"


BEGIN_NAMESPACE_MW


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
    
//    thaw();
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
  stim->load(display);
}

void StimulusNode::unload(shared_ptr<StimulusDisplay> display){
  stim->unload(display);
}

bool StimulusNode::isLoaded(){
  return stim->isLoaded();
}

int StimulusNode::getDeferred(){  
  return stim->getDeferred(); 
}

bool StimulusNode::needDraw() {
    return stim && stim->needDraw();
}

void StimulusNode::draw(shared_ptr<StimulusDisplay> stimulus_display) {
	// TODO: error check
	stim->draw(stimulus_display);
    //live_stim->draw(stimulus_display);
}

// Freeze the stimulus as needed, and set the frozen flag
bool StimulusNode::isFrozen(){
	return frozen;
}


void StimulusNode::freeze(){
    lock();
	
    stim->freeze();
    frozen = true;

	unlock();
}

void StimulusNode::thaw(){
    lock();
    stim->thaw();
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

void StimulusNode::announce(Datum announceData, MWTime time) {
    if (stim != NULL) {
        stim->announce(announceData, time);
    }
}

void StimulusNode::announceStimulusDraw(MWTime now) {
	if(stim == NULL) {
    //if(live_stim == NULL) {
		// TODO: warn
		return;
	}
	
    stim->announceStimulusDraw(now);
	//live_stim->announceStimulusDraw(now);
	
}

Datum StimulusNode::getCurrentAnnounceDrawData() {
	if(stim == NULL) {
    //if(live_stim == NULL) {
		// TODO: warn
        Datum undef;
		return undef;
	}

    return stim->getCurrentAnnounceDrawData();
    //return live_stim->getCurrentAnnounceDrawData();
	
}

void StimulusNode::addToDisplay(shared_ptr<StimulusDisplay> display){
    shared_ptr<StimulusNode> this_stimnode = component_shared_from_this<StimulusNode>();
	display->addStimulusNode(this_stimnode);
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

void StimulusGroupReferenceNode::addToDisplay(shared_ptr<StimulusDisplay> display){
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

void StimulusGroupReferenceNode::unload(shared_ptr<StimulusDisplay> display){
	int index_value = getIndexValue();
	int nelements = stimulus_nodes->getNElements();
	if(index_value >=0 && index_value < nelements ){
		(stimulus_nodes->getElement(index_value))->unload(display);
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
void StimulusGroupReferenceNode::draw(shared_ptr<StimulusDisplay>  display){
    
    throw SimpleException("Illegal attempt to draw a StimulusGroupReference Stimulus Node.");
    
//	int index_value = getIndexValue();
//	int nelements = stimulus_nodes->getNElements();
//	if(index_value >=0 && index_value < nelements ){
//		(stimulus_nodes->getElement(index_value))->draw(display);
//	}
}


#define STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_BOOL_RETURN(FUNC)             \
    int index_value = getIndexValue();                                      \
    int nelements = stimulus_nodes->getNElements();                         \
    if(index_value >=0 && index_value < nelements ){                        \
        return (stimulus_nodes->getElement(index_value))->FUNC();           \
    } else {                                                                \
        return false;                                                       \
    }

#define STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_VOID_RETURN(FUNC)             \
    int index_value = getIndexValue();                                      \
    int nelements = stimulus_nodes->getNElements();                         \
    if(index_value >=0 && index_value < nelements ){                        \
        (stimulus_nodes->getElement(index_value))->FUNC();                  \
    } 

#define STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_SET_BOOL(FUNC, ARG)           \
    int index_value = getIndexValue();                                      \
    int nelements = stimulus_nodes->getNElements();                         \
    if(index_value >=0 && index_value < nelements ){                        \
        (stimulus_nodes->getElement(index_value))->FUNC(ARG);               \
    }


shared_ptr<Stimulus> StimulusGroupReferenceNode::getStimulus() {
    int index_value = getIndexValue();
    int nelements = stimulus_nodes->getNElements();
    if(index_value >=0 && index_value < nelements ){
        return (stimulus_nodes->getElement(index_value))->getStimulus();
    }
    return shared_ptr<Stimulus>();
}


void StimulusGroupReferenceNode::setVisible(bool _vis){ 
	STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_SET_BOOL(setVisible, _vis)
}

bool StimulusGroupReferenceNode::isVisible(){          
	STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_BOOL_RETURN(isVisible)
}	


// Set a flag to determine whether to draw this stimulus on the
// next update
void StimulusGroupReferenceNode::setPending(){
    STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_VOID_RETURN(setPending)
}

void StimulusGroupReferenceNode::clearPending(){
    STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_VOID_RETURN(clearPending)
}

bool StimulusGroupReferenceNode::isPending(){
    STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_BOOL_RETURN(isPending)
}

void StimulusGroupReferenceNode::setPendingRemoval(){
    STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_VOID_RETURN(setPendingRemoval)
}

void StimulusGroupReferenceNode::clearPendingRemoval(){
    STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_VOID_RETURN(clearPendingRemoval)
}

bool StimulusGroupReferenceNode::isPendingRemoval(){
    STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_BOOL_RETURN(isPendingRemoval)
}    

void StimulusGroupReferenceNode::setPendingVisible(bool _vis){
    STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_SET_BOOL(setPendingVisible, _vis)
}

bool StimulusGroupReferenceNode::isPendingVisible(){
    STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_BOOL_RETURN(isPendingVisible)
}


/*void StimulusGroupReferenceNode::setVisibleOnLastUpdate(bool _vis){ // JJD add
 (stimulus_nodes->getElement((int)(*index)))->setVisibleOnLastUpdate(_vis);
 }
 
 bool StimulusGroupReferenceNode::wasVisibleOnLastUpdate(){          // JJD add
 return (stimulus_nodes->getElement((int)(*index)))->wasVisibleOnLastUpdate();
 }*/

void StimulusGroupReferenceNode::announce(Datum announceData, MWTime time) {
    int index_value = getIndexValue();
    int nelements = stimulus_nodes->getNElements();
    if (index_value >= 0 && index_value < nelements) {
        (stimulus_nodes->getElement(index_value))->announce(announceData, time);
    }
}

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

void StimulusGroupReferenceNode::bringToFront(){
    STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_VOID_RETURN(bringToFront)
}

void StimulusGroupReferenceNode::sendToBack(){
    STIM_GROUP_REFERENCE_NODE_PASSTHROUGH_VOID_RETURN(sendToBack)
}



END_NAMESPACE_MW























