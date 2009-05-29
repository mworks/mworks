/*
 *  DriftingGratingStimulus.cpp
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DriftingGratingStimulus.h"
#include "DriftingGratingConstants.h"
using namespace mw;

mDriftingGratingStimulus::mDriftingGratingStimulus(const std::string &_tag, 
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
												   const shared_ptr<mGratingData> &_grating) : mDynamicStimulus (_tag,
																												 a_scheduler,
																												 a_display,
																												 _frames_per_second,
																												 _statistics_reporting,
																												 _error_reporting) {
	
	
	
	xoffset = _xoffset;
	yoffset = _yoffset;
	width = _width;
	height = _height;
	
	rotation = _rot;
	alpha_multiplier = _alpha;
	
	spatial_frequency = _frequency;
	speed = _speed;
	starting_phase = _starting_phase;
	direction_in_degrees = _direction;
	
	mask = _mask;
	grating = _grating;
	
	mask_textures = new GLuint[display->getNContexts()];
	grating_textures = new GLuint[display->getNContexts()];
	
	glGenTextures(display->getNContexts(), mask_textures);
	glGenTextures(display->getNContexts(), grating_textures);
	
	
	for(int i = 0; i < display->getNContexts(); ++i) {
		display->setCurrent(i);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_1D);
		
		
		// select our current texture
		glBindTexture( GL_TEXTURE_1D, grating_textures[i] );
		
		// select modulate to mix texture with color for shading
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
		
		// when texture area is small, bilinear filter the closest mipmap
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
		// when texture area is large, bilinear filter the first mipmap
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		
		// if wrap is true, the texture wraps over at the edges (repeat)
		//       ... false, the texture ends at the edges (clamp)
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		
		gluBuild1DMipmaps(GL_TEXTURE_1D,
						  GL_LUMINANCE,
						  grating->getDataSize(),
						  GL_LUMINANCE,
						  GL_FLOAT,
						  grating->get1DData());
		
		glEnable(GL_TEXTURE_1D);
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(1,1,1,alpha_multiplier->getValue().getFloat()); //R,G,B,A
		
		//////build mask
		
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glEnable(GL_TEXTURE_2D);
		
		// select our current texture
		glBindTexture( GL_TEXTURE_2D, mask_textures[i] );
		
		// select modulate to mix texture with color for shading
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
		
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );	
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		
		gluBuild2DMipmaps(GL_TEXTURE_2D,
						  2,
						  mask->getSize(),
						  mask->getSize(),
						  GL_LUMINANCE_ALPHA,
						  GL_FLOAT,
						  mask->get2DData());
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
	}
}   

mDriftingGratingStimulus::~mDriftingGratingStimulus(){
	glDeleteTextures(display->getNContexts(), mask_textures);
	glDeleteTextures(display->getNContexts(), grating_textures);
	
	delete [] mask_textures;
	delete [] grating_textures;
}


Stimulus * mDriftingGratingStimulus::frozenClone() {
	shared_ptr<Variable> frames_per_second_clone(frames_per_second->frozenClone());
	shared_ptr<Variable> x_offset_clone(xoffset->frozenClone());
	shared_ptr<Variable> y_offset_clone(yoffset->frozenClone());
	shared_ptr<Variable> width_clone(width->frozenClone());
	shared_ptr<Variable> height_clone(height->frozenClone());
	shared_ptr<Variable> rotation_clone(rotation->frozenClone());
	shared_ptr<Variable> alpha_clone(alpha_multiplier->frozenClone());
	shared_ptr<Variable> direction_clone(direction_in_degrees->frozenClone());
	shared_ptr<Variable> frequency_clone(spatial_frequency->frozenClone());
	shared_ptr<Variable> speed_clone(speed->frozenClone());
	shared_ptr<Variable> starting_phase_clone(starting_phase->frozenClone());
	
	Stimulus *cloned_stimulus = new mDriftingGratingStimulus(tag,
															  scheduler,
															  display,
															  frames_per_second_clone,
															  statistics_reporting,
															  error_reporting,
															  x_offset_clone,
															  y_offset_clone,
															  width_clone,
															  height_clone,
															  rotation_clone,
															  alpha_clone,
															  direction_clone,
															  frequency_clone,
															  speed_clone,
															  starting_phase_clone,
															  mask,
															  grating);
	
	return cloned_stimulus;
}

void mDriftingGratingStimulus::stop() {
}

void mDriftingGratingStimulus::draw(StimulusDisplay * display) {
	glPushMatrix();	
	glTranslatef(xoffset->getValue().getFloat(), yoffset->getValue().getFloat(), 0);
	glRotatef(rotation->getValue().getFloat(),0,0,1);
	GLfloat scale_size = MAX(width->getValue().getFloat(), height->getValue().getFloat());
	glScalef(scale_size, scale_size, 1.0); // scale it up		
	
	
	
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_1D);
	
	
    // select our current texture
    glBindTexture( GL_TEXTURE_1D, grating_textures[display->getCurrentContextIndex()] );
	
    // select modulate to mix texture with color for shading
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
	
    // when texture area is small, bilinear filter the closest mipmap
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first mipmap
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	
    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	
	glEnable(GL_TEXTURE_1D);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1,1,1,alpha_multiplier->getValue().getFloat()); //R,G,B,A
	
	//////build mask
	
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);
	
    // select our current texture
    glBindTexture( GL_TEXTURE_2D, mask_textures[display->getCurrentContextIndex()] );
	
    // select modulate to mix texture with color for shading
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
	
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );	
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_ARB);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB_ARB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB_ARB, GL_PREVIOUS);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA_ARB, GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE);
	glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA);
	
	glBegin(GL_QUADS);

		
		const MonkeyWorksTime elapsed_time = (scheduler->getClock()->getCurrentTimeUS() - start_time);
		
		// here's the description of this equation
		// starting_phase is in degrees ->  degrees*pi/180 = radians
		// 
		// speed is in degrees/second
		// spatial_frequency is in cycles/degree
		// elapsed_time is time since the start the 'play' in us
		// us/100000 = seconds
		//
		// degrees   cycles    1 second    pi radians
		// ------- * ------ * ---------- * ----------- * us = radians of phase
		//    s      degree   1000000 us   180 degrees
		//
		// multiply by -1 so it the grating goes in the correct direction
		const float phase = -1*(starting_phase->getValue().getFloat()*(M_DG_PI/180) + speed->getValue().getFloat()*spatial_frequency->getValue().getFloat()*(2*M_DG_PI)*elapsed_time/1000000);
		const float direction_in_radians = direction_in_degrees->getValue().getFloat()*(M_DG_PI/180);		
		const float aspect = width->getValue().getFloat()/height->getValue().getFloat();
		
		const float f = cos(direction_in_radians);
		const float g = sin(direction_in_radians);
		const float d = ((f+g)-1)/2;
		const float texture_bl = 0-d;		
		const float texture_br = texture_bl+f;
		const float texture_tr = 1+d;
		const float texture_tl = texture_bl+g;
		
		const float mask_s_ratio = 1-MIN(1,aspect);
		const float mask_t_ratio = 1-MIN(1,1/aspect);
		
		const float phase_proportion = phase/(2*M_DG_PI);
		const float cycle_proportion = spatial_frequency->getValue().getFloat()*width->getValue().getFloat();
		
		glNormal3f(0.0, 0.0, 1.0);
		
		glMultiTexCoord1fARB(GL_TEXTURE0_ARB, (cycle_proportion*texture_bl)+phase_proportion);
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0-mask_s_ratio, 0-mask_t_ratio);
		glVertex3f(-0.5,-0.5,0);
		
		glMultiTexCoord1fARB(GL_TEXTURE0_ARB, (cycle_proportion*texture_br)+phase_proportion);
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1+mask_s_ratio, 0-mask_t_ratio);
		glVertex3f(0.5,-0.5,0);
		
		glMultiTexCoord1fARB(GL_TEXTURE0_ARB, (cycle_proportion*texture_tr)+phase_proportion);
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1+mask_s_ratio, 1+mask_t_ratio);
		glVertex3f(0.5,0.5,0);
		
		glMultiTexCoord1fARB(GL_TEXTURE0_ARB, (cycle_proportion*texture_tl)+phase_proportion);
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0-mask_s_ratio, 1+mask_t_ratio);
		glVertex3f(-0.5,0.5,0);
	
	glEnd(); // GL_QUADS		
	
	
	glDisable(GL_TEXTURE_1D);
	
	glPopMatrix();	

	boost::mutex::scoped_lock locker(stim_lock);
	last_phase = phase*(180/M_DG_PI);
}

inline Data mDriftingGratingStimulus::getCurrentAnnounceDrawData() {
	boost::mutex::scoped_lock locker(stim_lock);
	Data announce_data = mDynamicStimulus::getCurrentAnnounceDrawData();
	announce_data.addElement("rotation", rotation->getValue());
	announce_data.addElement("xoffset", xoffset->getValue());
	announce_data.addElement("yoffset", yoffset->getValue());
	announce_data.addElement("width", width->getValue());
	announce_data.addElement("height", height->getValue());
	announce_data.addElement("alpha_multiplier", alpha_multiplier->getValue());
	announce_data.addElement("frequency", spatial_frequency->getValue());
	announce_data.addElement("speed", speed->getValue());
	announce_data.addElement("starting_phase", starting_phase->getValue());
	announce_data.addElement("current_phase", last_phase);
	announce_data.addElement("direction", direction_in_degrees->getValue());
	announce_data.addElement("grating", grating->getName());
	announce_data.addElement("mask", mask->getName());
	
	return announce_data;
}
