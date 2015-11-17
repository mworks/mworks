/*
 *  CoxlabCorePlugins.cpp
 *  CoxlabCorePlugins
 *
 *  Created by David Cox on 4/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "NE500PumpNetworkDevice.h"


BEGIN_NAMESPACE_MW


class NE500Plugin : public Plugin {
    void registerComponents(boost::shared_ptr<mw::ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, NE500PumpNetworkDevice>();
        registry->registerFactory<StandardComponentFactory, NE500DeviceChannel>();
    }
};


extern "C" Plugin* getPlugin() {
    return new NE500Plugin();
}


END_NAMESPACE_MW
