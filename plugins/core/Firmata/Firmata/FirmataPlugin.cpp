//
//  FirmataPlugin.cpp
//  Firmata
//
//  Created by Christopher Stawarz on 6/10/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "FirmataDevice.hpp"
#include "FirmataDigitalInputChannel.hpp"
#include "FirmataDigitalOutputChannel.hpp"


BEGIN_NAMESPACE_MW


class FirmataPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, FirmataDevice>();
        registry->registerFactory<StandardComponentFactory, FirmataDigitalInputChannel>();
        registry->registerFactory<StandardComponentFactory, FirmataDigitalOutputChannel>();
    }
};


extern "C" Plugin* getPlugin() {
    return new FirmataPlugin();
}


END_NAMESPACE_MW
