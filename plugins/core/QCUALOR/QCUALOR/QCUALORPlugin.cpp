//
//  QCUALORPlugin.cpp
//  QCUALOR
//
//  Created by Christopher Stawarz on 3/22/22.
//

#include "QCUALORDevice.hpp"


BEGIN_NAMESPACE_MW


class QCUALORPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, QCUALORDevice>();
    }
};


extern "C" Plugin * getPlugin() {
    return new QCUALORPlugin();
}


END_NAMESPACE_MW
