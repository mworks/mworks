/*
 *  GratingData.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/7/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */


#ifndef GRATING_DATA_H
#define GRATING_DATA_H

#include "MonkeyWorksCore/GenericVariable.h"
using namespace mw;

class mGratingData {
protected:
	shared_ptr <Variable> data_sample_rate;
	GLfloat *data;
	unsigned int current_data_size;
public:
	mGratingData(const shared_ptr <Variable> &_data_sample_size);
	virtual ~mGratingData();
	virtual const GLfloat * get1DData() const;
	virtual unsigned int getDataSize() const;
	virtual const std::string getName() const = 0;
};

#endif 
