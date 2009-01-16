/*
 *  SVDfit.cpp
 *  MonkeyWorksCore
 *
 *  Created by dicarlo on 9/16/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "SVDfit.h"
using namespace mw;

// ported from DLAB code

// SVD from numerical recipies
// all data (matricies) should start at index value 1 !!! (thats what it expects)
//	alternatively -- pass the matric pointer so that it is pointing at the address
//	  before the address at which you first have data.

// functions ---------
// when JJD ported this, he made it such that we now pass in three things 
//  in the constructor:
//  The linear model is: Xb = Y in all of JJD core code
//  1) the matrix describing the basis results  (X) (nData x nParams)
//  2) the observed data values (target values) (Y)  (nData x 1)
//  3) the error in each target value           (SD) (nData x 1)
//
//  The numerical recipes used different terminology:  ua = y
//  (and that is still embedded in this code)
//
//  Thus, when you see the following vars in this piece of C code, here is the mapping:
//  
//  see:  u  (think JJD X)
//  see:  a  (think JJD b = "Parameters")
//  see:  y  (think JJD Y)
//  see:  b  (think JJD Y weighted by SD)

// data are passed using standard JJD notation.
//  The model is:  Xb = Y
SVDfit::SVDfit(float **X, float *Y, float *SD, int nData, int nParams) {// inputs
                        
        // create memory space and copy data into appropriate matricies
        //  we make this copy because the code works directly on these
        //  (also add 1 to all indices) (FORTRAN style)
        // X ==> u    Y ==> y    SD ==> sig       
        
        // mapping            
        ndata = nData;            
        ma = nParams;
        
        // create the memory workspaces
        u = new2DfloatArray(ndata+1,ma+1);   
        v = new2DfloatArray(ma+1,ma+1);
        w = new float [ma+1];
        a = new float [ma+1];
        y = new float [ndata+1];
        sig = new float [ndata+1];
        
        // fill the matricies
        for (int i=1;i<=ndata;i++) {
            for (int j=1;j<=ma;j++) {
                u[i][j]=X[i-1][j-1];
            }
            y[i]=Y[i-1];
        }

        // weighting 
        if (SD == NULL) {
            for (int i=1;i<=ndata;i++) sig[i] = 1;      // no weighting
        }
        else {
            for (int i=1;i<=ndata;i++) sig[i] = SD[i-1];
        }

        
}

SVDfit::~SVDfit() {

    delete u;       // TODO -- will this actually delete all memory for the 2D array?
    delete v;
    delete w;
    delete a;
    delete y;
    delete sig;
    
}

// return the set of best fit parameters
bool SVDfit::doFit(float *Parameters, int *nParameters, float *chisq) {           // outputs

	int j,i;
	float wmax,tmp,thresh;
	//float sum;
    	
    float *b = new float [ndata+1];
     
    // put in weighting for SD
	for (i=1;i<=ndata;i++) {
		tmp=1.0/sig[i];
		for (j=1;j<=ma;j++) {
            u[i][j]=u[i][j]*tmp;
        }
		b[i]=y[i]*tmp;
	}
    
    // create the pseudoinverse
	//svdcmp(u,ndata,ma,w,v);			// originally, u is the input (and output) matrix to svdcmp
	svdcmp(ndata,ma);					
		
	wmax=0.0;
	for (j=1;j<=ma;j++)
		if (w[j] > wmax) wmax=w[j];
	thresh=TOL*wmax;
	for (j=1;j<=ma;j++)
		if (w[j] < thresh) w[j]=0.0;
	
    
    // apply the pseudo inverse
	svbksb(ndata,ma,b,a);					// b is the data vectore weighted by sd of data
											// a is the output matrix


    // move from fortran style to c style (index start at 0)
    for (j=1;j<=ma;j++) {
       Parameters[j-1] = a[j];
    }
   	*nParameters = ma;     // return number of params
    
    
    
    // TODO -- this must be updated (error in fit)
	*chisq=0.0;
    /*
	for (i=1;i<=ndata;i++) {
    
		(*funcs)((short)(x[i]),afunc,ma);
		for (sum=0.0,j=1;j<=ma;j++) {
            sum += a[j]*afunc[j];
        }
		*chisq += (tmp=(y[i]-sum)/sig[i],tmp*tmp);
	}
    */

    delete b;
    
    return (true);
	
}

/* (C) Copr. 1986-92 Numerical Recipes Software ):5-). */

