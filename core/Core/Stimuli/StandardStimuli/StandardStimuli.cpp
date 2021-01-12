/**
 * StandardStimuli.cpp
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#define MWORKS_EXCLUDE_LEGACY_STIMULI
#include "StandardStimuli.h"

#include "BlankScreenStimulus.hpp"
#include "RectangleStimulus.hpp"
#include "StandardStimulusFactory.h"


BEGIN_NAMESPACE_MW


void registerStandardStimuli(ComponentRegistry &registry) {
    registry.registerFactory<StandardStimulusFactory, BlankScreenStimulus>();
    registry.registerFactory<StandardStimulusFactory, RectangleStimulus>();
}


END_NAMESPACE_MW
