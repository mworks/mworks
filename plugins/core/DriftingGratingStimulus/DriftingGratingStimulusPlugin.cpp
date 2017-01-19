/*
 *  DriftingGratingStimulusPlugins.cpp
 *  DriftingGratingStimulusPlugins
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DriftingGratingStimulus.h"


BEGIN_NAMESPACE_MW


class DriftingGratingStimulusPlugin : public Plugin {
    void registerComponents(shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardStimulusFactory, DriftingGratingStimulus>();
    }
};


extern "C" Plugin* getPlugin() {
    return new DriftingGratingStimulusPlugin();
}


END_NAMESPACE_MW
