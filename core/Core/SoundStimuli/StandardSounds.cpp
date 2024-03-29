/*
 *  StandardSounds.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "StandardSounds.h"

#include "AudioFileSound.hpp"
#include "ToneSound.hpp"
#include "StandardSoundFactory.hpp"


BEGIN_NAMESPACE_MW


void registerStandardSounds(ComponentRegistry &registry) {
    registry.registerFactory<StandardSoundFactory, AudioFileSound>();
    registry.registerFactoryAlias<AudioFileSound>("sound/wav_file");
    registry.registerFactory<StandardSoundFactory, ToneSound>();
}


END_NAMESPACE_MW
