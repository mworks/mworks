/*
 *  MathUtilities.h
 *  MWorksCore
 *
 *  Created by dicarlo on 9/19/05.
 *  Copyright 2005 MIT.. All rights reserved.
 *
 *  DDC 12/09: really?
 *
 */

#ifndef _MATH_UTILITIES
#define _MATH_UTILITIES


#include "MWorksMacros.h"


BEGIN_NAMESPACE_MW


float  **new2DfloatArray(int rows, int cols);
void delete2DfloatArray(float **arr, int rows);


END_NAMESPACE_MW


#endif