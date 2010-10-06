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
protected:
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
	
	shared_ptr<mMask> mask;
	shared_ptr<mGratingData> grating;
	
	vector<GLuint> mask_textures;
	vector<GLuint> grating_textures;
	

    
	float last_phase;
public:
	DriftingGratingStimulus(const std::string &_tag, 
                            shared_ptr<Variable> _frames_per_second,
                            shared_ptr<Variable> _xoffset, 
                            shared_ptr<Variable> _yoffset, 
                            shared_ptr<Variable> _width,
                            shared_ptr<Variable> _height,
                            shared_ptr<Variable> _rot,
                            shared_ptr<Variable> _alpha,
                            shared_ptr<Variable> _direction,
                            shared_ptr<Variable> _frequency,
                            shared_ptr<Variable> _speed,
                            shared_ptr<Variable> _starting_phase,
                            shared_ptr<mMask> _mask,
                            shared_ptr<mGratingData> _grating);
    
   
	DriftingGratingStimulus(const DriftingGratingStimulus &tocopy);
	~DriftingGratingStimulus();
	

	virtual void load(shared_ptr<StimulusDisplay> display);
	virtual void unload(shared_ptr<StimulusDisplay> display);
    virtual void drawFrame(shared_ptr<StimulusDisplay> display, int frameNumber);
	virtual Datum getCurrentAnnounceDrawData();
};

#endif 
