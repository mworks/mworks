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

#include <MWorksCore/StandardDynamicStimulus.h>

#include "Mask.h"
#include "GratingData.h"

using namespace mw;


class DriftingGratingStimulus : public StandardDynamicStimulus {

private:
	shared_ptr<Variable> xoffset;
	shared_ptr<Variable> yoffset;
	
	shared_ptr<Variable> width;
	shared_ptr<Variable> height;
	
	shared_ptr<Variable> rotation; // planar rotation added in for free
	shared_ptr<Variable> alpha_multiplier;
	
	shared_ptr<Variable> direction_in_degrees;
	shared_ptr<Variable> spatial_frequency;
	shared_ptr<Variable> speed;
	shared_ptr<Variable> starting_phase;
	
	shared_ptr<Mask> mask;
	shared_ptr<GratingData> grating;
	
	vector<GLuint> mask_textures;
	vector<GLuint> grating_textures;
    
	float last_phase;

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
    
	virtual ~DriftingGratingStimulus() { }

	virtual void load(shared_ptr<StimulusDisplay> display);
	virtual void unload(shared_ptr<StimulusDisplay> display);
    virtual void drawFrame(shared_ptr<StimulusDisplay> display);
	virtual Datum getCurrentAnnounceDrawData();

};


#endif 





















