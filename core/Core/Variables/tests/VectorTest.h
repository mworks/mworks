#ifndef	VECTOR_TEST_H_
#define VECTOR_TEST_H_

/*
 *  VectorTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksCore/GenericData.h"


BEGIN_NAMESPACE_MW


class VectorTestFixture : public CppUnit::TestFixture {


	CPPUNIT_TEST_SUITE( VectorTestFixture );
	
	CPPUNIT_TEST( testGetNElements );
	CPPUNIT_TEST( testSetGetElement );
	CPPUNIT_TEST( testBracketOperator );
	
	CPPUNIT_TEST_SUITE_END();


	private:
	
	 Datum *a,*b,*c,*d;

	public:
	
		void setUp(){
				
			a = new Datum(10L);
			b = new Datum(20L);
			c = new Datum(12.34);
			d = new Datum(45.67);
		}

		void tearDown(){
			
			delete a;
			delete b;
			delete c;
			delete d;
		}
					
		void testGetNElements(){
		 Datum vec(M_LIST, 4);
			CPPUNIT_ASSERT( vec.getMaxElements() == 4 );
			
			CPPUNIT_ASSERT( vec.getNElements() == 0 );
			vec.setElement(0, *a);
			CPPUNIT_ASSERT( vec.getNElements() == 1 );
			vec.setElement(1, *b);
			CPPUNIT_ASSERT( vec.getNElements() == 2 );
			vec.setElement(2, *c);
			CPPUNIT_ASSERT( vec.getNElements() == 3 );
			vec.setElement(3, *d);
			
			CPPUNIT_ASSERT( vec.getNElements() == 4 );
		}
	
		void testSetGetElement(){
		 Datum vec(M_LIST, 10);
			
			for(int i = 0; i < 10; i++){
			 Datum test((long)(2*i+1));
				vec.setElement(i,test);
			}

			for (int i = 0; i < 10; i++){
			 Datum test2 = vec.getElement(i);
				
				CPPUNIT_ASSERT( (int) test2 ==  2*i+1 );
			}
		}
		
		void testBracketOperator(){
		 Datum vec(M_LIST, 1);
			vec.setElement(0, *a);
			
			CPPUNIT_ASSERT( (long)(vec[0]) == (long)(vec.getElement(0)) );
			CPPUNIT_ASSERT( (long)(vec[0]) == (long)(*a) );
			
		}
		
		// void testAddition(); // etc. etc...
		
		
};


END_NAMESPACE_MW


#endif

