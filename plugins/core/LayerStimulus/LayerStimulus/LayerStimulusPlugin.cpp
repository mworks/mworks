//
//  LayerStimulusPlugin.cpp
//  LayerStimulus
//
//  Created by Christopher Stawarz on 6/19/18.
//  Copyright © 2018 The MWorks Project. All rights reserved.
//

#include "LayerStimulus.hpp"
#include "MaskStimulus.hpp"


BEGIN_NAMESPACE_MW


class LayerStimulusPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardStimulusFactory, LayerStimulus>();
        registry->registerFactory<StandardStimulusFactory, MaskStimulus>();
    }
};


extern "C" Plugin* getPlugin() {
    return new LayerStimulusPlugin();
}


END_NAMESPACE_MW
