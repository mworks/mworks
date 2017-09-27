//
//  RCBLVDSPlugin.cpp
//  RCBLVDS
//
//  Created by Christopher Stawarz on 9/13/17.
//  Copyright © 2017 The MWorks Project. All rights reserved.
//

#include "RCBLVDSDevice.hpp"


BEGIN_NAMESPACE_MW


class RCBLVDSPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, RCBLVDSDevice>();
        registry->registerFactory<StandardComponentFactory, RCBLVDSDigitalInputChannel>();
    }
};


extern "C" Plugin* getPlugin() {
    return new RCBLVDSPlugin();
}


END_NAMESPACE_MW
