/*
 *  MovieStimulus.h
 *  MovieStimulusPlugin
 *
 *  Created by labuser on 5/16/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef MOVIE_STIMULUS_H_
#define MOVIE_STIMULUS_H_

#include "BaseMovieStimulus.h"


BEGIN_NAMESPACE_MW


class MovieStimulus : public BaseMovieStimulus {
    
public:
    static void describeComponent(ComponentInfo &info);
    
    explicit MovieStimulus(const ParameterValueMap &parameters);

    Datum getCurrentAnnounceDrawData() MW_OVERRIDE;
    
private:
    shared_ptr<StimulusGroup> stimulusGroup;
    
};


END_NAMESPACE_MW


#endif /* MOVIE_STIMULUS_H_ */






















