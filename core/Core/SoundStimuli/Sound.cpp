/*
 *  Sound.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "Sound.h"

#include <boost/bind.hpp>


BEGIN_NAMESPACE_MW


Sound::Sound(const ParameterValueMap &parameters) :
    Component(parameters),
    Announcable(ANNOUNCE_SOUND_TAGNAME),
    isPlaying(false),
    isPaused(false)
{
    stateSystemCallbackNotification =
        boost::shared_ptr<VariableCallbackNotification>(new VariableCallbackNotification(boost::bind(&Sound::stateSystemCallback, this, _1,_2)));
    state_system_mode->addNotification(stateSystemCallbackNotification);
}


Sound::~Sound() {
    stateSystemCallbackNotification->remove();
}


void Sound::stateSystemCallback(const Datum &data, MWorksTime time) {
    switch (data.getInteger()) {
        case IDLE:
            if (isPlaying) {
                stop();
            }
            break;
            
        case RUNNING:
            if (isPlaying && isPaused) {
                play();
            }
            break;
            
        case PAUSED:
            if (isPlaying && !isPaused) {
                pause();
            }
            break;
    }
}


END_NAMESPACE_MW


























