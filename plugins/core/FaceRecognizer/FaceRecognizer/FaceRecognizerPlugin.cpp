//
//  FaceRecognizerPlugin.cpp
//  FaceRecognizer
//
//  Created by Christopher Stawarz on 5/23/19.
//  Copyright © 2019 The MWorks Project. All rights reserved.
//

#include "FaceRecognizer.hpp"


BEGIN_NAMESPACE_MW


class FaceRecognizerNotAvailableFactory : public ComponentFactory {
    boost::shared_ptr<mw::Component> createObject(StdStringMap parameters, ComponentRegistryPtr reg) override {
        throw ComponentFactoryException("Face recognizer requires macOS 10.13 or later");
    }
};


class FaceRecognizerPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        if (@available(macOS 10.13, *)) {
            registry->registerFactory<StandardComponentFactory, FaceRecognizer>();
        } else {
            registry->registerFactory("iodevice/face_recognizer",
                                      boost::make_shared<FaceRecognizerNotAvailableFactory>());
        }
    }
};


extern "C" Plugin* getPlugin() {
    return new FaceRecognizerPlugin();
}


END_NAMESPACE_MW
