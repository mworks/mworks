/*
 *  GaussianDriftingGratingMask.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/13/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "GaussianMask.h"
#include "DriftingGratingConstants.h"

mGaussianMask::mGaussianMask(const shared_ptr<Variable> &_size,
							 const shared_ptr<Variable> &_mean,
							 const shared_ptr<Variable> &_std_dev) : mMask(_size) {
	mean = _mean;
	std_dev = _std_dev;
	
	const float radius = current_size/2;
	
	// for clarity
	const float s = std_dev->getValue().getFloat();
	const float u = mean->getValue().getFloat();
	const float pi = M_DG_PI;
	
	// leave the one pixel margin around the edge
	for(unsigned int channel=0; channel<M_MASK_CHANNELS; ++channel) {
		for(unsigned int row=1; row<current_size-1; ++row) {
			for(unsigned int column=1; column<current_size-1; ++column) {
				const float d = sqrt(((row-radius)*(row-radius)) + ((column-radius)*(column-radius)))/radius;
				
				const GLfloat value = (1/(s*sqrt(2*pi)))*exp(-1*(d-u)*(d-u)/(2*s*s));
				data[channel+M_MASK_CHANNELS*current_size*row+M_MASK_CHANNELS*column] = value;
			}
		}
	}
}

const std::string mGaussianMask::getName() const {
	return "gaussian";
}


