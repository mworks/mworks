/*
 *  MovingDotsPlugin.cpp
 *  MovingDots
 *
 *  Created by Christopher Stawarz on 8/6/10.
 *  Copyright 2010 MIT. All rights reserved.
 *
 */

#include "MovingDots.h"


BEGIN_NAMESPACE_MW


class MovingDotsPlugin : public Plugin {
	void registerComponents(shared_ptr<ComponentRegistry> registry) MW_OVERRIDE {
        registry->registerFactory<StandardStimulusFactory, MovingDots>();
    }	
};


extern "C" Plugin* getPlugin() {
    return new MovingDotsPlugin();
}


END_NAMESPACE_MW
