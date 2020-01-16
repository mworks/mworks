//
//  RenderActionsPlugin.cpp
//  RenderActions
//
//  Created by Christopher Stawarz on 3/19/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "RenderActions.hpp"


BEGIN_NAMESPACE_MW


class RenderActionsPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardStimulusFactory, RenderActions>();
    }
};


extern "C" Plugin* getPlugin() {
    return new RenderActionsPlugin();
}


END_NAMESPACE_MW
