//
//  OrientedNoisePlugin.cpp
//  OrientedNoise
//
//  Created by Christopher Stawarz on 10/30/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#include "OrientedNoise.hpp"


BEGIN_NAMESPACE_MW


class OrientedNoisePlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardStimulusFactory, OrientedNoise>();
    }
};


extern "C" Plugin* getPlugin() {
    return new OrientedNoisePlugin();
}


END_NAMESPACE_MW
