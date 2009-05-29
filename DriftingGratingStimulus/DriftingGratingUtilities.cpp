/*
 *  DriftingGratingUtilities.cpp
 *  DriftingGratingStimulusPlugin
 *
 *  Created by bkennedy on 11/13/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "DriftingGratingUtilities.h"

unsigned int mDriftingGratingUtilities::getNextPowerOfTwo(unsigned int number) {
	number--;
	for (int i=1; i<sizeof(unsigned int)*8; i=i*2) {
		number = number | number >> i;
	}
	++number;
	
	return number;
}


