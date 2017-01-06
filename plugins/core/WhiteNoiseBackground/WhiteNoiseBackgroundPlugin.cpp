/*
 *  WhiteNoiseBackgroundPlugin.cpp
 *  WhiteNoiseBackground
 *
 *  Created by Christopher Stawarz on 12/15/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "WhiteNoiseBackground.h"
#include "RandomizeBackground.h"


BEGIN_NAMESPACE_MW


class WhiteNoiseBackgroundPlugin : public Plugin {
    void registerComponents(shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardStimulusFactory, WhiteNoiseBackground>();
        registry->registerFactory<StandardComponentFactory, RandomizeBackground>();
    }	
};


extern "C" Plugin* getPlugin() {
    return new WhiteNoiseBackgroundPlugin();
}


END_NAMESPACE_MW
