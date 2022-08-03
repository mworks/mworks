/*
 *  StandardSounds.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 10/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#define MWORKS_EXCLUDE_LEGACY_SOUNDS
#include "StandardSounds.h"

#include "AudioFileSound.hpp"
#include "StandardSoundFactory.hpp"


BEGIN_NAMESPACE_MW


void registerStandardSounds(ComponentRegistry &registry) {
    registry.registerFactory<StandardSoundFactory, AudioFileSound>();
    registry.registerFactoryAlias<AudioFileSound>("sound/wav_file");
}


END_NAMESPACE_MW
