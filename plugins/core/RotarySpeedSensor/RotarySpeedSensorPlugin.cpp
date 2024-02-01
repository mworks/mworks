//
//  RotarySpeedSensorPlugin.cpp
//  RotarySpeedSensor
//
//  Created by Christopher Stawarz on 2/1/24.
//

#include "RotarySpeedSensorDevice.hpp"


BEGIN_NAMESPACE_MW


class RotarySpeedSensorPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, RotarySpeedSensorDevice>();
    }
};


extern "C" Plugin * getPlugin() {
    return new RotarySpeedSensorPlugin();
}


END_NAMESPACE_MW
