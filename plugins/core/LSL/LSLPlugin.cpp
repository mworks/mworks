//
//  LSLPlugin.cpp
//  LSL
//
//  Created by Christopher Stawarz on 7/28/25.
//

#include "LSLDevice.hpp"


BEGIN_NAMESPACE_MW


class LSLPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, LSLDevice>();
    }
};


extern "C" Plugin * getPlugin() {
    return new LSLPlugin();
}


END_NAMESPACE_MW
