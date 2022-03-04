//
//  ISCANPlugin.cpp
//  ISCAN
//
//  Created by Christopher Stawarz on 5/20/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "ISCANDevice.hpp"


BEGIN_NAMESPACE_MW


class ISCANPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, ISCANDevice>();
    }
};


extern "C" Plugin* getPlugin() {
    return new ISCANPlugin();
}


END_NAMESPACE_MW
