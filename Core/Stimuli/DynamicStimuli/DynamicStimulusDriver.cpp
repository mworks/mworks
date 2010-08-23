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

namespace mw{

DynamicStimulusDriver::DynamicStimulusDriver(shared_ptr<Scheduler> scheduler,
                                             shared_ptr<Variable> frames_per_second) :
    frames_per_second(frames_per_second),
    started(false),
    start_time(-1)
{
    if (!scheduler) {
        scheduler = Scheduler::instance();
        if (!scheduler) {
            throw SimpleException("No scheduler registered");
        }
    }
	
    clock = scheduler->getClock();
                                                                                              
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
	
    //mprintf("CALLED PLAY!");
    
    if (started) {
        return;
    }
    
    const int frameRate = frames_per_second->getValue().getInteger();
    const int refreshRate = StimulusDisplay::getCurrentStimulusDisplay()->getMainDisplayRefreshRate();

    if ((frameRate > refreshRate) || ((refreshRate % frameRate) != 0)) {
        merror(M_DISPLAY_MESSAGE_DOMAIN,
               "Requested frame rate (%d) is incompatible with display refresh rate (%d)",
               frameRate,
               refreshRate);
    }
    
    willPlay();
    
    start_time = clock->getCurrentTimeUS();
    interval_us = (MWorksTime)((double)1000000 / (double)frameRate);
    started = true;
}

void DynamicStimulusDriver::stop() {
	boost::mutex::scoped_lock locker(stim_lock);
	
    //mprintf("CALLED STOP!");
    
    if (!started) {
        return;
    }

	started = false;
    
    didStop();
}

MWTime DynamicStimulusDriver::getElapsedTime(){
    
    if(!started){
        return -1;
    }
    
    MWTime now = clock->getCurrentTimeUS();
    return now - start_time;
}

int DynamicStimulusDriver::getFrameNumber(){
    
    if (!started) {
        return -1;
    }

    MWTime elapsed = getElapsedTime();
    return elapsed / interval_us;
}

} // end namespace








