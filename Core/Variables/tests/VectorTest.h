#ifndef	VECTOR_TEST_H_
#define VECTOR_TEST_H_

/*
 *  VectorTest.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MonkeyWorksCore/GenericData.h"

namespace mw {
class VectorTestFixture : public CppUnit::TestFixture {


	CPPUNIT_TEST_SUITE( VectorTestFixture );
	
	CPPUNIT_TEST( testGetNElements );
	CPPUNIT_TEST( testSetGetElement );
	CPPUNIT_TEST( testBracketOperator );
	
	CPPUNIT_TEST_SUITE_END();


	private:
	
		Data *a,*b,*c,*d;

	public:
	
		void setUp(){
				
			a = new Data(10L);
			b = new Data(20L);
			c = new Data(12.34);
			d = new Data(45.67);
		}

		void tearDown(){
			
			delete a;
			delete b;
			delete c;
			delete d;
		}
					
		void testGetNElements(){
			fprintf(stderr, "Running VectorTestFixture::testGetNElements()\n");
			Data vec(M_LIST, 4);
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
			fprintf(stderr, "Running VectorTestFixture::testSetGetElement()\n");
		
			Data vec(M_LIST, 10);
			
			for(int i = 0; i < 10; i++){
				Data test((long)(2*i+1));
				vec.setElement(i,test);
			}

			for (int i = 0; i < 10; i++){
				Data test2 = vec.getElement(i);
				
				CPPUNIT_ASSERT( (int) test2 ==  2*i+1 );
			}
		}
		
		void testBracketOperator(){
			fprintf(stderr, "Running VectorTestFixture::testBracketOperator()\n");
		
			Data vec(M_LIST, 1);
			vec.setElement(0, *a);
			
			CPPUNIT_ASSERT( (long)(vec[0]) == (long)(vec.getElement(0)) );
			CPPUNIT_ASSERT( (long)(vec[0]) == (long)(*a) );
			
		}
		
		// void testAddition(); // etc. etc...
		
		
};
}


#endif

