//
//  HighPrecisionClockPlugin.cpp
//  HighPrecisionClock
//
//  Created by Christopher Stawarz on 4/12/13.
//  Copyright (c) 2013 The MWorks Project. All rights reserved.
//

#include "HighPrecisionClock.h"


BEGIN_NAMESPACE_MW


class SimpleClockFactory : public ComponentFactory {
    boost::shared_ptr<Component> createObject(StdStringMap parameters, ComponentRegistryPtr reg) override {
        return boost::make_shared<SimpleClock>();
    }
};


class HighPrecisionClockFactory : public ComponentFactory {
    boost::shared_ptr<Component> createObject(StdStringMap parameters, ComponentRegistryPtr reg) MW_OVERRIDE {
        return boost::make_shared<HighPrecisionClock>();
    }
};


class HighPrecisionClockPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) MW_OVERRIDE {
        registry->registerFactory("SimpleClock", boost::make_shared<SimpleClockFactory>());
        registry->registerFactory("HighPrecisionClock", boost::make_shared<HighPrecisionClockFactory>());
        // HighPrecisionClock replaced MachClock, but keep MachClock as an alias to support
        // old configuration files that specify it
        registry->registerFactoryAlias("HighPrecisionClock", "MachClock");
    }
};


extern "C" Plugin* getPlugin() {
    return new HighPrecisionClockPlugin();
}


END_NAMESPACE_MW
