#ifndef	GENERIC_DATA_TEST_H_
#define GENERIC_DATA_TEST_H_

/*
 *  GenericDataTest.h
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy 07142006
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sstream>
#include "MonkeyWorksCore/GenericData.h"

namespace mw {
using namespace std;

void *hammerit(void *thedatum);
void *hammerlist(void *thedatum);
void *hammerdict(void *thedatum);

class GenericDataTestFixture : public CppUnit::TestFixture {
  
  
	CPPUNIT_TEST_SUITE( GenericDataTestFixture );
	
	CPPUNIT_TEST( testList );
	CPPUNIT_TEST( testString );
	CPPUNIT_TEST( testDictionary );
	CPPUNIT_TEST( testEmptyDictionary );
	CPPUNIT_TEST( testDictLeakyness );
	CPPUNIT_TEST( testListLeakyness );
	CPPUNIT_TEST( testListOverwriteLeakyness );
	CPPUNIT_TEST( testDictOverwriteLeakyness );
	CPPUNIT_TEST( testDictionaryStrings );
	CPPUNIT_TEST( testDictionaryAddGetElements );
	CPPUNIT_TEST( testDictionaryKey );
	CPPUNIT_TEST( testDoubleTeamOnADictionary );
	CPPUNIT_TEST( testDataEqual );
	CPPUNIT_TEST( testListUnderADictionary );
	CPPUNIT_TEST( testAnyScalarConstructorBool );
	CPPUNIT_TEST( testAnyScalarConstructorInt );
	CPPUNIT_TEST( testAnyScalarConstructorLong );
	CPPUNIT_TEST( testAnyScalarConstructorByte );
	CPPUNIT_TEST( testAnyScalarConstructorShort );
	CPPUNIT_TEST( testAnyScalarConstructorWord );
	CPPUNIT_TEST( testAnyScalarConstructorDWord );
	CPPUNIT_TEST( testAnyScalarConstructorQWord );
	CPPUNIT_TEST( testAnyScalarConstructorFloat );
	CPPUNIT_TEST( testAnyScalarConstructorDouble );
	CPPUNIT_TEST( testAnyScalarConstructorString );
	CPPUNIT_TEST( testAnyScalarConstructorChar );
	CPPUNIT_TEST( testAnyScalarConstructorInvalid );
	CPPUNIT_TEST( testAnyScalarEqualBool );
	CPPUNIT_TEST( testAnyScalarEqualInt );
	CPPUNIT_TEST( testAnyScalarEqualLong );
	CPPUNIT_TEST( testAnyScalarEqualByte );
	CPPUNIT_TEST( testAnyScalarEqualShort );
	CPPUNIT_TEST( testAnyScalarEqualWord );
	CPPUNIT_TEST( testAnyScalarEqualDWord );
	CPPUNIT_TEST( testAnyScalarEqualQWord );
	CPPUNIT_TEST( testAnyScalarEqualFloat );
	CPPUNIT_TEST( testAnyScalarEqualDouble );
	CPPUNIT_TEST( testAnyScalarEqualString );
	CPPUNIT_TEST( testAnyScalarEqualChar );
	CPPUNIT_TEST( testAnyScalarEqualInvalid );
	CPPUNIT_TEST( testIs );
		
	//	CPPUNIT_TEST( testMemoryManagement );
	
	CPPUNIT_TEST_SUITE_END();
	
	
 private:
	
 public:
	
	void setUp(){
	}
	
	void tearDown(){
	}
	
	#define LOTS	1000000

//	void testMemoryManagement(){
//	
//		fprintf(stderr, "Testing Data memory management...\n");
//		fflush(stderr);
//		
//		Data test(M_LIST, 5);
//		
//		Data *data = new Data[5];
//		for(int i = 0; i < 5; i++){
//			data[i] = (long)i;
//		}
//		
//		for(int i = 0; i < 5; i++){
//			test.setElement(i, data[i]);
//		}
//		
//		for(int i = 0; i < 5; i++){
//			CPPUNIT_ASSERT(data[i] == test.getElement(i));
//		}
//		
//		
//		delete [] data;
//		
//		
//		for(int i = 0; i < 5; i++){
//			CPPUNIT_ASSERT((long)(test.getElement(i)) == i);
//		}
//		
//		
//		
//		fprintf(stderr, "Hammering an Data object across two threads...");
//		fflush(stderr);
//		Data datum(0L);
//		
//		pthread_t thread;
//		pthread_create(&thread, NULL, hammerit, (void *)(&datum));
//		
//		for(int i = 0; i < LOTS; i++){
//			if(i % (LOTS / 100) == 0){
//				fprintf(stderr, "."); fflush(stderr);
//			}
//			datum = (long)i;
//		}
//		fprintf(stderr, "\n"); fflush(stderr);
//		
//		ScarabDatum *scarabdatum = datum.getScarabDatum();
//		CPPUNIT_ASSERT(scarabdatum != NULL);
//		CPPUNIT_ASSERT(scarabdatum->type == SCARAB_INTEGER);
//	
//	
//		fprintf(stderr, "Hammering an M_LIST Data object across two threads...");
//		fflush(stderr);
//		Data list_datum(M_LIST, 1);
//		
//		pthread_t thread2;
//		pthread_create(&thread2, NULL, hammerlist, (void *)(&list_datum));
//		
//		for(int i = 0; i < LOTS; i++){
//			if(i % (LOTS / 100) == 0){
//				fprintf(stderr, "."); fflush(stderr);
//			}
//			list_datum.setElement(0, Data((long)i));
//		}
//		fprintf(stderr, "\n"); fflush(stderr);
//		
//		scarabdatum = list_datum.getScarabDatum();
//		CPPUNIT_ASSERT(scarabdatum != NULL);
//		CPPUNIT_ASSERT(scarabdatum->type == SCARAB_LIST);
//	
//		fprintf(stderr, "Hammering an M_DICT Data object across two threads...");
//		fflush(stderr);
//		Data dict_datum(M_DICTIONARY, 1);
//		
//		pthread_t thread3;
//		pthread_create(&thread3, NULL, hammerdict, (void *)(&dict_datum));
//		
//		for(int i = 0; i < LOTS; i++){
//			if(i % (LOTS / 100) == 0){
//				fprintf(stderr, "."); fflush(stderr);
//			}
//			dict_datum.addElement("test", Data((long)i));
//		}
//		fprintf(stderr, "\n"); fflush(stderr);
//		
//		scarabdatum = dict_datum.getScarabDatum();
//		CPPUNIT_ASSERT(scarabdatum != NULL);
//		CPPUNIT_ASSERT(scarabdatum->type == SCARAB_DICT);
//	
//	}

		

	void testString() {
		fprintf(stderr, "Running GenericDataTestFixture::testString()\n");
		Data data("Test string");
		char * test = "Test string";
		
		for(unsigned int i = 0; i<strlen(test); i++)
		{
			const char * result = data.getString();
			CPPUNIT_ASSERT(result[i] == test[i]);
		}
		
		Data data2;
		char * test2 = "Test second string";
		char * test2a = "Test second string";
		
		CPPUNIT_ASSERT(strcmp(test2, test2a) == 0);
		
		data2 = Data(test2);
		
		CPPUNIT_ASSERT(data2 == test2a);
		CPPUNIT_ASSERT(!(data2 != test2a));
		
		for(unsigned int i = 0; i<strlen(test2a); i++)
		{
			const char * result2 = data2.getString();
			CPPUNIT_ASSERT(result2[i] == test2a[i]);
		}
		
		
		data2 = "Test third string";
		char * test3a = "Test third string";
		char * test3b = "Test 3rd string";
		
		CPPUNIT_ASSERT(data2 == test3a);
		CPPUNIT_ASSERT(data2 != test3b);
		
		
		Data data4a = "Test 4th string";
		Data data4b = "Test fourth string";
		
		Data boolean = (data4a != data4b);
		CPPUNIT_ASSERT(boolean.getBool() == true);
		
		boolean = (data4b != data4a);
		CPPUNIT_ASSERT(boolean.getBool() == true);
		
		boolean = (data4a == data4b);
		CPPUNIT_ASSERT(boolean.getBool() == false);
		
		boolean = (data4b == data4a);
		CPPUNIT_ASSERT(boolean.getBool() == false);
		
		data4b = "Test 4th string";
		
		boolean = (data4a == data4b);		
		CPPUNIT_ASSERT(boolean.getBool() == true);		
		
		boolean = (data4b == data4a);		
		CPPUNIT_ASSERT(boolean.getBool() == true);		
		
		boolean = (data4a != data4b);
		CPPUNIT_ASSERT(boolean.getBool() == false);
		
		boolean = (data4b != data4a);
		CPPUNIT_ASSERT(boolean.getBool() == false);
		
	}

	void testList() {
		fprintf(stderr, "Running GenericDataTestFixture::testList()\n");
		Data undefined;
		Data list1(M_LIST, 1);
		Data testInt1(M_INTEGER, 1);		
		Data testInt2(M_INTEGER, 2);
		Data testInt3(M_INTEGER, 3);
		
		CPPUNIT_ASSERT(undefined != list1);
		CPPUNIT_ASSERT(undefined == list1.getElement(0));
		CPPUNIT_ASSERT(undefined == list1.getElement(1));
		
		CPPUNIT_ASSERT(list1.getNElements() == 0);
		CPPUNIT_ASSERT(list1.getMaxElements() == 1);
		
		Data list2(M_LIST, 1);
		
		CPPUNIT_ASSERT(list1 == list2);
		
		list1.setElement(0, testInt1);
		
		CPPUNIT_ASSERT(list1.getNElements() == 1);
		CPPUNIT_ASSERT(list1.getMaxElements() == 1);
		CPPUNIT_ASSERT(undefined != list1.getElement(0));
		CPPUNIT_ASSERT(undefined == list1.getElement(1));

		CPPUNIT_ASSERT(list1 != list2);
		
		list2.setElement(0, testInt2);
		
		CPPUNIT_ASSERT(list1 != list2);

		CPPUNIT_ASSERT(list1.getElement(0) == testInt1);
		CPPUNIT_ASSERT(list1.getElement(0) != testInt2);
		CPPUNIT_ASSERT(list2.getElement(0) != testInt1);
		CPPUNIT_ASSERT(list2.getElement(0) == testInt2);
				
		list2.setElement(0, testInt1);
		CPPUNIT_ASSERT(list1 == list2);
		
		CPPUNIT_ASSERT(list1.getElement(0) == testInt1);
		CPPUNIT_ASSERT(list1.getElement(0) != testInt2);
		CPPUNIT_ASSERT(list2.getElement(0) == testInt1);
		CPPUNIT_ASSERT(list2.getElement(0) != testInt2);
		
		
		Data list3(M_LIST, 1);
		list3.setElement(0, testInt1);
		list3.setElement(1, testInt2);
		
		CPPUNIT_ASSERT(list3.getNElements() == 2);
		CPPUNIT_ASSERT(list3.getMaxElements() == 2);
		CPPUNIT_ASSERT(list3.getElement(0) == testInt1);
		CPPUNIT_ASSERT(list3.getElement(1) == testInt2);

		CPPUNIT_ASSERT(list3.getElement(1) != testInt1);
		CPPUNIT_ASSERT(list3.getElement(0) != testInt2);

		Data list4(M_LIST, 2);
		list4.setElement(0, testInt1);
		list4.setElement(1, testInt2);
		list4.addElement(testInt3);

		CPPUNIT_ASSERT(list4.getNElements() == 3);
		CPPUNIT_ASSERT(list4.getMaxElements() == 3);
		CPPUNIT_ASSERT(list4.getElement(0) == testInt1);
		CPPUNIT_ASSERT(list4.getElement(1) == testInt2);
		CPPUNIT_ASSERT(list4.getElement(2) == testInt3);

		Data list5(M_LIST, 1);
		list5.setElement(0, testInt1);
		list5.setElement(10, testInt2);
		
		CPPUNIT_ASSERT(list5.getNElements() == 2);
		CPPUNIT_ASSERT(list5.getMaxElements() == 11);
		CPPUNIT_ASSERT(list5.getElement(0) == testInt1);
		CPPUNIT_ASSERT(list5.getElement(10) == testInt2);
		
		for(int i=1; i<10; ++i) {
			CPPUNIT_ASSERT(list5.getElement(i) == undefined);
		}

		Data list6(M_LIST, 2);
		list6.setElement(0, testInt1);
		list6.setElement(10, testInt2);		
		
		CPPUNIT_ASSERT(list6 == list5);
		
		Data list7(M_LIST, 2);
		list7.setElement(1, testInt1);
		list7.addElement(testInt2);
		CPPUNIT_ASSERT(list7.getNElements() == 2);
		CPPUNIT_ASSERT(list7.getMaxElements() == 3);
		CPPUNIT_ASSERT(list7.getElement(0) == undefined);
		CPPUNIT_ASSERT(list7.getElement(1) == testInt1);
		CPPUNIT_ASSERT(list7.getElement(2) == testInt2);
		
		Data list8(M_LIST, 3);
		list8.setElement(1, testInt1);
		list8.addElement(testInt2);
		CPPUNIT_ASSERT(list8.getNElements() == 2);
		CPPUNIT_ASSERT(list8.getMaxElements() == 3);
		CPPUNIT_ASSERT(list8.getElement(0) == undefined);
		CPPUNIT_ASSERT(list8.getElement(1) == testInt1);
		CPPUNIT_ASSERT(list8.getElement(2) == testInt2);
		
		Data list9(M_LIST, 12);
		list9.setElement(1, testInt1);
		list9.addElement(testInt2);
		CPPUNIT_ASSERT(list9.getNElements() == 2);
		CPPUNIT_ASSERT(list9.getMaxElements() == 12);
		CPPUNIT_ASSERT(list9.getElement(0) == undefined);
		CPPUNIT_ASSERT(list9.getElement(1) == testInt1);
		CPPUNIT_ASSERT(list9.getElement(2) == testInt2);
		
		Data list10(M_LIST, 1);
		list10.addElement(testInt1);
		CPPUNIT_ASSERT(list10.getNElements() == 1);
		CPPUNIT_ASSERT(list10.getMaxElements() == 1);
		CPPUNIT_ASSERT(list10.getElement(0) == testInt1);

		Data list11(M_LIST, 1);
		list11.setElement(0, testInt1);
		list11.addElement(testInt2);
		CPPUNIT_ASSERT(list11.getNElements() == 2);
		CPPUNIT_ASSERT(list11.getMaxElements() == 2);
		CPPUNIT_ASSERT(list11.getElement(0) == testInt1);
		CPPUNIT_ASSERT(list11.getElement(1) == testInt2);
	}
	
	void testListLeakyness() {
		fprintf(stderr, "Running GenericDataTestFixture::testListLeakyness()\n");

		// testing leakyness for simple lists
		const int num_elem = 8;
		Data list1(M_LIST, num_elem);
		ScarabDatum *list1Datum = list1.getScarabDatum();
		
		for(int i = 0; i<num_elem; i++) {
			Data data((long)i);
			ScarabDatum *dataDatum = data.getScarabDatum();
			CPPUNIT_ASSERT(dataDatum->ref_count == 1);
			
			CPPUNIT_ASSERT(list1Datum->ref_count == 1);						
			list1.addElement(data);
			CPPUNIT_ASSERT(list1Datum->ref_count == 1);			
			CPPUNIT_ASSERT(dataDatum->ref_count == 2);
		}
		
		CPPUNIT_ASSERT(list1Datum->ref_count == 1);			
		
		Data list2(M_LIST, 1);
		
		for(int i = 0; i<num_elem; i++) {
			Data data((long)i);
			ScarabDatum *dataDatum = data.getScarabDatum();
			CPPUNIT_ASSERT(dataDatum->ref_count == 1);
			
			ScarabDatum *list2Datum = list2.getScarabDatum();
			CPPUNIT_ASSERT(list2Datum->ref_count == 1);						
			list2.addElement(data);
			list2Datum = list2.getScarabDatum();
			CPPUNIT_ASSERT(list2Datum->ref_count == 1);			
			CPPUNIT_ASSERT(dataDatum->ref_count == 2);
		}
	}
	
	void testListOverwriteLeakyness() {
		fprintf(stderr, "Running GenericDataTestFixture::testListOverwriteLeakyness()\n");

		Data list1(M_LIST, 2);
		ScarabDatum *list1Datum = list1.getScarabDatum();
		
		ScarabDatum *long1Datum;
		ScarabDatum *long2Datum;
		{
			Data long1((long)1);
			Data long2((long)2);
			
			long1Datum = long1.getScarabDatum();
			long2Datum = long2.getScarabDatum();
			CPPUNIT_ASSERT(long1Datum->ref_count == 1);
			CPPUNIT_ASSERT(long2Datum->ref_count == 1);
			
			list1.setElement(0, long1);
			list1.setElement(1, long2);
			
			CPPUNIT_ASSERT(long1Datum->ref_count == 2);
			CPPUNIT_ASSERT(long2Datum->ref_count == 2);
		}
		CPPUNIT_ASSERT(long1Datum->ref_count == 1);
		CPPUNIT_ASSERT(long2Datum->ref_count == 1);
		
		ScarabDatum *long3Datum;
		{
			Data long3((long)3);
			long3Datum = long3.getScarabDatum();
			
			CPPUNIT_ASSERT(long1Datum->ref_count == 1);
			CPPUNIT_ASSERT(long2Datum->ref_count == 1);
			CPPUNIT_ASSERT(long3Datum->ref_count == 1);
			
			Data long1Copy(long1Datum);
			CPPUNIT_ASSERT(long1Datum->ref_count == 2);
			
			list1.setElement(0, long3);
			
			CPPUNIT_ASSERT(long1Datum->ref_count == 1);
			CPPUNIT_ASSERT(long2Datum->ref_count == 1);
			CPPUNIT_ASSERT(long3Datum->ref_count == 2);
		}
		
		CPPUNIT_ASSERT(list1Datum->ref_count == 1);
		CPPUNIT_ASSERT(long2Datum->ref_count == 1);
		CPPUNIT_ASSERT(long3Datum->ref_count == 1);
		
	}
	
	void testDictOverwriteLeakyness() {
		fprintf(stderr, "Running GenericDataTestFixture::testDictOverwriteLeakyness()\n");

		Data dic1(M_DICTIONARY, 2);
		ScarabDatum *dic1Datum = dic1.getScarabDatum();
		
		ScarabDatum *long1Datum;
		ScarabDatum *long2Datum;
		{
			Data long1((long)1);
			Data long2((long)2);
			
			long1Datum = long1.getScarabDatum();
			long2Datum = long2.getScarabDatum();
			CPPUNIT_ASSERT(long1Datum->ref_count == 1);
			CPPUNIT_ASSERT(long2Datum->ref_count == 1);
			
			dic1.addElement("long1", long1);
			dic1.addElement("long2", long2);
			
			CPPUNIT_ASSERT(long1Datum->ref_count == 2);
			CPPUNIT_ASSERT(long2Datum->ref_count == 2);
		}
		CPPUNIT_ASSERT(long1Datum->ref_count == 1);
		CPPUNIT_ASSERT(long2Datum->ref_count == 1);
		
		ScarabDatum *long3Datum;
		{
			Data long3((long)3);
			long3Datum = long3.getScarabDatum();
			
			CPPUNIT_ASSERT(long1Datum->ref_count == 1);
			CPPUNIT_ASSERT(long2Datum->ref_count == 1);
			CPPUNIT_ASSERT(long3Datum->ref_count == 1);
			
			Data long1Copy(long1Datum);
			CPPUNIT_ASSERT(long1Datum->ref_count == 2);
			
			dic1.addElement("long1", long3);
			
			CPPUNIT_ASSERT(long1Datum->ref_count == 1);
			CPPUNIT_ASSERT(long2Datum->ref_count == 1);
			CPPUNIT_ASSERT(long3Datum->ref_count == 2);
		}
		
		CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
		CPPUNIT_ASSERT(long2Datum->ref_count == 1);
		CPPUNIT_ASSERT(long3Datum->ref_count == 1);
		
	}
	
	
	void testListUnderADictionary() {
		fprintf(stderr, "Running GenericDataTestFixture::testListUnderADictionary()\n");

		ScarabDatum *dic1Datum;
		{
			Data dic1(M_DICTIONARY, 2);
			dic1Datum = dic1.getScarabDatum();
			CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
		
			ScarabDatum *list1Datum;
			ScarabDatum *list2Datum;
			ScarabDatum *int1Datum;
			ScarabDatum *int2Datum;
			ScarabDatum *int3Datum;
			ScarabDatum *int4Datum;

			{
				Data list1(M_LIST, 2);
				Data list2(M_LIST, 2);
				list1Datum = list1.getScarabDatum();
				list2Datum = list2.getScarabDatum();
				CPPUNIT_ASSERT(list1Datum->ref_count == 1);
				CPPUNIT_ASSERT(list2Datum->ref_count == 1);
				CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
			
		
				Data int1((long)1);
				Data int2((long)2);
				Data int3((long)3);
				Data int4((long)4);
		
				int1Datum = int1.getScarabDatum();
				int2Datum = int2.getScarabDatum();
				int3Datum = int3.getScarabDatum();
				int4Datum = int4.getScarabDatum();
				CPPUNIT_ASSERT(list1Datum->ref_count == 1);
				CPPUNIT_ASSERT(list2Datum->ref_count == 1);
				CPPUNIT_ASSERT(int1Datum->ref_count == 1);
				CPPUNIT_ASSERT(int2Datum->ref_count == 1);
				CPPUNIT_ASSERT(int3Datum->ref_count == 1);
				CPPUNIT_ASSERT(int4Datum->ref_count == 1);
				CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
		
				list1.setElement(0, int1);
				list1.setElement(1, int2);
				list2.setElement(0, int3);
				list2.setElement(1, int4);
				CPPUNIT_ASSERT(list1Datum->ref_count == 1);
				CPPUNIT_ASSERT(list2Datum->ref_count == 1);
				CPPUNIT_ASSERT(int1Datum->ref_count == 2);
				CPPUNIT_ASSERT(int2Datum->ref_count == 2);
				CPPUNIT_ASSERT(int3Datum->ref_count == 2);
				CPPUNIT_ASSERT(int4Datum->ref_count == 2);
				CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
			
				dic1.addElement("list1", list1);
				dic1.addElement("list2", list2);
				CPPUNIT_ASSERT(list1Datum->ref_count == 2);
				CPPUNIT_ASSERT(list2Datum->ref_count == 2);
				CPPUNIT_ASSERT(int1Datum->ref_count == 2);
				CPPUNIT_ASSERT(int2Datum->ref_count == 2);
				CPPUNIT_ASSERT(int3Datum->ref_count == 2);
				CPPUNIT_ASSERT(int4Datum->ref_count == 2);
				CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
			}
			CPPUNIT_ASSERT(list1Datum->ref_count == 1);
			CPPUNIT_ASSERT(list2Datum->ref_count == 1);
			CPPUNIT_ASSERT(int1Datum->ref_count == 1);
			CPPUNIT_ASSERT(int2Datum->ref_count == 1);
			CPPUNIT_ASSERT(int3Datum->ref_count == 1);
			CPPUNIT_ASSERT(int4Datum->ref_count == 1);
			CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
		}
	
		// OK that worked, lets reproduce what Jim had:
		ScarabDatum *dic2Datum;
		Data dic2a;
		const int BIG_NUMBER=20000;
		ScarabDatum *listElementArray[BIG_NUMBER];	
			{	
			Data dic2;
			dic2 = Data(M_DICTIONARY, 5);
			dic2Datum = dic2.getScarabDatum();
			CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
		
			double *doubleVector = new double [BIG_NUMBER];       
			for(int i=0; i<BIG_NUMBER; ++i) {
				doubleVector[i] = (double)i/10;
			}
			
			MonkeyWorksTime time = 42;
		
			// look here
			dic2.addElement("six",(long)6);    
			dic2.addElement("MWT",(Data)time);  

			
			ScarabDatum *key;

			key = scarab_new_string("six");
			ScarabDatum *six = scarab_dict_get(dic2Datum, key);
			scarab_free_datum(key);
			fprintf(stderr, "refcount = %d", six->ref_count);fflush(stderr);
			CPPUNIT_ASSERT(six->ref_count == 1);
			CPPUNIT_ASSERT(dic2Datum->ref_count == 1);

			key = scarab_new_string("MWT");
			ScarabDatum *MWT = scarab_dict_get(dic2Datum, key);
			scarab_free_datum(key);
			
			CPPUNIT_ASSERT(six->ref_count == 1);
			CPPUNIT_ASSERT(MWT->ref_count == 1);
			CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
			
			ScarabDatum *list3Datum;
			Data list3(M_LIST, BIG_NUMBER);
			list3Datum = list3.getScarabDatum();
			CPPUNIT_ASSERT(list3Datum->ref_count == 1);
			

			
			for(int i=0; i<BIG_NUMBER; ++i) {
				ScarabDatum *listElement;
				list3.setElement(i,doubleVector[i]);
				
				CPPUNIT_ASSERT(list3Datum->ref_count == 1);
				listElement = scarab_list_get(list3Datum, i);
				listElementArray[i] = scarab_list_get(list3Datum, i);
				CPPUNIT_ASSERT(list3Datum->ref_count == 1);
				CPPUNIT_ASSERT(listElement->ref_count == 1);
			}
			
			for (int i=0; i<BIG_NUMBER; ++i) {
				CPPUNIT_ASSERT(list3Datum->ref_count == 1);
				CPPUNIT_ASSERT(listElementArray[i]->ref_count == 1);
			}
			
			dic2.addElement("The list", list3);

			CPPUNIT_ASSERT(list3Datum->ref_count == 2);
			for (int i=0; i<BIG_NUMBER; ++i) {
				CPPUNIT_ASSERT(listElementArray[i]->ref_count == 1);
			}
			
			CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
			
			delete [] doubleVector;
			
			
			dic2a = dic2;
			ScarabDatum *dic2bDatum = dic2a.getScarabDatum();
			CPPUNIT_ASSERT(dic2bDatum->ref_count == 2);
			
			CPPUNIT_ASSERT(list3Datum->ref_count == 2);
			for (int i=0; i<BIG_NUMBER; ++i) {
				CPPUNIT_ASSERT(listElementArray[i]->ref_count == 1);
			}
			
			dic2bDatum->ref_count = BIG_NUMBER;
		}
		
		ScarabDatum *dic2aDatum = dic2a.getScarabDatum();

		CPPUNIT_ASSERT(dic2aDatum->ref_count == BIG_NUMBER-1);
		ScarabDatum *key2 = scarab_new_string("The list");
		ScarabDatum *list2aDatum = scarab_dict_get(dic2aDatum, key2);
		scarab_free_datum(key2);

		for(int i=0; i<BIG_NUMBER; ++i) {
			CPPUNIT_ASSERT(listElementArray[i]->ref_count == 1);
		}

		for(int i=0; i<BIG_NUMBER; ++i) {
			ScarabDatum *listElement2 = scarab_list_get(list2aDatum, i);
			CPPUNIT_ASSERT(listElement2->ref_count == 1);
			CPPUNIT_ASSERT(listElement2 == listElementArray[i]);
		}
		
		CPPUNIT_ASSERT(list2aDatum->ref_count == 1);
	}
	
	void testDictLeakyness() {
		fprintf(stderr, "Running GenericDataTestFixture::testDictLeakyness()\n");

		// testing leakyness for simple dictionaries
		const int num_elem = 1000;
		Data *dic1 = new Data(M_DICTIONARY,num_elem);
		ScarabDatum *dic1Datum = dic1->getScarabDatum();
		CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
		
		for(int i = 0; i<num_elem; i++)
		{	
			ostringstream oss;			
			oss << i;
			
			string key = "key" + oss.str();
			Data data((long)i);
			ScarabDatum *dataDatum = data.getScarabDatum();
			CPPUNIT_ASSERT(dataDatum->ref_count == 1);
			
			CPPUNIT_ASSERT(dic1Datum->ref_count == 1);						
			dic1->addElement(key.c_str(), data);
			CPPUNIT_ASSERT(dic1Datum->ref_count == 1);			
			CPPUNIT_ASSERT(dataDatum->ref_count == 2);
		}
		
		CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
		
		for(int i=0; i<dic1Datum->data.dict->tablesize; ++i) {
			ScarabDatum *key = dic1Datum->data.dict->keys[i];
			ScarabDatum *val = dic1Datum->data.dict->values[i];
			
			if(key) {
				CPPUNIT_ASSERT(key->ref_count == 1);			
				CPPUNIT_ASSERT(val->ref_count == 1);
			} else {
				CPPUNIT_ASSERT(val == 0);
			}
		}
		
		delete dic1;		
		
		// testing leakyness for more complicated dictionaries
		Data dic2(M_DICTIONARY,1);
		ScarabDatum *dic2Datum = dic2.getScarabDatum();
		CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
		
		for(int i = 0; i<num_elem; i++)
		{	
			ostringstream oss;			
			oss << i;
			
			string key = "key" + oss.str();
			Data data((long)i);
			ScarabDatum *dataDatum = data.getScarabDatum();
			CPPUNIT_ASSERT(dataDatum->ref_count == 1);
			
			CPPUNIT_ASSERT(dic2Datum->ref_count == 1);						
			dic2.addElement(key.c_str(), data);
			CPPUNIT_ASSERT(dic2Datum->ref_count == 1);			
			CPPUNIT_ASSERT(dataDatum->ref_count == 2);
		}
		
		CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
		
		for(int i=0; i<dic2Datum->data.dict->tablesize; ++i) {
			ScarabDatum *key = dic2Datum->data.dict->keys[i];
			ScarabDatum *val = dic2Datum->data.dict->values[i];

			if(key) {
				CPPUNIT_ASSERT(key->ref_count == 1);			
				CPPUNIT_ASSERT(val->ref_count == 1);
			} else {
				CPPUNIT_ASSERT(val == 0);
			}
		}
		
		
		// testing leakyness for getting dictionary items
		for(int i = 0; i<dic2.getNElements(); i++)
		{	
			ostringstream oss;			
			oss << i;
			
			string key = "key" + oss.str();
			
			Data data(dic2.getElement(key.c_str()));
			CPPUNIT_ASSERT(data == (long)i);
			ScarabDatum *dataDatum = data.getScarabDatum();
			
			// look here
			CPPUNIT_ASSERT(dic2Datum->ref_count == 1);			
			CPPUNIT_ASSERT(dataDatum->ref_count == 2);
		}
		
		Data data2 = dic2.getElement("This can't be a key");
		CPPUNIT_ASSERT(data2.getScarabDatum() == 0);
		CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
		
		for(int i=0; i<dic2Datum->data.dict->tablesize; ++i) {
			ScarabDatum *key = dic2Datum->data.dict->keys[i];
			ScarabDatum *val = dic2Datum->data.dict->values[i];
			
			if(key) {
				CPPUNIT_ASSERT(key->ref_count == 1);			
				CPPUNIT_ASSERT(val->ref_count == 1);
			} else {
				CPPUNIT_ASSERT(val == 0);
			}
		}

		
		
		// testing leakyness of sub dictionaries
		Data dicts[num_elem];
		Data master_dict(M_DICTIONARY, 1);
		ScarabDatum *masterDictDatum = master_dict.getScarabDatum();
		CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);
		
		for (int i=0; i<num_elem; ++i) {
			CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);
			Data newDict(M_DICTIONARY, 2);
			ScarabDatum *newDictDatum = newDict.getScarabDatum();
			CPPUNIT_ASSERT(newDictDatum->ref_count == 1);
			
			Data int1((long)i);
			ScarabDatum *int1Datum = int1.getScarabDatum();
			CPPUNIT_ASSERT(int1Datum->ref_count == 1);
			Data int2((long)2*i);
			ScarabDatum *int2Datum = int2.getScarabDatum();
			CPPUNIT_ASSERT(int2Datum->ref_count == 1);
			CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);
			
			
			string key1 = "key1";
			string key2 = "key2";
			newDict.addElement(key1.c_str(), int1);
			newDict.addElement(key2.c_str(), int2);
			CPPUNIT_ASSERT(newDictDatum->ref_count == 1);
			CPPUNIT_ASSERT(int1Datum->ref_count == 2);
			CPPUNIT_ASSERT(int2Datum->ref_count == 2);
			CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);

			dicts[i] = newDict;
			ScarabDatum *dictsDatum = dicts[i].getScarabDatum();
			CPPUNIT_ASSERT(dictsDatum->ref_count == 2);
			
			
			CPPUNIT_ASSERT(newDictDatum->ref_count == 2);
			CPPUNIT_ASSERT(int1Datum->ref_count == 2);
			CPPUNIT_ASSERT(int2Datum->ref_count == 2);
			
			ostringstream oss;			
			oss << i;
			
			string keyi = "key" + oss.str();
			master_dict.addElement(keyi.c_str(), dicts[i]);
			CPPUNIT_ASSERT(dictsDatum->ref_count == 3);
			CPPUNIT_ASSERT(int1Datum->ref_count == 2);
			CPPUNIT_ASSERT(int2Datum->ref_count == 2);
			CPPUNIT_ASSERT(newDictDatum->ref_count == 3);
			CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);
		}
		CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);
		
		ScarabDatum *dictsDatumArray[num_elem];
		for(int i=0; i<num_elem; ++i) {
			dictsDatumArray[i] = dicts[i].getScarabDatum();
			CPPUNIT_ASSERT(dictsDatumArray[i]->ref_count == 2);			
		}
		
		CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);
		for(int i=0; i<num_elem; ++i) {
			CPPUNIT_ASSERT(dictsDatumArray[i]->ref_count == 2);			
			dicts[i] = Data((long)i);
			CPPUNIT_ASSERT(dictsDatumArray[i]->ref_count == 1);			
		}
		CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);
		
	}	
		
	void testEmptyDictionary() {
		fprintf(stderr, "Running GenericDataTestFixture::testEmptyDictionary()\n");

		Data dic(M_DICTIONARY,1);

		CPPUNIT_ASSERT(!dic.hasKey("anything"));
		CPPUNIT_ASSERT(dic.getElement("something").isUndefined());
	}
	
	
	
	
	void testDictionary() {
		fprintf(stderr, "Running GenericDataTestFixture::testDictionary()\n");
			const int num_elem = 2000;
			
		Data *dic1 = new Data(M_DICTIONARY,1);
		
		for(int i = 0; i<num_elem; i++)
		{	
			ostringstream oss;			
			oss << i;
			
			
			string key = "key" + oss.str();
			Data data((long)i);
			
			dic1->addElement(key.c_str(), data);
		}
		ScarabDatum * sd = dic1->getScarabDatumCopy();
		delete dic1;

		Data *dic2 = new Data(sd);
		scarab_free_datum(sd);

		CPPUNIT_ASSERT(dic2->getNElements() == num_elem);

		for(int i = 0; i<num_elem; i++)
		{
			ostringstream oss;			
			oss << i;
			
			string key = "key" + oss.str();

			Data data = dic2->getElement(key.c_str());

			int result = data.getInteger();
			
			CPPUNIT_ASSERT(result == i);
		}
		
		Data *dic3 = new Data(*dic2);
		
		delete dic2;
		
		CPPUNIT_ASSERT(dic3->getNElements() == num_elem);
		
		for(int i = 0; i<num_elem; i++)
		{
			ostringstream oss;			
			oss << i;
			
			string key = "key" + oss.str();

			Data data = dic3->getElement(key.c_str());

			int result = data.getInteger();
			
			CPPUNIT_ASSERT(result == i);
		}
	
		Data dic4 = *dic3;	
		delete dic3;

		CPPUNIT_ASSERT(dic4.getNElements() == num_elem);
		
		for(int i = 0; i<num_elem; i++)
		{
			ostringstream oss;			
			oss << i;
		
			string key = "key" + oss.str();

			Data data = dic4.getElement(key.c_str());

			int result = data.getInteger();
			
			CPPUNIT_ASSERT(result == i);
		}
		
		Data dic5(M_DICTIONARY, 1.0);
		
		for(int i = 0; i<num_elem; ++i) {
			Data _i((long)i);
			
			dic5.addElement(_i, _i);
			CPPUNIT_ASSERT(dic5.getNElements() == i+1);
		}
		CPPUNIT_ASSERT(dic5.getNElements() == num_elem);

		for(int i = 0; i<num_elem; ++i) {
			Data _i((long)i);

			Data result(dic5.getElement(_i));
			CPPUNIT_ASSERT(result == _i);
		}
	}

	void testDictionaryStrings() {
		fprintf(stderr, "Running GenericDataTestFixture::testDictionaryStrings()\n");
		const int num_elem = 1000;
		fprintf(stderr, "next statement about dictionary sized 0 is OK\n");
		Data dic_c(M_DICTIONARY, 0);
		char * test = new char[20];
		char * key = new char[20];
		
		for(int i = 0; i<num_elem; i++)
		{	
			sprintf(test, "string%d", i);
			sprintf(key, "key%d", i);
			Data data(test);
			
			dic_c.addElement(key, data);
		}
		
		CPPUNIT_ASSERT(num_elem == dic_c.getNElements());
		
		for(int i = 0; i<num_elem; i++)
		{
			sprintf(test, "string%d", i);
			sprintf(key, "key%d", i);

			Data data = dic_c.getElement(key);

			string result = data.getString();

			CPPUNIT_ASSERT(result == test);
		}
		
		CPPUNIT_ASSERT(num_elem == dic_c.getNElements());
		delete [] key;
		delete [] test;
	}


	void testDictionaryAddGetElements() {
		fprintf(stderr, "Running GenericDataTestFixture::testDictionaryAddGetElements()\n");
		const int MAX_I=26;
		const int MAX_J=10;
		Data *dic_b;
		dic_b = new Data(M_DICTIONARY, 1);

		char key[3];

		for(int i = 0; i < MAX_I; i++){
			for(int j = 0; j < MAX_J; j++){
				CPPUNIT_ASSERT(dic_b->getNElements() == MAX_J*i+j);
				key[0]=(char)('a' + i);
				key[1]=(char)('a' + j);
				key[2]='\0';
				Data test((long)(2*i+1+j));
				dic_b->addElement(key,test);
				CPPUNIT_ASSERT(dic_b->getNElements() == (MAX_J*i+j+1));
			}
		}

		for(int j = 0; j < MAX_J; j++){
			for (int i = 0; i < MAX_I; i++){
				key[0]=(char)('a' + i);
				key[1]=(char)('a' + j);
				key[2]='\0';
				Data test2 = dic_b->getElement(key);
			
				CPPUNIT_ASSERT( (int) test2 ==  2*i+1+j );
			}
		}
		
		// now test reseting each element
		for(int i = 0; i < MAX_I; i++){
			for(int j = 0; j < MAX_J; j++){
				key[0]=(char)('a' + i);
				key[1]=(char)('a' + j);
				key[2]='\0';
				Data test((long)(3*i+1+j));
				dic_b->addElement(key,test);
				CPPUNIT_ASSERT(dic_b->getNElements() == MAX_I*MAX_J);
			}
		}
		
		for(int j = 0; j < MAX_J; j++){
			for (int i = 0; i < MAX_I; i++){
				key[0]=(char)('a' + i);
				key[1]=(char)('a' + j);
				key[2]='\0';
				Data test2 = dic_b->getElement(key);
			
				CPPUNIT_ASSERT( (int) test2 ==  3*i+1+j );
			}
		}
		
		// now test reseting each element
		for(int i = 0; i < MAX_I; i++){
			for(int j = 0; j < MAX_J; j++){
				key[0]=(char)('a' + i);
				key[1]=(char)('a' + j);
				key[2]='\0';
				Data test((long)(4*i+1+j));
				dic_b->addElement(key,test);
				CPPUNIT_ASSERT(dic_b->getNElements() == MAX_J*MAX_I);
			}
		}
		
		for(int j = 0; j < MAX_J; j++){
			for (int i = 0; i < MAX_I; i++){
				key[0]=(char)('a' + i);
				key[1]=(char)('a' + j);
				key[2]='\0';
				Data test2 = dic_b->getElement(key);
			
				CPPUNIT_ASSERT( (int) test2 ==  4*i+1+j );
			}
		}
		
		delete dic_b;
	}
	
	void testDictionaryKey() {
		fprintf(stderr, "Running GenericDataTestFixture::testDictionaryKey()\n");
		const int num_elem = 26;
		Data dic_d(M_DICTIONARY,1);
		Data val("Test string");
	
		char key[3];

		for(long i=0; i<num_elem; i++) {
			key[0]=(char)('a' + i);
			key[1]='a';
			key[2]='\0';
     
			dic_d.addElement(key, val);
		}

		for(long i=0; i<num_elem; i++) {
			key[0]=(char)('a' + i);
			key[1]='a';
			key[2]='\0';
     
			CPPUNIT_ASSERT(dic_d.hasKey(key));
		}

		for(long i=0; i<num_elem; i++) {
			key[0]=(char)('a' + i);
			key[1]='b';
			key[2]='\0';
     
			CPPUNIT_ASSERT(!dic_d.hasKey(key));
		}
		
	}
	
	void testDoubleTeamOnADictionary() {
		fprintf(stderr, "Running GenericDataTestFixture::testDoubleTeamOnADictionary()\n");

		Data list1(M_LIST, 2);
		Data list2(M_LIST, 2);

		{
			Data subdic(M_DICTIONARY, 2);
			ScarabDatum *dicDatum = subdic.getScarabDatum();
			Data int1((long)1);		
		
			subdic.addElement("Key 1", int1);

			CPPUNIT_ASSERT(dicDatum->ref_count == 1);
			list1.setElement(1, subdic);
			CPPUNIT_ASSERT(dicDatum->ref_count == 2);
			list2.setElement(1, subdic);
			CPPUNIT_ASSERT(dicDatum->ref_count == 3);
			
			CPPUNIT_ASSERT(list1.getNElements() == 1);
			CPPUNIT_ASSERT(list2.getNElements() == 1);
			CPPUNIT_ASSERT(subdic.getNElements() == 1);
		}
		CPPUNIT_ASSERT(list1.getNElements() == 1);
		CPPUNIT_ASSERT(list2.getNElements() == 1);

		Data dic1(list1.getElement(1));
		Data dic2(list2.getElement(1));
		
		CPPUNIT_ASSERT(dic1 == dic2);
		
		Data int2((long)2);
		dic1.addElement("Key 2", int2);
	
		
		// HERE'S THE DANGER POINT
		CPPUNIT_ASSERT(dic1 == dic2);

	}

	void testDataEqual() {
		fprintf(stderr, "Running GenericDataTestFixture::testDataEqual()\n");
		Data undef1;
		Data undef2;		
	
		Data dic1(M_DICTIONARY, 2);
		Data dic2(M_DICTIONARY, 2);
		
		Data list1(M_LIST, 2);
		Data list2(M_LIST, 2);
		
		Data int10(M_INTEGER, 10);
		Data int20(M_INTEGER, 20);

		Data float11(M_FLOAT, 1.1);
		Data float22(M_FLOAT, 2.2);

		Data string1("string1");
		Data string2("string2");

		Data temp;

		// various combinations of Data comparisons
		CPPUNIT_ASSERT(undef1 == undef2);
		CPPUNIT_ASSERT(undef2 == undef1);
			
		CPPUNIT_ASSERT(!(undef1 != undef2));
		CPPUNIT_ASSERT(!(undef2 != undef1));
			
		CPPUNIT_ASSERT(undef1 != int10);
		CPPUNIT_ASSERT(int10 != undef1);
			
		CPPUNIT_ASSERT(!(undef1 == int10));
		CPPUNIT_ASSERT(!(int10 == undef1));
			
		CPPUNIT_ASSERT(undef1 != float11);
		CPPUNIT_ASSERT(float11 != undef1);
			
		CPPUNIT_ASSERT(!(undef1 == float11));
		CPPUNIT_ASSERT(!(float11 == undef1));
			
		CPPUNIT_ASSERT(undef1 != string1);
		CPPUNIT_ASSERT(string1 != undef1);
			
		CPPUNIT_ASSERT(!(undef1 == string1));
		CPPUNIT_ASSERT(!(string1 == undef1));
			
		CPPUNIT_ASSERT(int10 != int20);
		CPPUNIT_ASSERT(int20 != int10);

		CPPUNIT_ASSERT(!(int10 == int20));
		CPPUNIT_ASSERT(!(int20 == int10));

		CPPUNIT_ASSERT(float11 != float22);
		CPPUNIT_ASSERT(float22 != float11);

		CPPUNIT_ASSERT(!(float11 == float22));
		CPPUNIT_ASSERT(!(float22 == float11));

		CPPUNIT_ASSERT(string1 != string2);
		CPPUNIT_ASSERT(string2 != string1);
		
		CPPUNIT_ASSERT(!(string1 == string2));
		CPPUNIT_ASSERT(!(string2 == string1));
		
		CPPUNIT_ASSERT(int10 != float11);
		CPPUNIT_ASSERT(float11 != int10);

		CPPUNIT_ASSERT(!(int10 == float11));
		CPPUNIT_ASSERT(!(float11 == int10));

		CPPUNIT_ASSERT(int10 != string1);
		CPPUNIT_ASSERT(string1 != int10);

		CPPUNIT_ASSERT(!(int10 == string1));
		CPPUNIT_ASSERT(!(string1 == int10));

		CPPUNIT_ASSERT(string1 != float11);
		CPPUNIT_ASSERT(float11 != string1);

		CPPUNIT_ASSERT(!(string1 == float11));
		CPPUNIT_ASSERT(!(float11 == string1));


		// two equal integers
		temp = int10;		
		CPPUNIT_ASSERT(temp == int10);
		CPPUNIT_ASSERT(int10 == temp);

		CPPUNIT_ASSERT(!(temp != int10));
		CPPUNIT_ASSERT(!(int10 != temp));

		// two equal floats
		temp = float11;
		CPPUNIT_ASSERT(temp == float11);
		CPPUNIT_ASSERT(float11 == temp);

		CPPUNIT_ASSERT(!(temp != float11));
		CPPUNIT_ASSERT(!(float11 != temp));

		// two equal strings
		temp = string1;
		CPPUNIT_ASSERT(string1 == temp);
		CPPUNIT_ASSERT(temp == string1);
		
		CPPUNIT_ASSERT(!(string1 != temp));
		CPPUNIT_ASSERT(!(temp != string1));
		
		
		// M_LIST
		list1.addElement(int10);
		list1.addElement(int20);

		// lists can't be equal to singletons
		CPPUNIT_ASSERT(undef1 != list1);
		CPPUNIT_ASSERT(list1 != undef1);
		
		CPPUNIT_ASSERT(!(undef1 == list1));
		CPPUNIT_ASSERT(!(list1 == undef1));
		
		CPPUNIT_ASSERT(int10 != list1);
		CPPUNIT_ASSERT(list1 != int10);
		
		CPPUNIT_ASSERT(!(int10 == list1));
		CPPUNIT_ASSERT(!(list1 == int10));
		
		CPPUNIT_ASSERT(string1 != list1);
		CPPUNIT_ASSERT(list1 != string1);
		
		CPPUNIT_ASSERT(!(string1 == list1));
		CPPUNIT_ASSERT(!(list1 == string1));
		
		CPPUNIT_ASSERT(float11 != list1);
		CPPUNIT_ASSERT(list1 != float11);
		
		CPPUNIT_ASSERT(!(float11 == list1));
		CPPUNIT_ASSERT(!(list1 == float11));
		
		// two equal lists
		temp = list1;		
		CPPUNIT_ASSERT(temp == list1);
		CPPUNIT_ASSERT(list1 == temp);

		CPPUNIT_ASSERT(!(temp != list1));
		CPPUNIT_ASSERT(!(list1 != temp));

		// two inequal sized lists
		list2.addElement(int10);
		CPPUNIT_ASSERT(list1 != list2);
		CPPUNIT_ASSERT(list2 != list1);

		CPPUNIT_ASSERT(!(list1 == list2));
		CPPUNIT_ASSERT(!(list2 == list1));

		// two inequal object lists
		list2.setElement(0, int20);		
		list2.addElement(int10);		
		CPPUNIT_ASSERT(list1 != list2);
		CPPUNIT_ASSERT(list2 != list1);

		CPPUNIT_ASSERT(!(list1 == list2));
		CPPUNIT_ASSERT(!(list2 == list1));

		// M_DICTIONARY
		string key1("key1");
		string key2("key2");
		
		dic1.addElement(key1.c_str(),int10);
		dic1.addElement(key2.c_str(),int20);

		// dics can't be equal to singletons
		CPPUNIT_ASSERT(undef1 != dic1);
		CPPUNIT_ASSERT(dic1 != undef1);
		
		CPPUNIT_ASSERT(!(undef1 == dic1));
		CPPUNIT_ASSERT(!(dic1 == undef1));
		
		CPPUNIT_ASSERT(int10 != dic1);
		CPPUNIT_ASSERT(dic1 != int10);
		
		CPPUNIT_ASSERT(!(int10 == dic1));
		CPPUNIT_ASSERT(!(dic1 == int10));
		
		CPPUNIT_ASSERT(string1 != dic1);
		CPPUNIT_ASSERT(dic1 != string1);
		
		CPPUNIT_ASSERT(!(string1 == dic1));
		CPPUNIT_ASSERT(!(dic1 == string1));
		
		CPPUNIT_ASSERT(float11 != dic1);
		CPPUNIT_ASSERT(dic1 != float11);

		CPPUNIT_ASSERT(!(float11 == dic1));
		CPPUNIT_ASSERT(!(dic1 == float11));

		// two equal dics
		temp = dic1;
		CPPUNIT_ASSERT(temp == dic1);
		CPPUNIT_ASSERT(dic1 == temp);
		
		CPPUNIT_ASSERT(!(temp != dic1));
		CPPUNIT_ASSERT(!(dic1 != temp));
		
		// two inequal sized dics
		fprintf(stderr, "next statements Re: \"...NULL ScarabDatum...\" and \"No key...\" are OK\n");
		dic2.addElement(key1.c_str(),int10);
		CPPUNIT_ASSERT(dic2 != dic1);
		CPPUNIT_ASSERT(dic1 != dic2);

		CPPUNIT_ASSERT(!(dic2 == dic1));
		CPPUNIT_ASSERT(!(dic1 == dic2));

		// two inequal dics
		dic2.addElement(key1.c_str(),int20);
		dic1.addElement(key2.c_str(),int10);		
		CPPUNIT_ASSERT(dic2 != dic1);
		CPPUNIT_ASSERT(dic1 != dic2);

		CPPUNIT_ASSERT(!(dic2 == dic1));
		CPPUNIT_ASSERT(!(dic1 == dic2));

		// dics and lists
		CPPUNIT_ASSERT(dic1 != list1);
		CPPUNIT_ASSERT(list1 != dic1);

		CPPUNIT_ASSERT(!(dic1 == list1));
		CPPUNIT_ASSERT(!(list1 == dic1));

		Data dic3(M_DICTIONARY, 6);
		Data dic4(M_DICTIONARY, 2);
		dic4.addElement(key1.c_str(), int10);
		dic4.addElement(key2.c_str(), int20);
		
		dic3.addElement(key1.c_str(), int10);
		CPPUNIT_ASSERT(dic3 != dic4);
		CPPUNIT_ASSERT(dic4 != dic3);

		CPPUNIT_ASSERT(!(dic4 == dic1));
		CPPUNIT_ASSERT(!(dic1 == dic4));

		// unequal sized (but containing the same elements)
		dic3.addElement(key2.c_str(), int20);
		CPPUNIT_ASSERT(dic3 == dic4);
		CPPUNIT_ASSERT(dic4 == dic3);

		CPPUNIT_ASSERT(!(dic3 != dic4));
		CPPUNIT_ASSERT(!(dic4 != dic3));

	}	
	
	void testAnyScalarConstructorBool();
	void testAnyScalarConstructorInt();
	void testAnyScalarConstructorLong();                                            
	void testAnyScalarConstructorByte();                                            
	void testAnyScalarConstructorShort();                                           
	void testAnyScalarConstructorWord();                                            
	void testAnyScalarConstructorDWord();                                           
	void testAnyScalarConstructorQWord();                                           
	void testAnyScalarConstructorFloat();                                           
	void testAnyScalarConstructorDouble();                                          
	void testAnyScalarConstructorString();                                          
	void testAnyScalarConstructorChar();                                            
	void testAnyScalarConstructorInvalid();  
	void testAnyScalarEqualBool();
	void testAnyScalarEqualInt();
	void testAnyScalarEqualLong();                                            
	void testAnyScalarEqualByte();                                            
	void testAnyScalarEqualShort();                                           
	void testAnyScalarEqualWord();                                            
	void testAnyScalarEqualDWord();                                           
	void testAnyScalarEqualQWord();                                           
	void testAnyScalarEqualFloat();                                           
	void testAnyScalarEqualDouble();                                          
	void testAnyScalarEqualString();                                          
	void testAnyScalarEqualChar();                                            
	void testAnyScalarEqualInvalid();  
	void testIs();
};


}
#endif

