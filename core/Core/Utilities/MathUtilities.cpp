/*
 *  MathUtilities.cpp
 *  MWorksCore
 *
 *  Created by dicarlo on 9/19/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "MathUtilities.h"
namespace mw {
	float **new2DfloatArray(int rows, int cols) {
		
        float **A;
        A = new float* [rows];
        for(int i=0;i<rows;i++) {
            A[i]=new float [cols];
        }
        return (A);
	};

    
    void delete2DfloatArray(float **arr, int rows) {
        for (int i = 0; i < rows; i++) {
            delete [] arr[i];
        }
        delete [] arr;
    }
}
