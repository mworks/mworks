/*
 *  Sound.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "Sound.h"


BEGIN_NAMESPACE_MW


const std::string Sound::AUTOLOAD("autoload");


void Sound::describeComponent(ComponentInfo &info) {
    Component::describeComponent(info);
    info.addParameter(AUTOLOAD, "YES");
}


Sound::Sound(const ParameterValueMap &parameters) :
    Component(parameters),
    Announcable(ANNOUNCE_SOUND_TAGNAME),
    autoload(parameters[AUTOLOAD])
{ }


void Sound::play(MWTime startTime) {
    merror(M_SYSTEM_MESSAGE_DOMAIN, "Sound does not support scheduled playback");
}


END_NAMESPACE_MW
