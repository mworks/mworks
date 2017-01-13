//
//  VideoStimulusPlugin.cpp
//  VideoStimulus
//
//  Created by Christopher Stawarz on 9/22/16.
//  Copyright © 2016 The MWorks Project. All rights reserved.
//

#include "VideoStimulus.hpp"


BEGIN_NAMESPACE_MW


class VideoStimulusPlugin : public Plugin {
    void registerComponents(boost::shared_ptr<ComponentRegistry> registry) override {
        registry->registerFactory<StandardStimulusFactory, VideoStimulus>();
    }
};


extern "C" Plugin* getPlugin() {
    return new VideoStimulusPlugin();
}


END_NAMESPACE_MW
