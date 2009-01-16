#ifndef	SCARAB_FILE_TEST_H_
#define SCARAB_FILE_TEST_H_

/*
 *  ScarabFileTest.h
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
#include "MonkeyWorksCore/VariableRegistry.h"
namespace mw {
class ScarabFileTestFixture : public CppUnit::TestFixture {


	CPPUNIT_TEST_SUITE( ScarabFileTestFixture );
	
	CPPUNIT_TEST( testIntegerPutGet );
	CPPUNIT_TEST( testBigIntegerPutGet );
	CPPUNIT_TEST( testDictPutGet );
	CPPUNIT_TEST( testNestedDictPutGet );
	CPPUNIT_TEST( testFloatPutGet );
	CPPUNIT_TEST( test128IntegerPutGet );
	CPPUNIT_TEST( test64BitIntegerPutGet );
	//CPPUNIT_TEST( testPutGetCodec );
									
	CPPUNIT_TEST_SUITE_END();


	protected:
	
		ScarabSession *session;

	public:
	
		void setUp(){
		}

		void tearDown(){
		}
			
		void testPutGetCodec(){
			fprintf(stderr, "Running ScarabFileTestFixture::testPutGetCodec()\n");

			/*printf("Testing put/get codec...\n");
			
			VariableRegistry *r = new VariableRegistry();
			Variable *testvar =
					r->addExperimentwideVariable(new VariableProperties(
											new Data((bool)false), "test",
											"Test test",
											"Testy test test test",
											M_NEVER, M_WHEN_CHANGED,
											true, false,
											M_CONTINUOUS_INFINITE));

			r->generateCodec();
			ScarabDatum *generated_codec = r->getCodec();

			// check the GENERATED codec
			CPPUNIT_ASSERT( 
					scarab_dict_number_of_elements(generated_codec) == 3+1 );
			ScarabDatum **gkeys = scarab_dict_keys(generated_codec);
			ScarabDatum **gvalues = scarab_dict_values(generated_codec);
			
			CPPUNIT_ASSERT( gkeys != NULL );
			CPPUNIT_ASSERT( gvalues != NULL );
			
			for(int i= 0; 
				i < scarab_dict_number_of_elements(generated_codec); i++){
			
				CPPUNIT_ASSERT( gkeys[i] != NULL );
				CPPUNIT_ASSERT( gvalues[i] != NULL );
				
				//printf("gkeys[i]->type = %d\n", gkeys[i]->type);
				CPPUNIT_ASSERT( gkeys[i]->type == SCARAB_INTEGER );
				CPPUNIT_ASSERT( gvalues[i]->type == SCARAB_DICT );
			}


			
			ScarabDatum *received_codec = putGet(generated_codec);
		
			// check the codec
			CPPUNIT_ASSERT( received_codec != NULL );
			CPPUNIT_ASSERT( received_codec->type == SCARAB_DICT );
			
			CPPUNIT_ASSERT( 
				scarab_dict_number_of_elements(received_codec) == 3+1 );
			ScarabDatum **keys = scarab_dict_keys(received_codec);
			ScarabDatum **values = scarab_dict_values(received_codec);
			
			CPPUNIT_ASSERT( keys != NULL );
			CPPUNIT_ASSERT( values != NULL );
			
			for(int i= 0; 
				i < scarab_dict_number_of_elements(received_codec); i++){
			
				CPPUNIT_ASSERT( keys[i] != NULL );
				CPPUNIT_ASSERT( values[i] != NULL );
				
				//printf("key[i]->type = %d\n", keys[i]->type);
				CPPUNIT_ASSERT( keys[i]->type == SCARAB_INTEGER );
				CPPUNIT_ASSERT( values[i]->type == SCARAB_DICT );
			}
			
			delete r;
			delete testvar;*/
		}
		
		
		void testIntegerPutGet(){
			fprintf(stderr, "Running ScarabFileTestFixture::testIntegerPutGet()\n");
			
			ScarabDatum *theint = scarab_new_integer(42);
			
			// put and get to a file
			ScarabDatum *received_int = putGet(theint);
			
			CPPUNIT_ASSERT( received_int != NULL );
			CPPUNIT_ASSERT( received_int->type == SCARAB_INTEGER );
			CPPUNIT_ASSERT( received_int->data.integer == 42 );
			
		}

		void test128IntegerPutGet(){
			fprintf(stderr, "Running ScarabFileTestFixture::test128IntegerPutGet()\n");
			ScarabDatum *theint = scarab_new_integer(128);
			
			// put and get to a file
			ScarabDatum *received_int = putGet(theint);
			
			CPPUNIT_ASSERT( received_int != NULL );
			CPPUNIT_ASSERT( received_int->type == SCARAB_INTEGER );
			CPPUNIT_ASSERT( received_int->data.integer == 128 );
		}

		void testBigIntegerPutGet(){
		  fprintf(stderr, "Running ScarabFileTestFixture::testBigIntegerPutGet()\n");

			printf("Testing put/get large integer...\n");
			ScarabDatum *theint = scarab_new_integer(100000);
			
			// put and get to a file
			ScarabDatum *received_int = putGet(theint);
			
			CPPUNIT_ASSERT( received_int != NULL );
			CPPUNIT_ASSERT( received_int->type == SCARAB_INTEGER );
			CPPUNIT_ASSERT( received_int->data.integer == 100000 );
			
		}

		void test64BitIntegerPutGet(){
		  fprintf(stderr, "Running ScarabFileTestFixture::test64BitIntegerPutGet()\n");
			
			printf("Testing put/get 64 bit integer...\n");
			ScarabDatum *theint = scarab_new_integer(((long long)0x01 << 31));
			
			CPPUNIT_ASSERT( theint->data.integer == ((long long)0x01 << 31) );
			
			// put and get to a file
			ScarabDatum *received_int = putGet(theint);
			
			CPPUNIT_ASSERT( received_int != NULL );
			CPPUNIT_ASSERT( received_int->type == SCARAB_INTEGER );
			fprintf(stderr, "Received: %lld", received_int->data.integer);
			fflush(stderr);
			CPPUNIT_ASSERT( received_int->data.integer == ((long long)0x01 << 31) );
			
		}

		void testFloatPutGet(){
		  fprintf(stderr, "Running ScarabFileTestFixture::testFloatPutGet()\n");
			
			printf("Testing put/get float...\n");
			
			// TODO: There is a long-standing hack that needs correcting here...
			// (sorry that this unit test just grandfathers it in...)
			
			ScarabDatum *thefloat = scarab_new_float(3.14159);
			
			// put and get to a file
			ScarabDatum *received_float = putGet(thefloat);
			
			CPPUNIT_ASSERT( received_float != NULL );
			CPPUNIT_ASSERT( received_float->type == SCARAB_FLOAT_OPAQUE );
			
			#if	__LITTLE_ENDIAN__
				double float_hack = *((double *)(received_float->data.opaque.data));
			#else
			
				char swap_bytes[sizeof(double)];
				char *double_bytes = (char *)(received_float->data.opaque.data);
				for(int i = 0; i < sizeof(double); i++){
					swap_bytes[i] = double_bytes[sizeof(double) - i];
				}
		
				double float_hack = *((double *)swap_bytes);
			#endif
			
			CPPUNIT_ASSERT( float_hack == 3.14159 );
			
		}



		void testDictPutGet(){
		  fprintf(stderr, "Running ScarabFileTestFixture::testDictPutGet()\n");
			
			printf("Testing put/get dictionary...\n");
			
			ScarabDatum *dict = scarab_dict_new(10,&scarab_dict_times2);
			for(int i = 0; i < 10; i++){
				scarab_dict_put(dict, scarab_new_integer(i),
									  scarab_new_integer(2*i+1));
			}
			
			
			// put and get to a file
			ScarabDatum *received_dict = putGet(dict);
			
			CPPUNIT_ASSERT( received_dict != NULL );
			CPPUNIT_ASSERT( received_dict->type == SCARAB_DICT );

			for(int i = 0; i < 10; i++){
				ScarabDatum *value = scarab_dict_get(dict, 
											scarab_new_integer(i));
											
				CPPUNIT_ASSERT( value->type == SCARAB_INTEGER );
				CPPUNIT_ASSERT( value->data.integer == 2*i+1 );
			}
		}


		void testNestedDictPutGet(){
		  fprintf(stderr, "Running ScarabFileTestFixture::testNestedDictPutGet()\n");
			
			printf("Testing put/get nested dictionaries...\n");
			
			ScarabDatum *dict = scarab_dict_new(10,&scarab_dict_times2);
			for(int i = 0; i < 10; i++){
				ScarabDatum *subdict = 
								scarab_dict_new(10,&scarab_dict_times2);
				
				for(int j = 0; j < 10; j++){
					scarab_dict_put(subdict, scarab_new_integer(j),
									  scarab_new_integer(2*j+1));
				}
				
				scarab_dict_put(dict, scarab_new_integer(i),
									  subdict);
			}
			
			
			// put and get to a file
			ScarabDatum *received_dict = putGet(dict);
			
			CPPUNIT_ASSERT( received_dict != NULL );
			CPPUNIT_ASSERT( received_dict->type == SCARAB_DICT );
			CPPUNIT_ASSERT(scarab_dict_number_of_elements(received_dict) == 10);

			for(int i = 0; i < 10; i++){
				ScarabDatum *value = scarab_dict_get(dict, 
											scarab_new_integer(i));
									
				CPPUNIT_ASSERT( value->type == SCARAB_DICT );
				CPPUNIT_ASSERT(
								scarab_dict_number_of_elements(value) == 10);
				for(int j = 0; j < 10; j++){
					ScarabDatum *subval = scarab_dict_get(value,
												scarab_new_integer(j));
					
					CPPUNIT_ASSERT( subval != NULL );
					CPPUNIT_ASSERT( subval->type == SCARAB_INTEGER );
					CPPUNIT_ASSERT( subval->data.integer == 2*j+1 );
				}
			}
		}

	
		virtual ScarabDatum *putGet(ScarabDatum *in){
			#define TEST_FILE	"scarab_file_test.tst"
			#define TEST_URI	"ldobinary:file://scarab_file_test.tst"
			
			scarab_create_file(TEST_FILE);
	

			// Connect for writing...
			session = scarab_session_connect(TEST_URI);
			
			CPPUNIT_ASSERT(  scarab_write(session, in) == 0  );
			
			scarab_session_close(session);
			
			
			// Connect for reading
			session = scarab_session_connect(TEST_URI);
			ScarabDatum *out;
			
			out = scarab_read(session);
			scarab_session_close(session);		
						
			return out;
		}
		
};

}

#endif

