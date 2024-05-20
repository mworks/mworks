/**
 * StandardStimuli.cpp
 *
 * Copyright 2005 MIT.  All rights reserved.
 */

#include "StandardStimuli.h"

#include "BlankScreenStimulus.hpp"
#include "RectangleStimulus.hpp"
#include "EllipseStimulus.hpp"
#include "ImageFileStimulus.hpp"
#include "FixationPoint.hpp"
#include "StandardStimulusFactory.h"


BEGIN_NAMESPACE_MW


void registerStandardStimuli(ComponentRegistry &registry) {
    registry.registerFactory<StandardStimulusFactory, BlankScreenStimulus>();
    registry.registerFactory<StandardStimulusFactory, RectangleStimulus>();
    registry.registerFactory<StandardStimulusFactory, EllipseStimulus>();
    registry.registerFactoryAlias<EllipseStimulus>("stimulus/circle");
    registry.registerFactory<StandardStimulusFactory, ImageFileStimulus>();
    registry.registerFactory<StandardStimulusFactory, FixationPoint>();
    registry.registerFactory<StandardStimulusFactory, CircularFixationPoint>();
}


END_NAMESPACE_MW
