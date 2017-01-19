/*
 *  DriftingGratingStimulus.h
 *  MWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef DRIFTING_GRATNG_STIMULUS_H
#define DRIFTING_GRATNG_STIMULUS_H

#include "Mask.h"
#include "GratingData.h"


BEGIN_NAMESPACE_MW


class DriftingGratingStimulus : public StandardDynamicStimulus {
    
public:
    static const std::string DIRECTION;
    static const std::string STARTING_PHASE;
    static const std::string FREQUENCY;
    static const std::string SPEED;
    static const std::string GRATING_TYPE;
    static const std::string MASK;
    static const std::string GRATING_SAMPLE_RATE;
    static const std::string INVERTED;
    static const std::string STD_DEV;
    static const std::string MEAN;
    
    static void describeComponent(ComponentInfo &info);
    
    explicit DriftingGratingStimulus(const ParameterValueMap &parameters);
    
    void load(shared_ptr<StimulusDisplay> display) override;
    void unload(shared_ptr<StimulusDisplay> display) override;
    void drawFrame(shared_ptr<StimulusDisplay> display) override;
    Datum getCurrentAnnounceDrawData() override;
    
private:
    shared_ptr<Variable> xoffset;
    shared_ptr<Variable> yoffset;
    
    shared_ptr<Variable> width;
    shared_ptr<Variable> height;
    
    shared_ptr<Variable> rotation;
    shared_ptr<Variable> alpha_multiplier;
    
    shared_ptr<Variable> direction_in_degrees;
    shared_ptr<Variable> spatial_frequency;
    shared_ptr<Variable> speed;
    shared_ptr<Variable> starting_phase;
    
    shared_ptr<Mask> mask;
    shared_ptr<GratingData> grating;
    
    GLuint mask_texture;
    GLuint grating_texture;
    
    float last_phase;
    
};


END_NAMESPACE_MW


#endif 





















