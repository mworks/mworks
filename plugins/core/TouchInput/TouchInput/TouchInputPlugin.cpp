//
//  TouchInputPlugin.cpp
//  TouchInput
//
//  Created by Christopher Stawarz on 3/20/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "TouchInputDevice.hpp"


BEGIN_NAMESPACE_MW


class TouchInputPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, TouchInputDevice>();
    }
};


extern "C" Plugin* getPlugin() {
    return new TouchInputPlugin();
}


END_NAMESPACE_MW
