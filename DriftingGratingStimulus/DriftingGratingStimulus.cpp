/*
 *  DriftingGratingStimulus.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DriftingGratingStimulus.h"
#include "DriftingGratingConstants.h"
using namespace mw;

DriftingGratingStimulus::DriftingGratingStimulus(const std::string &_tag, 
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
                                                 shared_ptr<mGratingData> _grating) :
    StandardDynamicStimulus(_tag, _frames_per_second)
{
	xoffset = registerVariable(_xoffset);
	yoffset = registerVariable(_yoffset);
	width = registerVariable(_width);
	height = registerVariable(_height);
	
	rotation = registerVariable(_rot);
	alpha_multiplier = registerVariable(_alpha);
	
	spatial_frequency = registerVariable(_frequency);
	speed = registerVariable(_speed);
	starting_phase = registerVariable(_starting_phase);
	direction_in_degrees = registerVariable(_direction);
	
	mask = _mask;
	grating = _grating;
}


void DriftingGratingStimulus::load(shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;
    
	GLuint *mask_textures_tmp = new GLuint[display->getNContexts()];
	GLuint *grating_textures_tmp = new GLuint[display->getNContexts()];
	
	glGenTextures(display->getNContexts(), mask_textures_tmp);
	glGenTextures(display->getNContexts(), grating_textures_tmp);
	
    for(int i = 0; i < display->getNContexts(); i++){
        mask_textures.push_back(mask_textures_tmp[i]);
        grating_textures.push_back(grating_textures_tmp[i]);
    }
    
    delete [] mask_textures_tmp;
    delete [] grating_textures_tmp;
    
	
	for(int i = 0; i < display->getNContexts(); ++i) {
		
        
        display->setCurrent(i);
		glDisable(GL_TEXTURE_2D);
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glEnable(GL_TEXTURE_1D);
		
        
        
        // ----------------------------------------
        //                  GRATING
        // ----------------------------------------
        
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
		
        
       
        
        
        // ----------------------------------------
        //                  MASK
        // ----------------------------------------
        
		
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
        
        // Unbind grating texture
        glActiveTextureARB(GL_TEXTURE0_ARB); 
        glBindTexture( GL_TEXTURE_1D, 0 );
        glBindTexture( GL_TEXTURE_2D, 0 );
        glDisable(GL_TEXTURE_1D);
        glDisable(GL_TEXTURE_2D);
        
        // Unbind mask texture
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture( GL_TEXTURE_1D, 0 );
        glBindTexture( GL_TEXTURE_2D, 0 );
        glDisable(GL_TEXTURE_1D);
        glDisable(GL_TEXTURE_2D);
        
        glActiveTextureARB(0);

	}
    
    loaded = true;
}   


DriftingGratingStimulus::~DriftingGratingStimulus(){
    stop();
    
    
    
    GLuint *mask_textures_tmp = new GLuint[mask_textures.size()];
    GLuint *grating_textures_tmp = new GLuint[grating_textures.size()];
    
    for(int i = 0; i < mask_textures.size(); i++){
        mask_textures_tmp[i] = mask_textures[i];
    }

    for(int i = 0; i < grating_textures.size(); i++){
        grating_textures_tmp[i] = grating_textures[i];
    }
    
    glDeleteTextures(mask_textures.size(), mask_textures_tmp);
    glDeleteTextures(grating_textures.size(), grating_textures_tmp);


    delete [] mask_textures_tmp;
    delete [] grating_textures_tmp;

}



void DriftingGratingStimulus::drawFrame(shared_ptr<StimulusDisplay> display, int frameNumber) {
    
	glPushMatrix();	
	glTranslatef(xoffset->getValue().getFloat(), yoffset->getValue().getFloat(), 0);
	glRotatef(rotation->getValue().getFloat(),0,0,1);
	GLfloat scale_size = MAX(width->getValue().getFloat(), height->getValue().getFloat());
	glScalef(scale_size, scale_size, 1.0); // scale it up		
	
    // ----------------------------------------
    //                  GRATING
    // ----------------------------------------
		
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);  // Associate with texture 0
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
	
    
    // ----------------------------------------
    //              MASK
    // ----------------------------------------

	glActiveTextureARB(GL_TEXTURE1_ARB); // associate with TEXTURE1_ARB
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
	
    
    // ----------------------------------------
    //              DRAW THE QUAD
    // ----------------------------------------
    
	glBegin(GL_QUADS);

		MWTime elapsed_time = getElapsedTime();
		
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
        double elapsed_seconds = (double)elapsed_time /  (double)1000000;
		const double phase = -1*(starting_phase->getValue().getFloat()*(M_DG_PI/180.) + speed->getValue().getFloat()*spatial_frequency->getValue().getFloat()*(2.*M_DG_PI)*elapsed_seconds);
		const double direction_in_radians = direction_in_degrees->getValue().getFloat()*(M_DG_PI/180.);		
		const double aspect = width->getValue().getFloat()/height->getValue().getFloat();
		
        //mprintf("drifting grating draw (%lld - %lld = %lld, %g, %g, %g, %g)", now, start_time, elapsed_time, elapsed_seconds, phase, direction_in_radians, aspect);
    
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
	

    // ----------------------------------------
    //                CLEAN-UP
    // ----------------------------------------
    
    // Unbind grating texture
    glActiveTextureARB(GL_TEXTURE0_ARB); 
    glBindTexture( GL_TEXTURE_1D, 0 );
    glDisable(GL_TEXTURE_1D);
    
    // Unbind mask texture
    glActiveTextureARB(GL_TEXTURE1_ARB);
    glBindTexture( GL_TEXTURE_2D, 0 );
    glDisable(GL_TEXTURE_2D);
	
    glActiveTextureARB(0);
    
	glPopMatrix();	

	last_phase = phase*(180/M_DG_PI);
}

inline Datum DriftingGratingStimulus::getCurrentAnnounceDrawData() {
	boost::mutex::scoped_lock locker(stim_lock);
	Datum announce_data = DynamicStimulusDriver::getCurrentAnnounceDrawData();
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
