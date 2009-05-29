/*
 *  GaussianDriftingGratingMask.h
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/13/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef GAUSSIAN_MASK_H
#define GAUSSIAN_MASK_H

#include "Mask.h"

class mGaussianMask : public mMask {
protected:
	shared_ptr<Variable> mean;
	shared_ptr<Variable> std_dev;
	
public:
	mGaussianMask(const shared_ptr<Variable> &_size,
									 const shared_ptr<Variable> &_mean,
									 const shared_ptr<Variable> &_std_dev);
	virtual const std::string getName() const;
};



#endif