void SVDfit::svdcmp(int m, int n)
{
	float pythag(float a, float b);
	int flag,i,its,j,jj,k,l,nm;
	float anorm,c,f,g,h,s,scale,x,y,z;

    float *rv1 = new float [n];
	
	g=scale=anorm=0.0;
	for (i=1;i<=n;i++) {
		l=i+1;
		rv1[i]=scale*g;
		g=s=scale=0.0;
		if (i <= m) {
			for (k=i;k<=m;k++) scale += fabs(u[k][i]);
			if (scale) {
				for (k=i;k<=m;k++) {
					u[k][i] /= scale;
					s += u[k][i]*u[k][i];
				}
				f=u[i][i];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				u[i][i]=f-g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=i;k<=m;k++) s += u[k][i]*u[k][j];
					f=s/h;
					for (k=i;k<=m;k++) u[k][j] += f*u[k][i];
				}
				for (k=i;k<=m;k++) u[k][i] *= scale;
			}
		}
		w[i]=scale *g;
		g=s=scale=0.0;
		if (i <= m && i != n) {
			for (k=l;k<=n;k++) scale += fabs(u[i][k]);
			if (scale) {
				for (k=l;k<=n;k++) {
					u[i][k] /= scale;
					s += u[i][k]*u[i][k];
				}
				f=u[i][l];
				g = -SIGN(sqrt(s),f);
				h=f*g-s;
				u[i][l]=f-g;
				for (k=l;k<=n;k++) rv1[k]=u[i][k]/h;
				for (j=l;j<=m;j++) {
					for (s=0.0,k=l;k<=n;k++) s += u[j][k]*u[i][k];
					for (k=l;k<=n;k++) u[j][k] += s*rv1[k];
				}
				for (k=l;k<=n;k++) u[i][k] *= scale;
			}
		}
		anorm=FMAX(anorm,(fabs(w[i])+fabs(rv1[i])));
	}
	for (i=n;i>=1;i--) {
		if (i < n) {
			if (g) {
				for (j=l;j<=n;j++)
					v[j][i]=(u[i][j]/u[i][l])/g;
				for (j=l;j<=n;j++) {
					for (s=0.0,k=l;k<=n;k++) s += u[i][k]*v[k][j];
					for (k=l;k<=n;k++) v[k][j] += s*v[k][i];
				}
			}
			for (j=l;j<=n;j++) v[i][j]=v[j][i]=0.0;
		}
		v[i][i]=1.0;
		g=rv1[i];
		l=i;
	}
	for (i=IMIN(m,n);i>=1;i--) {
		l=i+1;
		g=w[i];
		for (j=l;j<=n;j++) u[i][j]=0.0;
		if (g) {
			g=1.0/g;
			for (j=l;j<=n;j++) {
				for (s=0.0,k=l;k<=m;k++) s += u[k][i]*u[k][j];
				f=(s/u[i][i])*g;
				for (k=i;k<=m;k++) u[k][j] += f*u[k][i];
			}
			for (j=i;j<=m;j++) u[j][i] *= g;
		} else for (j=i;j<=m;j++) u[j][i]=0.0;
		++u[i][i];
	}
	for (k=n;k>=1;k--) {
		for (its=1;its<=30;its++) {
			flag=1;
			for (l=k;l>=1;l--) {
				nm=l-1;
				if ((float)(fabs(rv1[l])+anorm) == anorm) {
					flag=0;
					break;
				}
				if ((float)(fabs(w[nm])+anorm) == anorm) break;
			}
			if (flag) {
				c=0.0;
				s=1.0;
				for (i=l;i<=k;i++) {
					f=s*rv1[i];
					rv1[i]=c*rv1[i];
					if ((float)(fabs(f)+anorm) == anorm) break;
					g=w[i];
					h=pythag(f,g);
					w[i]=h;
					h=1.0/h;
					c=g*h;
					s = -f*h;
					for (j=1;j<=m;j++) {
						y=u[j][nm];
						z=u[j][i];
						u[j][nm]=y*c+z*s;
						u[j][i]=z*c-y*s;
					}
				}
			}
			z=w[k];
			if (l == k) {
				if (z < 0.0) {
					w[k] = -z;
					for (j=1;j<=n;j++) v[j][k] = -v[j][k];
				}
				break;
			}
			if (its == 30) merror(M_SYSTEM_MESSAGE_DOMAIN, 
								"SVD no convergence in 30 svdcmp iterations");
			x=w[l];
			nm=k-1;
			y=w[nm];
			g=rv1[nm];
			h=rv1[k];
			f=((y-z)*(y+z)+(g-h)*(g+h))/(2.0*h*y);
			g=pythag(f,1.0);
			f=((x-z)*(x+z)+h*((y/(f+SIGN(g,f)))-h))/x;
			c=s=1.0;
			for (j=l;j<=nm;j++) {
				i=j+1;
				g=rv1[i];
				y=w[i];
				h=s*g;
				g=c*g;
				z=pythag(f,h);
				rv1[j]=z;
				c=f/z;
				s=h/z;
				f=x*c+g*s;
				g = g*c-x*s;
				h=y*s;
				y *= c;
				for (jj=1;jj<=n;jj++) {
					x=v[jj][j];
					z=v[jj][i];
					v[jj][j]=x*c+z*s;
					v[jj][i]=z*c-x*s;
				}
				z=pythag(f,h);
				w[j]=z;
				if (z) {
					z=1.0/z;
					c=f*z;
					s=h*z;
				}
				f=c*g+s*y;
				x=c*y-s*g;
				for (jj=1;jj<=m;jj++) {
					y=u[jj][j];
					z=u[jj][i];
					u[jj][j]=y*c+z*s;
					u[jj][i]=z*c-y*s;
				}
			}
			rv1[l]=0.0;
			rv1[k]=f;
			w[k]=x;
		}
	}
	delete rv1;
    
}

/* (C) Copr. 1986-92 Numerical Recipes Software ):5-). */


void SVDfit::svbksb(int m, int n, float b[], float x[])
{
	int jj,j,i;
	float s;
	
    float *tmp = new float [n];
	//tmp=vector(1,n);
	
	for (j=1;j<=n;j++) {
		s=0.0;
		if (w[j]) {
			for (i=1;i<=m;i++) s += u[i][j]*b[i];
			s /= w[j];
		}
		tmp[j]=s;
	}
	for (j=1;j<=n;j++) {
		s=0.0;
		for (jj=1;jj<=n;jj++) s += v[j][jj]*tmp[jj];
		x[j]=s;
	}
	//free_vector(tmp,1,n);
    delete tmp;
}



float pythag(float a, float b)
{
	float absa,absb;
	absa=fabs(a);
	absb=fabs(b);
	if (absa > absb) return absa*sqrt(1.0+SQR(absb/absa));
	else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+SQR(absa/absb)));
}



/* (C) Copr. 1986-92 Numerical Recipes Software ):5-). */
