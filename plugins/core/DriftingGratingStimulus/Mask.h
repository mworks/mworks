/*
 *  DriftingGratingMask.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/7/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef MASK_H
#define MASK_H

#include "MWorksCore/GenericVariable.h"

using namespace mw;

#define M_MASK_CHANNELS 2

class Mask {
protected:
	GLfloat *data;
	boost::shared_ptr<Variable> size;
	unsigned int current_size;
	
	
public:
	Mask(const shared_ptr<Variable> &_size);
	virtual ~Mask();
	GLint getSize() const;
	const GLfloat *get2DData() const;
	virtual const std::string getName() const = 0;
};

#endif 
