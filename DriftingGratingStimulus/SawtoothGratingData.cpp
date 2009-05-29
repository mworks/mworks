/*
 *  SawtoothGratingData.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/13/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "SawtoothGratingData.h"


mSawtoothGratingData::mSawtoothGratingData(const shared_ptr <Variable> &_data_sample_rate,
										   const shared_ptr <Variable> &_inverted) : mGratingData(_data_sample_rate) {
	inverted=_inverted;
	
	for(unsigned int i=0; i<current_data_size; ++i) {
		data[i] = inverted->getValue().getInteger() ? 1-((float)i/current_data_size) : (float)i/current_data_size; 
	}	
}


const std::string mSawtoothGratingData::getName() const {
	return inverted->getValue().getInteger() ? "inverted_sawtooth" : "sawtooth";
}

