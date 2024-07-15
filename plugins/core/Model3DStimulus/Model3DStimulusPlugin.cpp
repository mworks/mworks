//
//  Model3DStimulusPlugin.cpp
//  Model3DStimulus
//
//  Created by Christopher Stawarz on 7/15/24.
//

#include "Model3DStimulus.hpp"


BEGIN_NAMESPACE_MW


class Model3DStimulusPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardStimulusFactory, Model3DStimulus>();
    }
};


extern "C" Plugin * getPlugin() {
    return new Model3DStimulusPlugin();
}


END_NAMESPACE_MW
