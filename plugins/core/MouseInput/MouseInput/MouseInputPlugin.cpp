//
//  MouseInputPlugin.cpp
//  MouseInput
//
//  Created by Christopher Stawarz on 5/29/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#include "MouseInputDevice.h"


BEGIN_NAMESPACE_MW


class MouseInputPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) MW_OVERRIDE {
        registry->registerFactory<StandardComponentFactory, MouseInputDevice>();
    }
};


extern "C" Plugin* getPlugin() {
    return new MouseInputPlugin();
}


END_NAMESPACE_MW
