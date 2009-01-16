#ifndef	SCARAB_DATA_TEST_H_
#define SCARAB_DATA_TEST_H_

/*
 *  ScarabTest.h
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

#include "MonkeyWorksCore/ScarabServices.h"
namespace mw {
class ScarabDataTestFixture : public CppUnit::TestFixture {


	CPPUNIT_TEST_SUITE( ScarabDataTestFixture );
	
	CPPUNIT_TEST( testDictPutGet );
	CPPUNIT_TEST( testDictKeys );
	//CPPUNIT_TEST( testTooSmallDictPutGet );
	
	CPPUNIT_TEST_SUITE_END();


	private:
		// NADA

	public:
	
		void setUp(){
		}

		void tearDown(){
		}
					
		void testDictPutGet(){
			fprintf(stderr, "Running ScarabDataTestFixture::testDictPutGet()\n");

			//fprintf(stderr, "Testing...\n"); fflush(stderr);
			#define SCARAB_TEST_DICT_SIZE	10
			ScarabDatum *dict = scarab_dict_new(
												SCARAB_TEST_DICT_SIZE, 
												&scarab_dict_times2);
			
			for(int i = 0; i < SCARAB_TEST_DICT_SIZE; i++){
				ScarabDatum *key = scarab_new_integer(i);
				ScarabDatum *value = scarab_new_integer(2*i + 1);
				
				scarab_dict_put(dict, 
								key, 
								value);
				
				scarab_free_datum(value);
				scarab_free_datum(key);
				
			}
			
			for(int i = 0; i < SCARAB_TEST_DICT_SIZE; i++){
				ScarabDatum *key = scarab_new_integer(i);
				
				ScarabDatum *value = 
							scarab_dict_get(dict, key); 
				scarab_free_datum(key);
				
				CPPUNIT_ASSERT( value != NULL );
				CPPUNIT_ASSERT( value->type == SCARAB_INTEGER );
				CPPUNIT_ASSERT( value->data.integer == 2*i + 1 );
				
			}
			
			scarab_free_datum(dict);
		}
	
	
		void testTooSmallDictPutGet(){
			fprintf(stderr, "Running ScarabDataTestFixture::testTooSmallDictPutGet()\n");
			
			//fprintf(stderr, "Testing...\n"); fflush(stderr);
			#define SCARAB_TEST_DICT_SIZE	10
			ScarabDatum *dict = scarab_dict_new(
												SCARAB_TEST_DICT_SIZE, 
												&scarab_dict_times2);
			
			for(int i = 0; i < SCARAB_TEST_DICT_SIZE * 2; i++){
				
				ScarabDatum *key = scarab_new_integer(i);
				ScarabDatum *value = scarab_new_integer(2*i + 1);
				
				scarab_dict_put(dict, 
								key, 
								value);
				
				scarab_free_datum(value);
				scarab_free_datum(key);
			}
			
			for(int i = 0; i < SCARAB_TEST_DICT_SIZE * 2; i++){
				
				ScarabDatum *key = scarab_new_integer(i);
				
				ScarabDatum *value = 
					scarab_dict_get(dict, key); 
				scarab_free_datum(key);
				
				CPPUNIT_ASSERT( value != NULL );
				CPPUNIT_ASSERT( value->type == SCARAB_INTEGER );
				CPPUNIT_ASSERT( value->data.integer == 2*i + 1 );
				
			}
			
			scarab_free_datum(dict);
		}

	
	
		void testDictKeys(){
			fprintf(stderr, "Running ScarabDataTestFixture::testDictKeys()\n");
			//fprintf(stderr, "Testing...\n"); fflush(stderr);
			#define SCARAB_TEST_DICT_SIZE	10
			ScarabDatum *dict = scarab_dict_new(
												SCARAB_TEST_DICT_SIZE, 
												&scarab_dict_times2);
			
			for(int i = 0; i < SCARAB_TEST_DICT_SIZE; i++){
				
				ScarabDatum *key = scarab_new_integer(i);
				ScarabDatum *value = scarab_new_integer(2*i + 1);
				
				scarab_dict_put(dict, 
								key, 
								value);
				
				scarab_free_datum(value);
				scarab_free_datum(key);
			}
			
			for(int i = 0; i < SCARAB_TEST_DICT_SIZE; i++){
				
				ScarabDatum *key = scarab_new_integer(i);
				
				ScarabDatum *value = 
					scarab_dict_get(dict, key); 
				scarab_free_datum(key);
				
				CPPUNIT_ASSERT( value != NULL );
				CPPUNIT_ASSERT( value->type == SCARAB_INTEGER );
				CPPUNIT_ASSERT( value->data.integer == 2*i + 1 );
				
			}
			
			scarab_free_datum(dict);
		}
		
};

}

#endif

