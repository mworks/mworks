/*
 *  TriangleGratingData.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/13/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "TriangleGratingData.h"


mTriangleGratingData::mTriangleGratingData(const shared_ptr <Variable> &_data_sample_rate) : mGratingData(_data_sample_rate) {
	for(unsigned int i=0; i<current_data_size; ++i) {
		float half_size = (float)current_data_size/2;
		
		data[i] = i < half_size ? (half_size-i)/half_size : (i-half_size)/half_size; 
	}	
}

const std::string mTriangleGratingData::getName() const {
	return "triangle";
}
