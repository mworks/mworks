//
//  FaceRecognizerPlugin.cpp
//  FaceRecognizer
//
//  Created by Christopher Stawarz on 5/23/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "FaceRecognizer.hpp"


BEGIN_NAMESPACE_MW


class FaceRecognizerPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardComponentFactory, FaceRecognizer>();
    }
};


extern "C" Plugin* getPlugin() {
    return new FaceRecognizerPlugin();
}


END_NAMESPACE_MW
