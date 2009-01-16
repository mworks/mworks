/*
 *  SVDfit.h
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 9/16/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _SVDFIT
#define _SVDFIT

//#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>
#include "MathUtilities.h"
#include "MonkeyWorksCore/Experiment.h"   // to get merror function
namespace mw {

#define TOL 1.0e-5
//#define NRANSI

// prototypes -----

float pythag(float a, float b);

class SVDfit {

    protected:
        
        float **u;
        float **v;
        float *w;
        float *a;
        float *y;
        float *sig;
        int nParams, ma;
        int nData, ndata;
        
        void svbksb(int m, int n, float b[], float x[]);
        void svdcmp(int m, int n);

    public:
    
        SVDfit(float **X, float *Y, float *SD, int nData, int nParams);
        ~SVDfit();
        bool doFit(float *Parameters, int *nParameters, float *chisq);

};

static inline float FMAX(const float a, const float b) {
	return a > b ? a : b;
}

static inline int IMIN(const int a, const int b) {
	return a < b ? a : b;
}

static inline float SQR(const float a) {
	return a*a;
}

static inline float SIGN(const float a, const float b) {
	return b >= 0.0 ? fabs(a) : -fabs(a);
}
}
#endif
