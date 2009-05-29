/*
 *  SquareGratingData.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/7/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef SQUARE_GRATING_DATA_H
#define SQUARE_GRATING_DATA_H

#include "GratingData.h"

class mSquareGratingData : public mGratingData {
public:
	mSquareGratingData(const shared_ptr <Variable> &_data_sample_size);
	const std::string getName() const;
};

#endif 

