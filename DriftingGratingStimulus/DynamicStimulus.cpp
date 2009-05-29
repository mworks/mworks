/*
 *  Dynamicstimulus.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "DynamicStimulus.h"
#include "boost/bind.hpp"

mDynamicStimulus::mDynamicStimulus(const std::string &new_tag,
								   const boost::shared_ptr<Scheduler> &a_scheduler,
								   const boost::shared_ptr<StimulusDisplay> &a_display,
								   const boost::shared_ptr<Variable> &_frames_per_second,
								   const boost::shared_ptr<Variable> &_statistics_reporting,
								   const boost::shared_ptr<Variable> &_error_reporting) : Stimulus(new_tag) {
	scheduler = a_scheduler;
	display = a_display;
	frames_per_second = _frames_per_second;
	statistics_reporting = _statistics_reporting;
	error_reporting = _error_reporting;
	
	started = FALSE;
}

mDynamicStimulus::mDynamicStimulus(const mDynamicStimulus &tocopy) : Stimulus((const Stimulus&)tocopy){}

void mDynamicStimulus::play() {
	boost::mutex::scoped_lock locker(stim_lock);
	
	if (!started) {
		const float frames_per_us = frames_per_second->getValue().getFloat()/1000000;
		
		started = true;
		start_time = scheduler->getClock()->getCurrentTimeUS();
		
		shared_ptr<mDynamicStimulus> this_one = shared_from_this();
		
		if(schedule_node != 0) {
			schedule_node->cancel();	
		}
		schedule_node = scheduler->scheduleUS(FILELINE,
											  0,
											  (MonkeyWorksTime)(1/frames_per_us), 
											  M_REPEAT_INDEFINITELY, 
											  boost::bind(nextUpdate, this_one),
											  M_DEFAULT_PRIORITY,
											  M_DEFAULT_WARN_SLOP_US,
											  M_DEFAULT_FAIL_SLOP_US,
											  M_MISSED_EXECUTION_CATCH_UP);	
	}	
}

void mDynamicStimulus::stop() {
	boost::mutex::scoped_lock locker(stim_lock);
	
	// just drew the final frame
	started = false;
	
	// cancel any existing updates
	schedule_node->cancel();	
}

inline void mDynamicStimulus::callUpdateDisplay() {
	display->asynchronousUpdateDisplay();
}

inline Data mDynamicStimulus::getCurrentAnnounceDrawData() {
	Data announce_data(M_DICTIONARY, 4);
	announce_data.addElement(STIM_NAME,tag);        // char
	announce_data.addElement(STIM_ACTION,STIM_ACTION_DRAW);
	announce_data.addElement(STIM_TYPE,"dynamic_stimulus");  
	announce_data.addElement("start_time", start_time);  
	return announce_data;
}

void *nextUpdate(const shared_ptr<mDynamicStimulus> &ds){
	ds->callUpdateDisplay();	
    return NULL;
}










