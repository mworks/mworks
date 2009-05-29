/*
 *  RectangleDriftingGratingMask.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/7/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */


#ifndef RECTANGLE_MASK_H
#define RECTANGLE_MASK_H

#include "Mask.h"

class mRectangleMask : public mMask {
public:
	mRectangleMask(const shared_ptr<Variable> &_size);
	virtual const std::string getName() const;
};

#endif 


