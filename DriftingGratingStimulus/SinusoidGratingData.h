/*
 *  SinusoidGratingData.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/7/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef SINUSOID_GRATING_DATA_H
#define SINUSOID_GRATING_DATA_H

#include "GratingData.h"

class mSinusoidGratingData : public mGratingData {
public:
	mSinusoidGratingData(const shared_ptr <Variable> &_data_sample_size);
	const std::string getName() const;
};

#endif 
