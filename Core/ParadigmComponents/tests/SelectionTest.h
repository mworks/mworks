#ifndef	SELECTION_TEST_H_
#define SELECTION_TEST_H_

/*
 *  SelectionTest.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


// THIS TEST IS DEPRECATED BECAUSE OF THE NEW RELATIONSHIP BETWEEN
// SELECTION objects and SELECTABLE objects
//
// Basically, a selection won't "go" anymore unless it has a properly 
// associated selectable.  The tests in SelectionVariable accomplish
// all of the testing that was intended in this file.
#if	0


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MonkeyWorksCore/Selection.h"
#include "MonkeyWorksCore/States.h"


class SelectionTestFixture : public CppUnit::TestFixture {


	CPPUNIT_TEST_SUITE( SelectionTestFixture );
	
	CPPUNIT_TEST( testSequentialAscending );
	CPPUNIT_TEST( testSequentialDescending );
	CPPUNIT_TEST( testSequentialAscendingMultipleCycles );
	
	CPPUNIT_TEST_SUITE_END();


	private:
	
		ListState *a_selectable;
		Selection *test;
		
	public:
	
		void setUp(){
			a_selectable = new ListState(10);
			
		}

		void tearDown(){
			//delete a_selectable;
		}
					
		void testSequentialAscending(){
			fprintf(stderr, "Testing sequential ascending selections...\n");
			fflush(stderr);
		
			test = new SequentialSelection(a_selectable,true);
			test->setN(10);
			test->setSamplingMethod(M_SAMPLES);
					
			for(int i = 0; i < 9; i++){
				CPPUNIT_ASSERT( test->next() == i);
				CPPUNIT_ASSERT( test->advance() == true );
			}
			
			CPPUNIT_ASSERT( test->advance() == false);
			
			CPPUNIT_ASSERT( test->next() == 9 );
			
			test->reset();
			
			CPPUNIT_ASSERT( test->next() == 0 );
			
			CPPUNIT_ASSERT( test->advance() == true );
			//delete test;
		}
	
		void testSequentialAscendingMultipleCycles(){
			fprintf(stderr, 
					"Testing sequential ascending selections 2 cycles...\n");
			fflush(stderr);
			test = new SequentialSelection(a_selectable,true);
			test->setN(20);
			test->setSamplingMethod(M_SAMPLES);
			
			for(int i = 0; i < 9; i++){
				CPPUNIT_ASSERT( test->next() == i);
				CPPUNIT_ASSERT( test->advance() == true );
			}
			
			CPPUNIT_ASSERT( test->advance() == true);
			
			CPPUNIT_ASSERT( test->next() == 0 );
			
			CPPUNIT_ASSERT( test->advance() == true);
			
			CPPUNIT_ASSERT( test->next() == 1 );
			
			test->reset();
			
			CPPUNIT_ASSERT( test->next() == 0 );
			
			CPPUNIT_ASSERT( test->advance() == true );
			//delete test;
		}
		
		void testSequentialDescending(){
			
			test = new SequentialSelection(a_selectable,false);
			test->setN(10);
			test->setSamplingMethod(M_SAMPLES);
			
			for(int i = 9; i >= 0; i--){
				CPPUNIT_ASSERT( test->next() == i );
				test->advance();
			}
			
			CPPUNIT_ASSERT( test->next() == 0 );
			
			test->reset();
			
			CPPUNIT_ASSERT( test->next() == 9 );
			
			//delete test;
		}
};



#endif

#endif
