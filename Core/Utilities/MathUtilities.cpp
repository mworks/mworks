/*
 *  MathUtilities.cpp
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 9/19/05.
 *  Copyright 2005 MIT. All rights reserved.
 *
 */

#include "MathUtilities.h"
namespace mw {
	double **new2DdoubleArray(int rows, int cols) {
		
        double **A;
        A = new double* [rows];
        for(int i=0;i<rows;i++) {
            A[i]=new double [cols];
        }
        return (A);
	};
	
	
	float **new2DfloatArray(int rows, int cols) {
		
        float **A;
        A = new float* [rows];
        for(int i=0;i<rows;i++) {
            A[i]=new float [cols];
        }
        return (A);
	};
}
