/*
 *  MovieStimulusPlugins.cpp
 *  MovieStimulusPlugin
 *
 *  Created by bkennedy on 8/14/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "FrameListStimulus.h"
#include "MovieStimulus.h"
#include "ImageDirectoryMovieStimulus.h"


BEGIN_NAMESPACE_MW


class MovieStimulusPlugin : public Plugin {
    void registerComponents(shared_ptr<ComponentRegistry> registry) MW_OVERRIDE {
        registry->registerFactory<StandardStimulusFactory, FrameListStimulus>();
        registry->registerFactory<StandardStimulusFactory, MovieStimulus>();
        registry->registerFactory<StandardStimulusFactory, ImageDirectoryMovieStimulus>();
    }
};


extern "C" Plugin* getPlugin() {
    return new MovieStimulusPlugin();
}


END_NAMESPACE_MW
