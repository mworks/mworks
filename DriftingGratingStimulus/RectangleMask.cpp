/*
 *  RectangleDriftingGratingMask.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/7/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "RectangleMask.h"

mRectangleMask::mRectangleMask(const shared_ptr<Variable> &_size) : mMask(_size) {
	for(int k=0; k<M_MASK_CHANNELS; ++k) {
		for(int i=1; i<current_size-1; ++i) {
			for(int j=1; j<current_size-1; ++j) {
				data[k+M_MASK_CHANNELS*current_size*i+M_MASK_CHANNELS*j] = 1;
			}
		}
	}	
}

const std::string mRectangleMask::getName() const {
	return "rectangle";
}

