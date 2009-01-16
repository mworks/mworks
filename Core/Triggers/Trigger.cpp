/*
 *  Trigger.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 6/24/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "Trigger.h"
#include "Utilities.h"
using namespace mw;

#define VERBOSE_TRIGGERS 0

/**
*    Trigger functions
*/
Trigger::Trigger(){
	active = true;
	tripped = false;
}
Trigger::~Trigger(){ }

bool Trigger::evaluate(){
	return false;
}

void Trigger::setActive(bool _active) {
    active = _active; 
    // JJD added this -- a non-active trigger cannot be true.
    //  Some triggers come and go (e.g. fixation points), so they may need
    //  to be activated and deactivated potentially rapidly.
    tripped = false;
}


AsynchronousTriggerNotification::AsynchronousTriggerNotification(
												AsynchronousTrigger *_target){
								
	target = _target;
}


/**
*    AsynchronousTrigger functions
*/
AsynchronousTrigger::AsynchronousTrigger(shared_ptr<Variable> _trigger){
	trigger_variable = _trigger;
	notifications = new ExpandableList<AsynchronousTriggerNotification>();
}


AsynchronousTrigger::~AsynchronousTrigger(){
	for (int i=0; i<notifications->getNElements(); ++i) {
		shared_ptr <AsynchronousTriggerNotification> notif = notifications->getElement(i);
		notif->remove();
	}
	
	delete notifications; // this will delete all of the notifications and,
						  // better yet, remove them from their corresponding
						  // variables' notification chains
}

// JJD added this -- a non-active trigger cannot be true.
//  Some triggers come and go (e.g. fixation points), so they may need
//  to be activated and deactivated potentially rapidly.
// TODO -- Dave, how do we make the stimulus draw routines deactivate the trigger?
void AsynchronousTrigger::setActive(bool _active) {
    active = _active; 
    tripped = false;
    // only cause variable change if it actually changed
    if ((!active) && (bool)(*trigger_variable)) {
        (*trigger_variable) = false;
    }
}


bool AsynchronousTrigger::evaluate(){ return false;}

void AsynchronousTrigger::addVariable(shared_ptr<Variable> var){
	if(var == NULL){
		mwarning(M_PARADIGM_MESSAGE_DOMAIN,
				"Attempt to add a notification to a null variable");
		return;
	}
	
	shared_ptr<AsynchronousTriggerNotification> notif(
									new AsynchronousTriggerNotification(this));
	var->addNotification(notif);
	
	notifications->addReference(notif);
}

GreaterThanThresholdTrigger::GreaterThanThresholdTrigger(
								   shared_ptr<Variable> _threshold, 
								   shared_ptr<Variable> _watchvar, 
								   shared_ptr<Variable> _trigger):
								   AsynchronousTrigger(_trigger){

	threshold = _threshold;
	watchvar = _watchvar;
}

LessThanThresholdTrigger::LessThanThresholdTrigger(shared_ptr<Variable> _threshold, 
												 shared_ptr<Variable> _watchvar, 
												 shared_ptr<Variable> _trigger):
												 AsynchronousTrigger(_trigger){
														  
	threshold = _threshold;
	watchvar = _watchvar;
}

SquareRegionTrigger::SquareRegionTrigger(shared_ptr<Variable> _centerx, 
									 shared_ptr<Variable> _centery, shared_ptr<Variable> _width,
									 shared_ptr<Variable> _watchx, shared_ptr<Variable> _watchy, 
									 shared_ptr<Variable> _trigger):
									 AsynchronousTrigger(_trigger){
									 
	centerx = _centerx;
	centery = _centery;
	width = _width;
	watchx = _watchx;
	watchy = _watchy;
	addVariable(watchx);
	addVariable(watchy);
}

bool SquareRegionTrigger::evaluate(){

	bool result;
    
	if(!active) return false;

	double watchx_val = (double)(watchx->getValue());
	double watchy_val = (double)(watchy->getValue());
	
	double centerx_val = (double)(centerx->getValue());
	double centery_val = (double)(centery->getValue());

	double width_val = (double)(width->getValue());

	if(	watchx_val < (centerx_val + (width_val / 2)) &&
		
		watchx_val > (centerx_val - (width_val / 2)) &&
		
		watchy_val < (centery_val + (width_val / 2)) &&
			
		watchy_val > (centery_val - (width_val / 2))){
		
		// DDC: what was this doing here?
		/*if(trigger_variable){ 
			(*trigger_variable) = true;
		}*/
		result = true;
	} else {
		result = false;
	}
    //mprintf("x: %f %f  y: %f %f Trigger result = %d",(double)(*watchx), (double)(*centerx), (double)(*watchy), (double)(*centery), result);
	
	if(!tripped && result){
		(*trigger_variable) = true;
		tripped = true;
	}

	if(tripped && !result){
		(*trigger_variable) = false;
		tripped = false;

	}
	
	
	return result;
}



RectangularRegionTrigger::RectangularRegionTrigger(shared_ptr<Variable> _lowerx, shared_ptr<Variable> _lowery, 
				shared_ptr<Variable> _upperx, shared_ptr<Variable> _uppery, 
				shared_ptr<Variable> _watchx, shared_ptr<Variable> _watchy, 
				shared_ptr<Variable> _trigger) : AsynchronousTrigger(_trigger){
				
				
	lowerx = _lowerx;
	lowery = _lowery;
	upperx = _upperx;
	uppery = _uppery;
	watchx = _watchx;
	watchy = _watchy;
	//trigger_variable = _trigger;
}
	
bool RectangularRegionTrigger::evaluate(){

	bool result;
	
	if(!active) return false;

	
	if((double)(*watchx) < (double)(*upperx) && 
	   (double)(*watchx) > (double)(*lowerx) &&
	   (double)(*watchy) < (double)(*uppery) && 
	   (double)(*watchy) > (double)(*uppery)){
		
		result = true;
	} else {
		result = false;
	}
	
	if(!tripped && result){
		(*trigger_variable) = true;
		tripped = true;
	}

	if(tripped && !result){
		(*trigger_variable) = false;
		tripped = false;
	}
	
	return result;
	
}

CircularRegionTrigger::CircularRegionTrigger(shared_ptr<Variable> _centerx, 
			shared_ptr<Variable> _centery, shared_ptr<Variable> _width, shared_ptr<Variable> _watchx, 
			shared_ptr<Variable> _watchy, shared_ptr<Variable> _trigger):
			AsynchronousTrigger(_trigger){

	centerx = _centerx;
	centery = _centery;
	watchx = _watchx;
	watchy = _watchy;
	width = _width;
}

bool CircularRegionTrigger::evaluate(){
	
	bool result;
	
	if(!active) return false;
	
	if( (((double)*watchx - (double)*centerx) * 
		((double)*watchx - (double)*centerx) + 
		
		((double)*watchy - (double)*centery) * 
		((double)*watchy - (double)*watchy)) 
		
		< ((double)*width * (double)*width / 4)){
		
		if(trigger_variable) (*trigger_variable) = true;
		result = true;
	} else {
		result = false;
	}
	
	if(!tripped && result){
		(*trigger_variable) = true;
		tripped = true;
	}

	if(tripped && !result){
		(*trigger_variable) = false;
		tripped = false;
	}
	
	return result;

		
}




