/*
 *  SquareGratingData.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/7/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "SquareGratingData.h"
#include "DriftingGratingConstants.h"


mSquareGratingData::mSquareGratingData(const shared_ptr <Variable> &_data_sample_rate) : mGratingData(_data_sample_rate) {
	for(unsigned int i=0; i<current_data_size; ++i) {
		// use cos so it matches other gratings
		data[i] = cos(2*M_DG_PI*i/current_data_size) > 0 ? 1 : 0;
	}	
}

const std::string mSquareGratingData::getName() const {
	return "square";
}
