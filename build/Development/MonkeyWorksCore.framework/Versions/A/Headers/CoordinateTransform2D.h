#ifndef COORDINATE_TRANSFORM_2D_H
#define COORDINATE_TRANSFORM_2D_H


#include "ExpandableList.h"
//#include "LAPACK.h"
namespace mw {
class CoordinateTransform2D{
	
	public:
		virtual void transform(float x, float y, float *x_out, float *y_out);
};


class LinearTransform2D : public CoordinateTransform2D{

	protected:
	
		float m, b, n, c;  // m & n: slopes, b & c: intercepts
		
	public:
	
		LinearTransform2D(float _m, float _b, float _n, float _c){
		
			m = _m;
			b = _b;
			n = _n;
			c = _c;
		}
		
		virtual void transform(float x, float y, float *x_out, float *y_out){
		
			*x_out = m*x + b;
			*y_out = n*y + c;
		}
};

class QuadraticTransform2D : public CoordinateTransform2D{

	// x_out = a*x^2  + b*x + c*xy + d*y + e*y^2 + f 
	protected:
	
		float a,b,c,d,e,f;
		float a2, b2, c2, d2, e2, f2;
	
	public:
	
		QuadraticTransform2D(float _a, float _b, float _c, float _d, float _e, float _f, float _a2, float _b2, float _c2, float _d2, float _e2, float _f2){
			
			a = _a;
			b = _b;
			c = _c;
			d = _d;
			e = _e;
			f = _f;

			a2 = _a2;
			b2 = _b2;
			c2 = _c2;
			d2 = _d2;
			e2 = _e2;
			f2 = _f2;

		}
		
		virtual void transform(float x, float y, float *x_out, float *y_out){
			*x_out = a*x*x + b*x + c*x*y + d*y + e*y*y + f;
			*y_out = a2*x*x + b2*x + c2*x*y + d2*y + e2*y*y + f2;
		}
};

// class ThinPlateSpineTransform2D : public CoordinateTransform2D {			 
		
	
class QuadraticCalibration{

	protected:
	
		QuadraticTransform2D *quad;
		bool ready;
		ExpandableList<float> *xactual;
		ExpandableList<float> *yactual;
		ExpandableList<float> *xmeasured;
		ExpandableList<float> *ymeasured;
		int ndatapoints;
	
	
	public:
	
		QuadraticCalibration(){
			ready = false;
			xactual = new ExpandableList<float>();
			yactual = new ExpandableList<float>();
			xmeasured = new ExpandableList<float>();
			ymeasured = new ExpandableList<float>();
			
			ndatapoints = 0;
			quad = new QuadraticTransform2D(0, 1, 0, 0, 0, 0,   0, 0, 0, 1, 0, 0); // linear

		}
	
	
		void addData(float x_actual, float y_actual, float x_measured, float y_measured){
			xactual->addElement(x_actual);
			yactual->addElement(y_actual);
			
			xmeasured->addElement(x_measured);
			ymeasured->addElement(y_measured);
			
			ndatapoints++;
		}
		
		
		void transform(float x, float y, float *x_out, float *y_out){
		
			if(quad == NULL){
				merror(M_SYSTEM_MESSAGE_DOMAIN,
					"Error: calibration not ready");
			}	
		
			quad->transform(x,y,x_out,y_out);
		}
		
		void computeCalibration(){
			
			// Solve a linear system to determine coefficents a through f2
			// x_out = a*x^2  + b*x + c*xy + d*y + e*y^2 + f 
			// y_out = a2*x^2  + b2*x + c2*xy + d2*y + e2*y^2 + f2 
			
			
			// A CLAPACK routine.  Ugly and weird to work with but portable and razor-fast.
			//
			// Solves   b = A*x
			//
			// trans (in): what type of solution to find. 'N' is standard ls
			// m (in): rows of A
			// n (in): cols of A
			// nrhs (in): number of right hand sides -- i.e. to solve two systems at once
			// a (in/out): m x n matrix
			// lda (in): set equal to or greater than m
			// b (in/out): right hand side vector
			// ldb (in): should be m, right?
			// work : workspace array of length lwork
			// lwork: length of workspace, set to size 3*n
			// info (out): 0 if okay
	/*		
			char trans = 'N';
			integer m = 2*ndatapoints;
			integer n = 12;
			integer nrhs = 1;
			doublereal *a = (doublereal *)calloc(m * n, sizeof(doublereal));
			float thisx,thisy;
			//for(int j = 0; j < n; j++){  // clapack routines are column major
			
			// okay, so this is clearly weird
			int nd = ndatapoints;
			
			int j = 0;
			for(int i = 0; i < nd; i++){
				thisx = *((*xmeasured)[i]);
				a[j*2*nd + i] = thisx*thisx;
				a[j*2*nd + i + nd] = thisx*thisx;
			}
			
			j = 1;
			for(int i = 0; i < nd; i++){
				thisx = *((*xmeasured)[i]);
				a[j*2*nd + i] = thisx;
				a[j*2*nd + i + nd] = thisx;
			}
					
			j = 2;
			for(int i = 0; i < nd; i++){
				thisx = *((*xmeasured)[i]);
				thisy = *((*ymeasured)[i]);
				a[j*2*nd + i] = thisx*thisy;
				a[j*2*nd + i + nd] = thisx*thisy;
			}

			j = 3;
			for(int i = 0; i < nd; i++){
				thisy = *((*ymeasured)[i]);
				a[j*2*nd + i] = thisy;
				a[j*2*nd + i + nd] = thisy;
			}			

			j = 4;
			for(int i = 0; i < nd; i++){
				thisy = *((*ymeasured)[i]);
				a[j*2*nd + i] = thisy*thisy;
				a[j*2*nd + i + nd] = thisy*thisy;
			}			

			j = 5;
			for(int i = 0; i < nd; i++){
				a[j*2*nd + i] = 1;
				a[j*2*nd + i + nd] = 1;
			}			


			doublereal *b = (doublereal *)calloc(m, sizeof(doublereal));
			
			for(int i = 0; i < nd; i++){
				b[i] = *((*xactual)[i]);
				b[i+nd] = *((*yactual)[i]);
			}
			
			doublereal *work = (doublereal *)calloc(3*n, sizeof(doublereal));
			integer lwork = 3*n;
			integer info;
			
			int result = dgels_(&trans, &m, &n, &nrhs, a, &m, b, &m, work, &lwork, &info); 

			float a1,b1,c1,d1,e1,f1, a2,b2,c2,d2,e2,f2;

			a1 = b[0];
			b1 = b[1];
			c1 = b[2];
			d1 = b[3];
			e1 = b[4];
			f1 = b[5];
			
			a2 = b[6];
			b2 = b[7];
			c2 = b[8];
			d2 = b[9];
			e2 = b[10];
			f2 = b[11];
			
			quad = new QuadraticTransform2D(a1,b1,c1,d1,e1,f1, a2,b2,c2,d2,e2,f2);
			ready = true;
		*/
		}
		
		
		void reset(){
		
			// x_out = a*x^2  + b*x + c*xy + d*y + e*y^2 + f
			quad = new QuadraticTransform2D(0, 1, 0, 0, 0, 0,   0, 0, 0, 1, 0, 0); // linear
			ready = false;
		}
		
		void loadCalibration(float a,float b,float c,float d,float e,float f,float a2,float b2,float c2,float d2,float e2,float f2){
			quad = new QuadraticTransform2D(a,b,c,d,e,f, a2,b2,c2,d2,e2,f2);
		}
};	
}
#endif		
	
