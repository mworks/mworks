/*
 *  GratingData.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/13/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */


#include "GratingData.h"
#include "DriftingGratingUtilities.h"

#include <algorithm>

using namespace mw;

GratingData::GratingData(const shared_ptr <Variable> &_data_sample_rate) {
	data_sample_rate = _data_sample_rate;
	
	current_data_size = std::max(2u, DriftingGratingUtilities::getNextPowerOfTwo(data_sample_rate->getValue().getInteger()));
	data = new GLfloat[current_data_size];	
}

GratingData::~GratingData() {
	delete [] data;
}

const GLfloat * GratingData::get1DData() const {
	return data;
}

unsigned int GratingData::getDataSize() const {
	return current_data_size;
}

