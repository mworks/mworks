/*
 *  DriftingGratingStimulus.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#ifndef DRIFTING_GRATNG_STIMULUS_H
#define DRIFTING_GRATNG_STIMULUS_H

#include "DynamicStimulus.h"
#include "Mask.h"
#include "GratingData.h"
using namespace mw;

class mDriftingGratingStimulus : public mDynamicStimulus {
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
	
	GLuint *mask_textures;
	GLuint *grating_textures;
	
	float last_phase;
public:
	mDriftingGratingStimulus(const std::string &_tag, 
							 const shared_ptr<Scheduler> &a_scheduler,
							 const shared_ptr<StimulusDisplay> &a_display,
							 const shared_ptr<Variable> &_frames_per_second,
							 const shared_ptr<Variable> &_statistics_reporting,
							 const shared_ptr<Variable> &_error_reporting,
							 const shared_ptr<Variable> &_xoffset, 
							 const shared_ptr<Variable> &_yoffset, 
							 const shared_ptr<Variable> &_width,
							 const shared_ptr<Variable> &_height,
							 const shared_ptr<Variable> &_rot,
							 const shared_ptr<Variable> &_alpha,
							 const shared_ptr<Variable> &_direction,
							 const shared_ptr<Variable> &_frequency,
							 const shared_ptr<Variable> &_speed,
							 const shared_ptr<Variable> &_starting_phase,
							 const shared_ptr<mMask> &_mask,
							 const shared_ptr<mGratingData> &_grating);
	mDriftingGratingStimulus(const mDriftingGratingStimulus &tocopy);
	~mDriftingGratingStimulus();
	virtual Stimulus * frozenClone() ;
	virtual void stop();
	
	virtual void draw(StimulusDisplay *display);
	virtual Data getCurrentAnnounceDrawData();
};

#endif 
