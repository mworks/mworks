/*
 *  BoostMathExampleTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy 2006
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "BoostMathExampleTest.h"

// for random number generation
#include "boost/random.hpp"

// for linear algebra
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>

// for trig
#include <cmath>

using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( BoostMathExampleTestFixture, "Unit Test" );
//CPPUNIT_TEST_SUITE_REGISTRATION( BoostMathExampleTestFixture );


void BoostMathExampleTestFixture::setUp() {	
}

void BoostMathExampleTestFixture::tearDown() {
}

void BoostMathExampleTestFixture::example1Test() {
	fprintf(stderr, "Running BoostMathExampleTestFixture::example1Test()\n"); 

	using namespace boost;

	int mean = 0;
	int sigma = 1;
	
	// for random numbers, you need both a random number generator and a 
	// random distribution

	
	mt19937 rng;  // mt19937 is a general purpose psuedo-random number 
				  // generator

	normal_distribution<double> nd(mean, sigma); // normal distribution
											  // is just as advertised
	
	// these should match what is already created
	variate_generator<mt19937&, normal_distribution<double> > sampler(rng,nd);
	//                ^         ^-- this is the distribution to use
	//                ^-- this is the random number genreator
	
	double x = sampler(); // this gets a random value within the distribution
	
	for(int i=0; i<100; ++i) {
		x = sampler();
		//fprintf(stderr, "sampled value: %f\n", x);
	}
}


// here's an additional example using a poisson distribution and slightly
// different syntax.  I haven't decided what one I like better.  I think
// the "using namespace boost" is easier to read.
void BoostMathExampleTestFixture::example2Test() {	
	fprintf(stderr, "Running BoostMathExampleTestFixture::example2Test()\n"); 
	double lambda = 1;
	
	boost::mt19937 rng;  // we'll still use mt19937
	
	boost::poisson_distribution<int> pd(lambda); 
	
	boost::variate_generator<boost::mt19937&, 
		boost::poisson_distribution<int> > sampler(rng,pd);
	
	int x = sampler();
	
	for(int i=0; i<10000; ++i) {
		x = sampler();
		//fprintf(stderr, "sampled value: %d\n", x);
	}
}

// lin-alg example
void BoostMathExampleTestFixture::example3Test() {	
	fprintf(stderr, "Running BoostMathExampleTestFixture::example3Test()\n"); 
	using namespace boost::numeric::ublas;
	
	vector<double> x (2);
	x(0) = 1; x(1) = 2;
	
	matrix<double> A(2,2);
	A(0,0) = 0; A(0,1) = 1;
	A(1,0) = 2; A(1,1) = 3;
	
	vector<double> y = prod(A, x);

	CPPUNIT_ASSERT(y(0) == 2);
	CPPUNIT_ASSERT(y(1) == 8);
}

// trig example
void BoostMathExampleTestFixture::example4Test() {	
	fprintf(stderr, "Running BoostMathExampleTestFixture::example4Test()\n"); 
	// this isn't really Boost but I included it anyway

//	const double pi = 3.141592653589793238462643383279502884197169399375105820;
//	const double angle = 45;
//	double angle_in_rad = 45*(pi/180);
//
//  // precision errors prevert these tests from passing, but you get the point
//	CPPUNIT_ASSERT(cos(pi) == 1);
//	CPPUNIT_ASSERT(sin(angle_in_rad) == sqrt(2));
}

