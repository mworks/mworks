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
using namespace mw;

mGratingData::mGratingData(const shared_ptr <Variable> &_data_sample_rate) {
	data_sample_rate = _data_sample_rate;
	
	current_data_size = MAX(2,mDriftingGratingUtilities::getNextPowerOfTwo(data_sample_rate->getValue().getInteger()));	
	data = new GLfloat[current_data_size];	
}

mGratingData::~mGratingData() {
	delete [] data;
}

const GLfloat * mGratingData::get1DData() const {
	return data;
}

unsigned int mGratingData::getDataSize() const {
	return current_data_size;
}

