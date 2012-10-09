/*
 *  DynamicStimulusDriver.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/14/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "DynamicStimulusDriver.h"

#include "StandardVariables.h"

#include <boost/bind.hpp>


BEGIN_NAMESPACE_MW


DynamicStimulusDriver::DynamicStimulusDriver() :
    startTime(NOT_STARTED),
    pauseTime(NOT_PAUSED),
    elapsedTimeWhilePaused(0)
{
    stateSystemCallbackNotification =
        shared_ptr<VariableCallbackNotification>(new VariableCallbackNotification(boost::bind(&DynamicStimulusDriver::stateSystemCallback, this, _1,_2)));
    state_system_mode->addNotification(stateSystemCallbackNotification);
}


DynamicStimulusDriver::~DynamicStimulusDriver() {
    stateSystemCallbackNotification->remove();
}


void DynamicStimulusDriver::stateSystemCallback(const Datum &data, MWorksTime time) {
    switch (data.getInteger()) {
        case IDLE:
            stop();
            break;
            
        case RUNNING:
            unpause();
            break;
            
        case PAUSED:
            pause();
            break;
    }
}


void DynamicStimulusDriver::play() {
	boost::mutex::scoped_lock locker(stim_lock);
    
    if (!isPlaying()) {
        startPlaying();
    }
}


void DynamicStimulusDriver::stop() {
	boost::mutex::scoped_lock locker(stim_lock);
    
    if (isPlaying()) {
        stopPlaying();
    }
}


void DynamicStimulusDriver::pause() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    if (isPlaying() && !isPaused()) {
        beginPause();
    }
}


void DynamicStimulusDriver::unpause() {
    boost::mutex::scoped_lock locker(stim_lock);
    
    if (isPlaying() && isPaused()) {
        endPause();
    }
}


MWTime DynamicStimulusDriver::getElapsedTime() const {
    if (!isPlaying()) {
        return NOT_STARTED;
    }
    
    MWTime currentTime;
    if (isPaused()) {
        currentTime = pauseTime;
    } else {
        currentTime = getCurrentTime();
    }
    
    return (currentTime - startTime) - elapsedTimeWhilePaused;
}


void DynamicStimulusDriver::startPlaying() {
    startTime = getCurrentTime();
}


void DynamicStimulusDriver::stopPlaying() {
    startTime = NOT_STARTED;
    pauseTime = NOT_PAUSED;
    elapsedTimeWhilePaused = 0;
}


void DynamicStimulusDriver::beginPause() {
    pauseTime = getCurrentTime();
}


void DynamicStimulusDriver::endPause() {
    elapsedTimeWhilePaused += getCurrentTime() - pauseTime;
    pauseTime = NOT_PAUSED;
}


END_NAMESPACE_MW


























