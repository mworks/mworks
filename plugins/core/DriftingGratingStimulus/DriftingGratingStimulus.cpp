/*
 *  DriftingGratingStimulus.cpp
 *  MWorksCore
 *
 *  Created by bkennedy on 8/26/08.
 *  Copyright 2008 MIT. All rights reserved.
 *
 */

#include "DriftingGratingStimulus.h"
#include "EllipseMask.h"
#include "GaussianMask.h"
#include "RectangleMask.h"
#include "SawtoothGratingData.h"
#include "SinusoidGratingData.h"
#include "SquareGratingData.h"
#include "TriangleGratingData.h"

#include <MWorksCore/StandardStimuli.h>

#include <algorithm>


const std::string DriftingGratingStimulus::DIRECTION("direction");
const std::string DriftingGratingStimulus::STARTING_PHASE("starting_phase");
const std::string DriftingGratingStimulus::FREQUENCY("spatial_frequency");
const std::string DriftingGratingStimulus::SPEED("speed");
const std::string DriftingGratingStimulus::GRATING_TYPE("grating_type");
const std::string DriftingGratingStimulus::MASK("mask");
const std::string DriftingGratingStimulus::GRATING_SAMPLE_RATE("grating_sample_rate");
const std::string DriftingGratingStimulus::INVERTED("inverted");
const std::string DriftingGratingStimulus::STD_DEV("std_dev");
const std::string DriftingGratingStimulus::MEAN("mean");


void DriftingGratingStimulus::describeComponent(ComponentInfo &info) {
    StandardDynamicStimulus::describeComponent(info);
    
    info.setSignature("stimulus/drifting_grating");

    info.addParameter(BasicTransformStimulus::X_SIZE);
    info.addParameter(BasicTransformStimulus::Y_SIZE);
    info.addParameter(BasicTransformStimulus::X_POSITION, "0.0");
    info.addParameter(BasicTransformStimulus::Y_POSITION, "0.0");
    info.addParameter(BasicTransformStimulus::ROTATION, "0.0");
    info.addParameter(BasicTransformStimulus::ALPHA_MULTIPLIER, "1.0");
    
    info.addParameter(DIRECTION, "0.0");
    info.addParameter(STARTING_PHASE, "0.0");
    info.addParameter(FREQUENCY);
    info.addParameter(SPEED);
    info.addParameter(GRATING_TYPE);
    info.addParameter(MASK);
    info.addParameter(GRATING_SAMPLE_RATE, "32");
    info.addParameter(INVERTED, "0");
    info.addParameter(STD_DEV, "1.0");
    info.addParameter(MEAN, "0.0");
}


DriftingGratingStimulus::DriftingGratingStimulus(const ParameterValueMap &parameters) :
    StandardDynamicStimulus(parameters),
    xoffset(registerVariable(parameters[BasicTransformStimulus::X_POSITION])),
    yoffset(registerVariable(parameters[BasicTransformStimulus::Y_POSITION])),
    width(registerVariable(parameters[BasicTransformStimulus::X_SIZE])),
    height(registerVariable(parameters[BasicTransformStimulus::Y_SIZE])),
    rotation(registerVariable(parameters[BasicTransformStimulus::ROTATION])),
    alpha_multiplier(registerVariable(parameters[BasicTransformStimulus::ALPHA_MULTIPLIER])),
    spatial_frequency(registerVariable(parameters[FREQUENCY])),
    speed(registerVariable(parameters[SPEED])),
    starting_phase(registerVariable(parameters[STARTING_PHASE])),
    direction_in_degrees(registerVariable(parameters[DIRECTION]))
{
    const std::string &grating_type = parameters[GRATING_TYPE].str();
    shared_ptr<Variable> grating_sample_rate(parameters[GRATING_SAMPLE_RATE]);

	if (grating_type == "sinusoid") {
		grating = shared_ptr<SinusoidGratingData>(new SinusoidGratingData(grating_sample_rate));
	} else if (grating_type == "square") {
		grating = shared_ptr<SquareGratingData>(new SquareGratingData(grating_sample_rate));
	} else if (grating_type == "triangle") {
		grating = shared_ptr<TriangleGratingData>(new TriangleGratingData(grating_sample_rate));
	} else if (grating_type == "sawtooth") {
		grating = shared_ptr<SawtoothGratingData>(new SawtoothGratingData(grating_sample_rate,
                                                                          VariablePtr(parameters[INVERTED])));
	} else {
		throw SimpleException("illegal grating type", grating_type);		
	}

    const std::string &mask_type = parameters[MASK].str();
	shared_ptr<Variable> mask_size(new ConstantVariable(128L));

	if (mask_type == "rectangle") {
		mask = shared_ptr<Mask>(new RectangleMask(mask_size));
	} else if (mask_type == "ellipse") {
		mask = shared_ptr<Mask>(new EllipseMask(mask_size));
	} else if (mask_type == "gaussian") {
		mask = shared_ptr<Mask>(new GaussianMask(mask_size,
                                                 VariablePtr(parameters[MEAN]),
                                                 VariablePtr(parameters[STD_DEV])));
	} else {
		throw SimpleException("illegal mask", mask_type);				
	}
}


