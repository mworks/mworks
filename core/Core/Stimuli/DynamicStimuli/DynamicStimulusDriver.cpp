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
    startTime(NOT_STARTED)
{
    stateSystemCallbackNotification =
        shared_ptr<VariableCallbackNotification>(new VariableCallbackNotification(boost::bind(&DynamicStimulusDriver::stateSystemCallback, this, _1,_2)));
    state_system_mode->addNotification(stateSystemCallbackNotification);
}


DynamicStimulusDriver::~DynamicStimulusDriver() {
    stateSystemCallbackNotification->remove();
}


void DynamicStimulusDriver::stateSystemCallback(const Datum &data, MWorksTime time) {
    if (data.getInteger() == IDLE) {
        stop();
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


MWTime DynamicStimulusDriver::getElapsedTime() const {
    if (!isPlaying()) {
        return NOT_STARTED;
    }
    
    return getCurrentTime() - startTime;
}


void DynamicStimulusDriver::startPlaying() {
    startTime = getCurrentTime();
}


void DynamicStimulusDriver::stopPlaying() {
    startTime = NOT_STARTED;
}


END_NAMESPACE_MW


























