//
//  ParametricShapePlugin.cpp
//  ParametricShape
//
//  Created by Christopher Stawarz on 10/6/21.
//

#include "ParametricShape.hpp"


BEGIN_NAMESPACE_MW


class ParametricShapePlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardStimulusFactory, ParametricShape>();
    }
};


extern "C" Plugin* getPlugin() {
    return new ParametricShapePlugin();
}


END_NAMESPACE_MW
