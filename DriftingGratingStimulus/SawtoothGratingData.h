/*
 *  SawtoothGratingData.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/13/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef SAWTOOTH_GRATING_DATA_H
#define SAWTOOTH_GRATING_DATA_H

#include "GratingData.h"

class mSawtoothGratingData : public mGratingData {
protected:
	shared_ptr<Variable> inverted;
public:
	mSawtoothGratingData(const shared_ptr <Variable> &_data_sample_size,
							const shared_ptr <Variable> &_inverted);
	const std::string getName() const;
};

#endif 
