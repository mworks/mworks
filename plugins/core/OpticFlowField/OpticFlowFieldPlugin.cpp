//
//  OpticFlowFieldPlugin.cpp
//  OpticFlowField
//
//  Created by Christopher Stawarz on 5/31/23.
//

#include "OpticFlowField.hpp"


BEGIN_NAMESPACE_MW


class OpticFlowFieldPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardStimulusFactory, OpticFlowField>();
    }
};


extern "C" Plugin* getPlugin() {
    return new OpticFlowFieldPlugin();
}


END_NAMESPACE_MW
