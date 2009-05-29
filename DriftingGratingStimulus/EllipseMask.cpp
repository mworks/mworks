/*
 *  EllipseDriftingGratingMask.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/7/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "EllipseMask.h"
using namespace mw;

mEllipseMask::mEllipseMask(const shared_ptr<Variable> &_size) : mMask(_size) {
	const float radius = current_size/2;

	// leave a 1 pixel border around the mask so when it clamps to the edge.  The array is initialized to all 
	// zeros in the base constructor
	for(unsigned int channel=0; channel<M_MASK_CHANNELS; ++channel) {
		for(unsigned int row=1; row<current_size-1; ++row) {
			for(unsigned int column=1; column<current_size-1; ++column) {
				if( sqrt((((row-radius)*(row-radius)) + ((column-radius)*(column-radius)))) <= radius) {
					data[channel+M_MASK_CHANNELS*current_size*row+M_MASK_CHANNELS*column] = 1.0;
				}
			}
		}
	}
}

const std::string mEllipseMask::getName() const {
	return "ellipse";
}


