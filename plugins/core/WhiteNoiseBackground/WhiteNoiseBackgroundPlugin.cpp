/*
 *  WhiteNoiseBackgroundPlugin.cpp
 *  WhiteNoiseBackground
 *
 *  Created by Christopher Stawarz on 12/15/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include <MWorksCore/Plugin.h>
#include <MWorksCore/StandardStimulusFactory.h>

#include "WhiteNoiseBackground.h"
#include "RandomizeBackground.h"

using namespace mw;


class WhiteNoiseBackgroundPlugin : public Plugin {
	virtual void registerComponents(shared_ptr<ComponentRegistry> registry) {
        registry->registerFactory<StandardStimulusFactory, WhiteNoiseBackground>();
        registry->registerFactory<StandardComponentFactory, RandomizeBackground>();
    }	
};


MW_SYMBOL_PUBLIC
extern "C" Plugin* getPlugin() {
    return new WhiteNoiseBackgroundPlugin();
}