void DriftingGratingStimulus::load(shared_ptr<StimulusDisplay> display) {
    if (loaded)
        return;
	
	for(int i = 0; i < display->getNContexts(); ++i) {
		
        
        OpenGLContextLock ctxLock = display->setCurrent(i);
        
        GLuint textures[2];
        glGenTextures(2, textures);
        mask_textures.push_back(textures[0]);
        grating_textures.push_back(textures[1]);
        
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
		glColor4f(1,1,1,1); 
        
       
        
        
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


void DriftingGratingStimulus::unload(shared_ptr<StimulusDisplay> display) {
    if (!loaded)
        return;

    for (int i = 0; i < display->getNContexts(); i++) {
        OpenGLContextLock ctxLock = display->setCurrent(i);
        glDeleteTextures(1, &(mask_textures[i]));
        glDeleteTextures(1, &(grating_textures[i]));
    }
    
    mask_textures.clear();
    grating_textures.clear();
    
    loaded = false;
}


void DriftingGratingStimulus::drawFrame(shared_ptr<StimulusDisplay> display) {
	glPushMatrix();	
	glTranslatef(xoffset->getValue().getFloat(), yoffset->getValue().getFloat(), 0);
	glRotatef(rotation->getValue().getFloat(),0,0,1);
	GLfloat scale_size = std::max(width->getValue().getFloat(), height->getValue().getFloat());
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
	glColor4f(1,1,1,1);
	
    
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
		const double phase = -1*(starting_phase->getValue().getFloat()*(M_PI/180.) + speed->getValue().getFloat()*spatial_frequency->getValue().getFloat()*(2.*M_PI)*elapsed_seconds);
		const double direction_in_radians = direction_in_degrees->getValue().getFloat()*(M_PI/180.);		
		const double aspect = width->getValue().getFloat()/height->getValue().getFloat();
		
        //mprintf("drifting grating draw (%lld - %lld = %lld, %g, %g, %g, %g)", now, start_time, elapsed_time, elapsed_seconds, phase, direction_in_radians, aspect);
    
		const float f = cos(direction_in_radians);
		const float g = sin(direction_in_radians);
		const float d = ((f+g)-1)/2;
		const float texture_bl = 0-d;		
		const float texture_br = texture_bl+f;
		const float texture_tr = 1+d;
		const float texture_tl = texture_bl+g;
		
		const float mask_s_ratio = 1-std::min(1.0,aspect);
		const float mask_t_ratio = 1-std::min(1.0,1.0/aspect);
		
		const float phase_proportion = phase/(2*M_PI);
		const float cycle_proportion = spatial_frequency->getValue().getFloat() * scale_size;
		
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
    
	last_phase = phase*(180/M_PI);
	
	
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
	
	
    //
    // If alpha_multiplier is less than 1, draw another polygon on top to reduce contrast
    //

    float tB = alpha_multiplier->getValue().getFloat();

    if ((tB < 1.0f) && (tB >= 0.0f)) {
        glBegin(GL_QUADS);

            glColor4f(0.5,0.5,0.5,1-tB);   // MH 100730 I believe all grating types have mean 0.5 hardcoded now, fade to 0.5
        
            glVertex3f(-0.5,-0.5,0);
            glVertex3f(0.5,-0.5,0);
            glVertex3f(0.5,0.5,0);
            glVertex3f(-0.5,0.5,0);

        glEnd();  // GL_QUADS
    }
    

	glPopMatrix();	
}


Datum DriftingGratingStimulus::getCurrentAnnounceDrawData() {
	boost::mutex::scoped_lock locker(stim_lock);
	Datum announce_data = StandardDynamicStimulus::getCurrentAnnounceDrawData();

	announce_data.addElement(STIM_TYPE, "drifting_grating");  
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

























