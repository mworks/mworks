/*
 *  TriangleGratingData.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/13/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef TRIANGLE_GRATING_DATA_H
#define TRIANGLE_GRATING_DATA_H

#include "GratingData.h"

class mTriangleGratingData : public mGratingData {
public:
	mTriangleGratingData(const shared_ptr <Variable> &_data_sample_size);
	const std::string getName() const;
};

#endif 
