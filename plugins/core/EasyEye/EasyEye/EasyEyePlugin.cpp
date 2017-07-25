//
//  EasyEyePlugin.cpp
//  EasyEye
//
//  Created by Christopher Stawarz on 7/25/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "EasyEyeDevice.hpp"


BEGIN_NAMESPACE_MW


class EasyEyePlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, EasyEyeDevice>();
    }
};


extern "C" Plugin* getPlugin() {
    return new EasyEyePlugin();
}


END_NAMESPACE_MW
