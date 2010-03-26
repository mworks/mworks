/*
 *  DynamicStimulusDriver.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "DynamicStimulusDriver.h"
#include "boost/bind.hpp"
#include "MWorksCore/StandardVariables.h"

DynamicStimulusDriver::DynamicStimulusDriver(
								   const boost::shared_ptr<Scheduler> &a_scheduler,
								   const boost::shared_ptr<StimulusDisplay> &a_display,
								   const boost::shared_ptr<Variable> &_frames_per_second,
								   const boost::shared_ptr<Variable> &_statistics_reporting,
								   const boost::shared_ptr<Variable> &_error_reporting){
    start_time = -1;
    scheduler = a_scheduler;
	display = a_display;
	frames_per_second = _frames_per_second;
	statistics_reporting = _statistics_reporting;
	error_reporting = _error_reporting;
	
    schedule_node == shared_ptr<ScheduleTask>();
	started = false;
    
                                                                                              
    state_system_callback = shared_ptr<VariableCallbackNotification>(
                                new VariableCallbackNotification(boost::bind(&DynamicStimulusDriver::stateSystemCallback, this, _1,_2))
                            );
    state_system_mode->addNotification(state_system_callback);
}

DynamicStimulusDriver::DynamicStimulusDriver(const DynamicStimulusDriver &tocopy){}

void DynamicStimulusDriver::stateSystemCallback(const Datum& data, MWorksTime time){
    if(data.getInteger() == IDLE){
        stop();
    }
}

DynamicStimulusDriver::~DynamicStimulusDriver(){
 
    state_system_callback->remove();
}

void DynamicStimulusDriver::play() {
	boost::mutex::scoped_lock locker(stim_lock);
	
    mprintf("CALLED PLAY!");
    shared_ptr<Clock> clock = Clock::instance(false);
    start_time = clock->getCurrentTimeUS();
    
	if (!started) {
		//const float frames_per_us = frames_per_second->getValue().getFloat()/1000000;
		MWorksTime interval = (MWorksTime)((double)1000000 / frames_per_second->getValue().getFloat());
        
		started = true;
        
		
		shared_ptr<DynamicStimulusDriver> this_one = shared_from_this();
		
		if(schedule_node != 0) {
			schedule_node->cancel();	
		}
		schedule_node = scheduler->scheduleUS(FILELINE,
											  0,
											  interval, 
											  M_REPEAT_INDEFINITELY, 
											  boost::bind(nextUpdate, this_one),
											  M_DEFAULT_PRIORITY,
											  10000000*M_DEFAULT_WARN_SLOP_US,
											  10000000*M_DEFAULT_FAIL_SLOP_US,
											  M_MISSED_EXECUTION_DROP);	
	}	
}

void DynamicStimulusDriver::stop() {
	boost::mutex::scoped_lock locker(stim_lock);
	
    mprintf("CALLED STOP!");
	// just drew the final frame
	started = false;
	
	// cancel any existing updates
	if(schedule_node != NULL){
        schedule_node->cancel();	
    }
}

void DynamicStimulusDriver::callUpdateDisplay() {
	// DDC: this is crazy bad
    //display->asynchronousUpdateDisplay();
    
    bool still_going = false;
    
    {
        boost::mutex::scoped_lock locker(stim_lock);
        still_going = started;
    }
    
    if(still_going){
        // this is thread-safe already (and doesn't spawn a new
        // thread every time!)
        display->updateDisplay(false);
    }
}

// TODO: must work out what to do here, because this is ugly
Datum DynamicStimulusDriver::getCurrentAnnounceDrawData() {
	Datum announce_data(M_DICTIONARY, 4);
	announce_data.addElement(STIM_NAME,"");        // char
	announce_data.addElement(STIM_ACTION,STIM_ACTION_DRAW);
	announce_data.addElement(STIM_TYPE,"dynamic_stimulus");  
	announce_data.addElement("start_time", start_time);  
	return announce_data;
}

void *nextUpdate(const shared_ptr<DynamicStimulusDriver> &ds){
	ds->callUpdateDisplay();	
    return NULL;
}










