/*
 *  DriftingGratingStimulusPlugins.cpp
 *  DriftingGratingStimulusPlugins
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include <MWorksCore/Plugin.h>
#include <MWorksCore/StandardStimulusFactory.h>

#include "DriftingGratingStimulus.h"

using namespace mw;


class DriftingGratingStimulusPlugin : public Plugin {
    void registerComponents(shared_ptr<ComponentRegistry> registry) {
        registry->registerFactory<StandardStimulusFactory, DriftingGratingStimulus>();
    }
};


MW_SYMBOL_PUBLIC
extern "C" Plugin* getPlugin(){
    return new DriftingGratingStimulusPlugin();
}
