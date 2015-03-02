/*
 *  GenericDataTest.cpp
 *  MWorksCore
 *
 *  Created by Ben Kennedy 2006
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "GenericDataTest.h"

#include <algorithm>
#include <sstream>

#include <boost/bind.hpp>

#include "MWorksCore/GenericData.h"
#include "MWorksCore/StandardVariables.h"

using std::ostringstream;


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( GenericDataTestFixture, "Unit Test" );


#define LOTS	1000000


static void *hammerit(void *thedatum){
    Datum *datum = (Datum *)thedatum;
    for(int i = 0; i < LOTS; i++){
        *datum = (long)i;
    }
    
    return 0;
}


static void *hammerlist(void *thedatum){
    Datum *datum = (Datum *)thedatum;
    for(int i = 0; i < LOTS; i++){
        datum->setElement(0, Datum((long)i));
    }
    
    return 0;
}


static void *hammerdict(void *thedatum){
    Datum *datum = (Datum *)thedatum;
    for(int i = 0; i < LOTS; i++){
        datum->addElement("test", Datum((long)i));
    }
    
    return 0;
}


void GenericDataTestFixture::setUp() {
    if (!GlobalMessageVariable) {
        VariableProperties *props = new VariableProperties(new Datum(0),
                                                           ANNOUNCE_MESSAGE_VAR_TAGNAME,
                                                           "message",
                                                           "message event channel",
                                                           M_NEVER,
                                                           M_WHEN_CHANGED,
                                                           true,
                                                           false,
                                                           M_STRUCTURED,
                                                           PRIVATE_SYSTEM_VARIABLES);
        GlobalMessageVariable = shared_ptr<GlobalVariable>(new GlobalVariable(props));
    }
    
    messageNotification = shared_ptr<VariableNotification>(
        new VariableCallbackNotification(boost::bind(&GenericDataTestFixture::handleNewMessage, this, _1, _2))
    );
    GlobalMessageVariable->addNotification(messageNotification);
}


void GenericDataTestFixture::tearDown() {
    messageNotification->remove();
}


#if INTERNALLY_LOCKED_MDATA
void GenericDataTestFixture::testMemoryManagement(){
    Datum test(M_LIST, 5);
    
    Datum *data = new Datum[5];
    for(int i = 0; i < 5; i++){
        data[i] = (long)i;
    }
    
    for(int i = 0; i < 5; i++){
        test.setElement(i, data[i]);
    }
    
    for(int i = 0; i < 5; i++){
        CPPUNIT_ASSERT(data[i] == test.getElement(i));
    }
    
    
    delete [] data;
    
    
    for(int i = 0; i < 5; i++){
        CPPUNIT_ASSERT((long)(test.getElement(i)) == i);
    }
    
    
    
    //fprintf(stderr, "Hammering an Datum object across two threads...");
    //fflush(stderr);
    Datum datum(0L);
    
    pthread_t thread;
    pthread_create(&thread, NULL, hammerit, (void *)(&datum));
    
    for(int i = 0; i < LOTS; i++){
        //if(i % (LOTS / 100) == 0){
        //	fprintf(stderr, "."); fflush(stderr);
        //}
        datum = (long)i;
    }
    //fprintf(stderr, "\n"); fflush(stderr);
    
    ScarabDatum *scarabdatum = datum.getScarabDatum();
    CPPUNIT_ASSERT(scarabdatum != NULL);
    CPPUNIT_ASSERT(scarabdatum->type == SCARAB_INTEGER);
    
    
    //fprintf(stderr, "Hammering an M_LIST Datum object across two threads...");
    //fflush(stderr);
    Datum list_datum(M_LIST, 1);
    
    pthread_t thread2;
    pthread_create(&thread2, NULL, hammerlist, (void *)(&list_datum));
    
    for(int i = 0; i < LOTS; i++){
        //if(i % (LOTS / 100) == 0){
        //	fprintf(stderr, "."); fflush(stderr);
        //}
        list_datum.setElement(0, Datum((long)i));
    }
    //fprintf(stderr, "\n"); fflush(stderr);
    
    scarabdatum = list_datum.getScarabDatum();
    CPPUNIT_ASSERT(scarabdatum != NULL);
    CPPUNIT_ASSERT(scarabdatum->type == SCARAB_LIST);
    
    //fprintf(stderr, "Hammering an M_DICT Datum object across two threads...");
    //fflush(stderr);
    Datum dict_datum(M_DICTIONARY, 1);
    
    pthread_t thread3;
    pthread_create(&thread3, NULL, hammerdict, (void *)(&dict_datum));
    
    for(int i = 0; i < LOTS; i++){
        //if(i % (LOTS / 100) == 0){
        //	fprintf(stderr, "."); fflush(stderr);
        //}
        dict_datum.addElement("test", Datum((long)i));
    }
    //fprintf(stderr, "\n"); fflush(stderr);
    
    scarabdatum = dict_datum.getScarabDatum();
    CPPUNIT_ASSERT(scarabdatum != NULL);
    CPPUNIT_ASSERT(scarabdatum->type == SCARAB_DICT);
    
}
#endif


void GenericDataTestFixture::testString() {
    Datum data("Test string");
    const char * test = "Test string";
    
    for(unsigned int i = 0; i<strlen(test); i++)
    {
        const char * result = data.getString();
        CPPUNIT_ASSERT(result[i] == test[i]);
    }
    
    Datum data2;
    const char * test2 = "Test second string";
    const char * test2a = "Test second string";
    
    CPPUNIT_ASSERT(strcmp(test2, test2a) == 0);
    
    data2 = Datum(test2);
    
    CPPUNIT_ASSERT(data2 == test2a);
    CPPUNIT_ASSERT(!(data2 != test2a));
    
    for(unsigned int i = 0; i<strlen(test2a); i++)
    {
        const char * result2 = data2.getString();
        CPPUNIT_ASSERT(result2[i] == test2a[i]);
    }
    
    
    data2 = "Test third string";
    const char * test3a = "Test third string";
    const char * test3b = "Test 3rd string";
    
    CPPUNIT_ASSERT(data2 == test3a);
    CPPUNIT_ASSERT(data2 != test3b);
    
    
    Datum data4a = "Test 4th string";
    Datum data4b = "Test fourth string";
    
    Datum boolean = (data4a != data4b);
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


void GenericDataTestFixture::testList() {
    Datum undefined;
    Datum list1(M_LIST, 1);
    Datum testInt1(M_INTEGER, 1);
    Datum testInt2(M_INTEGER, 2);
    Datum testInt3(M_INTEGER, 3);
    
    CPPUNIT_ASSERT(undefined != list1);
    CPPUNIT_ASSERT(undefined == list1.getElement(0));
    CPPUNIT_ASSERT(undefined == list1.getElement(1));
    
    CPPUNIT_ASSERT(list1.getNElements() == 0);
    CPPUNIT_ASSERT(list1.getMaxElements() == 1);
    
    Datum list2(M_LIST, 1);
    
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
    
    
    Datum list3(M_LIST, 1);
    list3.setElement(0, testInt1);
    list3.setElement(1, testInt2);
    
    CPPUNIT_ASSERT(list3.getNElements() == 2);
    CPPUNIT_ASSERT(list3.getMaxElements() == 2);
    CPPUNIT_ASSERT(list3.getElement(0) == testInt1);
    CPPUNIT_ASSERT(list3.getElement(1) == testInt2);
    
    CPPUNIT_ASSERT(list3.getElement(1) != testInt1);
    CPPUNIT_ASSERT(list3.getElement(0) != testInt2);
    
    Datum list4(M_LIST, 2);
    list4.setElement(0, testInt1);
    list4.setElement(1, testInt2);
    list4.addElement(testInt3);
    
    CPPUNIT_ASSERT(list4.getNElements() == 3);
    CPPUNIT_ASSERT(list4.getMaxElements() == 3);
    CPPUNIT_ASSERT(list4.getElement(0) == testInt1);
    CPPUNIT_ASSERT(list4.getElement(1) == testInt2);
    CPPUNIT_ASSERT(list4.getElement(2) == testInt3);
    
    Datum list5(M_LIST, 1);
    list5.setElement(0, testInt1);
    list5.setElement(10, testInt2);
    
    CPPUNIT_ASSERT(list5.getNElements() == 2);
    CPPUNIT_ASSERT(list5.getMaxElements() == 11);
    CPPUNIT_ASSERT(list5.getElement(0) == testInt1);
    CPPUNIT_ASSERT(list5.getElement(10) == testInt2);
    
    for(int i=1; i<10; ++i) {
        CPPUNIT_ASSERT(list5.getElement(i) == undefined);
    }
    
    Datum list6(M_LIST, 2);
    list6.setElement(0, testInt1);
    list6.setElement(10, testInt2);
    
    CPPUNIT_ASSERT(list6 == list5);
    
    Datum list7(M_LIST, 2);
    list7.setElement(1, testInt1);
    list7.addElement(testInt2);
    CPPUNIT_ASSERT(list7.getNElements() == 2);
    CPPUNIT_ASSERT(list7.getMaxElements() == 3);
    CPPUNIT_ASSERT(list7.getElement(0) == undefined);
    CPPUNIT_ASSERT(list7.getElement(1) == testInt1);
    CPPUNIT_ASSERT(list7.getElement(2) == testInt2);
    
    Datum list8(M_LIST, 3);
    list8.setElement(1, testInt1);
    list8.addElement(testInt2);
    CPPUNIT_ASSERT(list8.getNElements() == 2);
    CPPUNIT_ASSERT(list8.getMaxElements() == 3);
    CPPUNIT_ASSERT(list8.getElement(0) == undefined);
    CPPUNIT_ASSERT(list8.getElement(1) == testInt1);
    CPPUNIT_ASSERT(list8.getElement(2) == testInt2);
    
    Datum list9(M_LIST, 12);
    list9.setElement(1, testInt1);
    list9.addElement(testInt2);
    CPPUNIT_ASSERT(list9.getNElements() == 2);
    CPPUNIT_ASSERT(list9.getMaxElements() == 12);
    CPPUNIT_ASSERT(list9.getElement(0) == undefined);
    CPPUNIT_ASSERT(list9.getElement(1) == testInt1);
    CPPUNIT_ASSERT(list9.getElement(2) == testInt2);
    
    Datum list10(M_LIST, 1);
    list10.addElement(testInt1);
    CPPUNIT_ASSERT(list10.getNElements() == 1);
    CPPUNIT_ASSERT(list10.getMaxElements() == 1);
    CPPUNIT_ASSERT(list10.getElement(0) == testInt1);
    
    Datum list11(M_LIST, 1);
    list11.setElement(0, testInt1);
    list11.addElement(testInt2);
    CPPUNIT_ASSERT(list11.getNElements() == 2);
    CPPUNIT_ASSERT(list11.getMaxElements() == 2);
    CPPUNIT_ASSERT(list11.getElement(0) == testInt1);
    CPPUNIT_ASSERT(list11.getElement(1) == testInt2);
}


void GenericDataTestFixture::testListLeakyness() {
    // testing leakyness for simple lists
    const int num_elem = 8;
    Datum list1(M_LIST, num_elem);
    ScarabDatum *list1Datum = list1.getScarabDatum();
    
    for(int i = 0; i<num_elem; i++) {
        Datum data((long)i);
        ScarabDatum *dataDatum = data.getScarabDatum();
        CPPUNIT_ASSERT(dataDatum->ref_count == 1);
        
        CPPUNIT_ASSERT(list1Datum->ref_count == 1);
        list1.addElement(data);
        CPPUNIT_ASSERT(list1Datum->ref_count == 1);
        CPPUNIT_ASSERT(dataDatum->ref_count == 2);
    }
    
    CPPUNIT_ASSERT(list1Datum->ref_count == 1);
    
    Datum list2(M_LIST, 1);
    
    for(int i = 0; i<num_elem; i++) {
        Datum data((long)i);
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


void GenericDataTestFixture::testListOverwriteLeakyness() {
    Datum list1(M_LIST, 2);
    ScarabDatum *list1Datum = list1.getScarabDatum();
    
    ScarabDatum *long1Datum;
    ScarabDatum *long2Datum;
    {
        Datum long1((long)1);
        Datum long2((long)2);
        
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
        Datum long3((long)3);
        long3Datum = long3.getScarabDatum();
        
        CPPUNIT_ASSERT(long1Datum->ref_count == 1);
        CPPUNIT_ASSERT(long2Datum->ref_count == 1);
        CPPUNIT_ASSERT(long3Datum->ref_count == 1);
        
        Datum long1Copy(long1Datum);
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


void GenericDataTestFixture::testDictOverwriteLeakyness() {
    Datum dic1(M_DICTIONARY, 2);
    ScarabDatum *dic1Datum = dic1.getScarabDatum();
    
    ScarabDatum *long1Datum;
    ScarabDatum *long2Datum;
    {
        Datum long1((long)1);
        Datum long2((long)2);
        
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
        Datum long3((long)3);
        long3Datum = long3.getScarabDatum();
        
        CPPUNIT_ASSERT(long1Datum->ref_count == 1);
        CPPUNIT_ASSERT(long2Datum->ref_count == 1);
        CPPUNIT_ASSERT(long3Datum->ref_count == 1);
        
        Datum long1Copy(long1Datum);
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


void GenericDataTestFixture::testListUnderADictionary() {
    ScarabDatum *dic1Datum;
    {
        Datum dic1(M_DICTIONARY, 2);
        dic1Datum = dic1.getScarabDatum();
        CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
        
        ScarabDatum *list1Datum;
        ScarabDatum *list2Datum;
        ScarabDatum *int1Datum;
        ScarabDatum *int2Datum;
        ScarabDatum *int3Datum;
        ScarabDatum *int4Datum;
        
        {
            Datum list1(M_LIST, 2);
            Datum list2(M_LIST, 2);
            list1Datum = list1.getScarabDatum();
            list2Datum = list2.getScarabDatum();
            CPPUNIT_ASSERT(list1Datum->ref_count == 1);
            CPPUNIT_ASSERT(list2Datum->ref_count == 1);
            CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
            
            
            Datum int1((long)1);
            Datum int2((long)2);
            Datum int3((long)3);
            Datum int4((long)4);
            
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
    Datum dic2a;
    const int BIG_NUMBER=20000;
    ScarabDatum *listElementArray[BIG_NUMBER];
    {
        Datum dic2;
        dic2 = Datum(M_DICTIONARY, 5);
        dic2Datum = dic2.getScarabDatum();
        CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
        
        double *doubleVector = new double [BIG_NUMBER];
        for(int i=0; i<BIG_NUMBER; ++i) {
            doubleVector[i] = (double)i/10;
        }
        
        MWTime time = 42;
        
        // look here
        dic2.addElement("six",(long)6);
        dic2.addElement("MWT",(Datum)time);
        
        
        ScarabDatum *key;
        
        key = scarab_new_string("six");
        ScarabDatum *six = scarab_dict_get(dic2Datum, key);
        scarab_free_datum(key);
        //fprintf(stderr, "refcount = %d", six->ref_count);fflush(stderr);
        CPPUNIT_ASSERT(six->ref_count == 1);
        CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
        
        key = scarab_new_string("MWT");
        ScarabDatum *MWT = scarab_dict_get(dic2Datum, key);
        scarab_free_datum(key);
        
        CPPUNIT_ASSERT(six->ref_count == 1);
        CPPUNIT_ASSERT(MWT->ref_count == 1);
        CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
        
        ScarabDatum *list3Datum;
        Datum list3(M_LIST, BIG_NUMBER);
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


void GenericDataTestFixture::testDictLeakyness() {
    // testing leakyness for simple dictionaries
    const int num_elem = 1000;
    Datum *dic1 = new Datum(M_DICTIONARY,num_elem);
    ScarabDatum *dic1Datum = dic1->getScarabDatum();
    CPPUNIT_ASSERT(dic1Datum->ref_count == 1);
    
    for(int i = 0; i<num_elem; i++)
    {
        ostringstream oss;
        oss << i;
        
        string key = "key" + oss.str();
        Datum data((long)i);
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
    Datum dic2(M_DICTIONARY,1);
    ScarabDatum *dic2Datum = dic2.getScarabDatum();
    CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
    
    for(int i = 0; i<num_elem; i++)
    {
        ostringstream oss;
        oss << i;
        
        string key = "key" + oss.str();
        Datum data((long)i);
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
        
        Datum data(dic2.getElement(key.c_str()));
        CPPUNIT_ASSERT(data == (long)i);
        ScarabDatum *dataDatum = data.getScarabDatum();
        
        // look here
        CPPUNIT_ASSERT(dic2Datum->ref_count == 1);
        CPPUNIT_ASSERT(dataDatum->ref_count == 2);
    }
    
    Datum data2 = dic2.getElement("This can't be a key");
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
    Datum dicts[num_elem];
    Datum master_dict(M_DICTIONARY, 1);
    ScarabDatum *masterDictDatum = master_dict.getScarabDatum();
    CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);
    
    for (int i=0; i<num_elem; ++i) {
        CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);
        Datum newDict(M_DICTIONARY, 2);
        ScarabDatum *newDictDatum = newDict.getScarabDatum();
        CPPUNIT_ASSERT(newDictDatum->ref_count == 1);
        
        Datum int1((long)i);
        ScarabDatum *int1Datum = int1.getScarabDatum();
        CPPUNIT_ASSERT(int1Datum->ref_count == 1);
        Datum int2((long)2*i);
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
        dicts[i] = Datum((long)i);
        CPPUNIT_ASSERT(dictsDatumArray[i]->ref_count == 1);
    }
    CPPUNIT_ASSERT(masterDictDatum->ref_count == 1);
    
}


void GenericDataTestFixture::testEmptyDictionary() {
    Datum dic(M_DICTIONARY,1);
    
    CPPUNIT_ASSERT(!dic.hasKey("anything"));
    CPPUNIT_ASSERT(dic.getElement("something").isUndefined());
}


void GenericDataTestFixture::testDictionary() {
    const int num_elem = 2000;
    
    Datum *dic1 = new Datum(M_DICTIONARY,1);
    
    for(int i = 0; i<num_elem; i++)
    {
        ostringstream oss;
        oss << i;
        
        
        string key = "key" + oss.str();
        Datum data((long)i);
        
        dic1->addElement(key.c_str(), data);
    }
    ScarabDatum * sd = dic1->getScarabDatumCopy();
    delete dic1;
    
    Datum *dic2 = new Datum(sd);
    scarab_free_datum(sd);
    
    CPPUNIT_ASSERT(dic2->getNElements() == num_elem);
    
    for(int i = 0; i<num_elem; i++)
    {
        ostringstream oss;
        oss << i;
        
        string key = "key" + oss.str();
        
        Datum data = dic2->getElement(key.c_str());
        
        int result = data.getInteger();
        
        CPPUNIT_ASSERT(result == i);
    }
    
    Datum *dic3 = new Datum(*dic2);
    
    delete dic2;
    
    CPPUNIT_ASSERT(dic3->getNElements() == num_elem);
    
    for(int i = 0; i<num_elem; i++)
    {
        ostringstream oss;
        oss << i;
        
        string key = "key" + oss.str();
        
        Datum data = dic3->getElement(key.c_str());
        
        int result = data.getInteger();
        
        CPPUNIT_ASSERT(result == i);
    }
    
    Datum dic4 = *dic3;
    delete dic3;
    
    CPPUNIT_ASSERT(dic4.getNElements() == num_elem);
    
    for(int i = 0; i<num_elem; i++)
    {
        ostringstream oss;
        oss << i;
        
        string key = "key" + oss.str();
        
        Datum data = dic4.getElement(key.c_str());
        
        int result = data.getInteger();
        
        CPPUNIT_ASSERT(result == i);
    }
    
    Datum dic5(M_DICTIONARY, 1.0);
    
    for(int i = 0; i<num_elem; ++i) {
        Datum _i((long)i);
        
        dic5.addElement(_i, _i);
        CPPUNIT_ASSERT(dic5.getNElements() == i+1);
    }
    CPPUNIT_ASSERT(dic5.getNElements() == num_elem);
    
    for(int i = 0; i<num_elem; ++i) {
        Datum _i((long)i);
        
        Datum result(dic5.getElement(_i));
        CPPUNIT_ASSERT(result == _i);
    }
}


void GenericDataTestFixture::testDictionaryStrings() {
    const int num_elem = 1000;
    //fprintf(stderr, "next statement about dictionary sized 0 is OK\n");
    Datum dic_c(M_DICTIONARY, 0);
    char * test = new char[20];
    char * key = new char[20];
    
    for(int i = 0; i<num_elem; i++)
    {
        sprintf(test, "string%d", i);
        sprintf(key, "key%d", i);
        Datum data(test);
        
        dic_c.addElement(key, data);
    }
    
    CPPUNIT_ASSERT(num_elem == dic_c.getNElements());
    
    for(int i = 0; i<num_elem; i++)
    {
        sprintf(test, "string%d", i);
        sprintf(key, "key%d", i);
        
        Datum data = dic_c.getElement(key);
        
        string result = data.getString();
        
        CPPUNIT_ASSERT(result == test);
    }
    
    CPPUNIT_ASSERT(num_elem == dic_c.getNElements());
    delete [] key;
    delete [] test;
}


void GenericDataTestFixture::testDictionaryAddGetElements() {
    const int MAX_I=26;
    const int MAX_J=10;
    Datum *dic_b;
    dic_b = new Datum(M_DICTIONARY, 1);
    
    char key[3];
    
    for(int i = 0; i < MAX_I; i++){
        for(int j = 0; j < MAX_J; j++){
            CPPUNIT_ASSERT(dic_b->getNElements() == MAX_J*i+j);
            key[0]=(char)('a' + i);
            key[1]=(char)('a' + j);
            key[2]='\0';
            Datum test((long)(2*i+1+j));
            dic_b->addElement(key,test);
            CPPUNIT_ASSERT(dic_b->getNElements() == (MAX_J*i+j+1));
        }
    }
    
    for(int j = 0; j < MAX_J; j++){
        for (int i = 0; i < MAX_I; i++){
            key[0]=(char)('a' + i);
            key[1]=(char)('a' + j);
            key[2]='\0';
            Datum test2 = dic_b->getElement(key);
            
            CPPUNIT_ASSERT( (int) test2 ==  2*i+1+j );
        }
    }
    
    // now test resetting each element
    for(int i = 0; i < MAX_I; i++){
        for(int j = 0; j < MAX_J; j++){
            key[0]=(char)('a' + i);
            key[1]=(char)('a' + j);
            key[2]='\0';
            Datum test((long)(3*i+1+j));
            dic_b->addElement(key,test);
            CPPUNIT_ASSERT(dic_b->getNElements() == MAX_I*MAX_J);
        }
    }
    
    for(int j = 0; j < MAX_J; j++){
        for (int i = 0; i < MAX_I; i++){
            key[0]=(char)('a' + i);
            key[1]=(char)('a' + j);
            key[2]='\0';
            Datum test2 = dic_b->getElement(key);
            
            CPPUNIT_ASSERT( (int) test2 ==  3*i+1+j );
        }
    }
    
    // now test resetting each element
    for(int i = 0; i < MAX_I; i++){
        for(int j = 0; j < MAX_J; j++){
            key[0]=(char)('a' + i);
            key[1]=(char)('a' + j);
            key[2]='\0';
            Datum test((long)(4*i+1+j));
            dic_b->addElement(key,test);
            CPPUNIT_ASSERT(dic_b->getNElements() == MAX_J*MAX_I);
        }
    }
    
    for(int j = 0; j < MAX_J; j++){
        for (int i = 0; i < MAX_I; i++){
            key[0]=(char)('a' + i);
            key[1]=(char)('a' + j);
            key[2]='\0';
            Datum test2 = dic_b->getElement(key);
            
            CPPUNIT_ASSERT( (int) test2 ==  4*i+1+j );
        }
    }
    
    delete dic_b;
}


void GenericDataTestFixture::testDictionaryKey() {
    const int num_elem = 26;
    Datum dic_d(M_DICTIONARY,1);
    Datum val("Test string");
    
    char key[3];
    
    for(long i=0; i<num_elem; i++) {
        key[0]=(char)('a' + i);
        key[1]='a';
        key[2]='\0';
        
        dic_d.addElement(key, val);
    }
    
    std::vector<Datum> allKeys = dic_d.getKeys();
    CPPUNIT_ASSERT(allKeys.size() == num_elem);
    
    for(long i=0; i<num_elem; i++) {
        key[0]=(char)('a' + i);
        key[1]='a';
        key[2]='\0';
        
        CPPUNIT_ASSERT(dic_d.hasKey(key));
        CPPUNIT_ASSERT(std::find(allKeys.begin(), allKeys.end(), key) != allKeys.end());
    }
    
    for(long i=0; i<num_elem; i++) {
        key[0]=(char)('a' + i);
        key[1]='b';
        key[2]='\0';
        
        CPPUNIT_ASSERT(!dic_d.hasKey(key));
    }
    
}


void GenericDataTestFixture::testDoubleTeamOnADictionary() {
    Datum list1(M_LIST, 2);
    Datum list2(M_LIST, 2);
    
    {
        Datum subdic(M_DICTIONARY, 2);
        ScarabDatum *dicDatum = subdic.getScarabDatum();
        Datum int1((long)1);
        
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
    
    Datum dic1(list1.getElement(1));
    Datum dic2(list2.getElement(1));
    
    CPPUNIT_ASSERT(dic1 == dic2);
    
    Datum int2((long)2);
    dic1.addElement("Key 2", int2);
    
    
    // HERE'S THE DANGER POINT
    CPPUNIT_ASSERT(dic1 == dic2);
    
}


void GenericDataTestFixture::testDataEqual() {
    Datum undef1;
    Datum undef2;
    
    Datum dic1(M_DICTIONARY, 2);
    Datum dic2(M_DICTIONARY, 2);
    
    Datum list1(M_LIST, 2);
    Datum list2(M_LIST, 2);
    
    Datum int10(M_INTEGER, 10);
    Datum int20(M_INTEGER, 20);
    
    Datum float11(M_FLOAT, 1.1);
    Datum float22(M_FLOAT, 2.2);
    
    Datum string1("string1");
    Datum string2("string2");
    
    Datum temp;
    
    // various combinations of Datum comparisons
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
    //fprintf(stderr, "next statements Re: \"...NULL ScarabDatum...\" and \"No key...\" are OK\n");
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
    
    Datum dic3(M_DICTIONARY, 6);
    Datum dic4(M_DICTIONARY, 2);
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


void GenericDataTestFixture::testIs() {
    Datum an_undefined;
    CPPUNIT_ASSERT(an_undefined.isUndefined());
    CPPUNIT_ASSERT(!an_undefined.isNumber());
    
    Datum a_float(M_FLOAT, 0.0);
    CPPUNIT_ASSERT(a_float.isFloat());
    CPPUNIT_ASSERT(a_float.isNumber());
    
    Datum an_int = 1L;
    CPPUNIT_ASSERT(an_int.isInteger());
    CPPUNIT_ASSERT(an_int.isNumber());
    
    Datum a_bool(M_BOOLEAN, false);
    CPPUNIT_ASSERT(a_bool.isBool());
    CPPUNIT_ASSERT(a_bool.isInteger());
    CPPUNIT_ASSERT(a_bool.isNumber());
    
    Datum a_string = "";
    CPPUNIT_ASSERT(a_string.isString());
    CPPUNIT_ASSERT(!a_string.isNumber());
    
    Datum a_list(M_LIST, 1);
    CPPUNIT_ASSERT(a_list.isList());
    CPPUNIT_ASSERT(!a_list.isNumber());
    
    Datum a_dict(M_DICTIONARY, 1);
    CPPUNIT_ASSERT(a_dict.isDictionary());
    CPPUNIT_ASSERT(!a_dict.isNumber());
}


void GenericDataTestFixture::testGetDataTypeName() {
    CPPUNIT_ASSERT( std::string("undefined") == Datum().getDataTypeName() );
    CPPUNIT_ASSERT( std::string("boolean") == Datum(true).getDataTypeName() );
    CPPUNIT_ASSERT( std::string("integer") == Datum(1).getDataTypeName() );
    CPPUNIT_ASSERT( std::string("float") == Datum(1.0).getDataTypeName() );
    CPPUNIT_ASSERT( std::string("string") == Datum("foo").getDataTypeName() );
    CPPUNIT_ASSERT( std::string("list") == Datum(M_LIST, 0).getDataTypeName() );
    CPPUNIT_ASSERT( std::string("dictionary") == Datum(M_DICTIONARY, 0).getDataTypeName() );
}


void GenericDataTestFixture::testGetBool() {
    // Boolean
    CPPUNIT_ASSERT( Datum(true).getBool() );
    CPPUNIT_ASSERT( !(Datum(false).getBool()) );
    
    // Integer
    CPPUNIT_ASSERT( Datum(1).getBool() );
    CPPUNIT_ASSERT( !(Datum(0).getBool()) );
    CPPUNIT_ASSERT( Datum(-1).getBool() );
    
    // Float
    CPPUNIT_ASSERT( Datum(1.0).getBool() );
    CPPUNIT_ASSERT( !(Datum(0.0).getBool()) );
    CPPUNIT_ASSERT( Datum(-1.0).getBool() );
    
    // Other
    CPPUNIT_ASSERT( !(Datum("foo").getBool()) );
    assertError("ERROR: Cannot convert string to boolean");
}


void GenericDataTestFixture::testGetInteger() {
    // Boolean
    CPPUNIT_ASSERT_EQUAL( 1LL, Datum(true).getInteger() );
    CPPUNIT_ASSERT_EQUAL( 0LL, Datum(false).getInteger() );
    
    // Integer
    CPPUNIT_ASSERT_EQUAL( 2LL,  Datum(2).getInteger() );
    CPPUNIT_ASSERT_EQUAL( -3LL,  Datum(-3).getInteger() );
    
    // Float
    CPPUNIT_ASSERT_EQUAL( 1LL,  Datum(1.7).getInteger() );
    CPPUNIT_ASSERT_EQUAL( -2LL,  Datum(-2.3).getInteger() );
    
    // Other
    CPPUNIT_ASSERT_EQUAL( 0LL, Datum("foo").getInteger() );
    assertError("ERROR: Cannot convert string to integer");
}


void GenericDataTestFixture::testGetFloat() {
    // Boolean
    CPPUNIT_ASSERT_EQUAL( 1.0, Datum(true).getFloat() );
    CPPUNIT_ASSERT_EQUAL( 0.0, Datum(false).getFloat() );
    
    // Integer
    CPPUNIT_ASSERT_EQUAL( 2.0,  Datum(2).getFloat() );
    CPPUNIT_ASSERT_EQUAL( -3.0,  Datum(-3).getFloat() );
    
    // Float
    CPPUNIT_ASSERT_EQUAL( 1.5,  Datum(1.5).getFloat() );
    CPPUNIT_ASSERT_EQUAL( -2.5,  Datum(-2.5).getFloat() );
    
    // Other
    CPPUNIT_ASSERT_EQUAL( 0.0, Datum("foo").getFloat() );
    assertError("ERROR: Cannot convert string to float");
}


void GenericDataTestFixture::testGetString() {
    // String
    CPPUNIT_ASSERT_EQUAL( std::string(""), std::string(Datum("").getString()) );
    CPPUNIT_ASSERT_EQUAL( std::string("foo"), std::string(Datum("foo").getString()) );
    
    // Other
    CPPUNIT_ASSERT( nullptr == Datum(3).getString() );
    assertError("ERROR: Cannot convert integer to string");
}


void GenericDataTestFixture::testOperatorUnaryMinus() {
    // Boolean
    {
        Datum d = -Datum(true);
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( -1LL, d.getInteger() );
    }
    
    // Integer
    {
        Datum d = -Datum(-3);
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 3LL, d.getInteger() );
    }
    
    // Float
    {
        Datum d = -Datum(1.5);
        CPPUNIT_ASSERT( d.isFloat() );
        CPPUNIT_ASSERT_EQUAL( -1.5, d.getFloat() );
    }
    
    // Other
    {
        Datum d = -Datum("foo");
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
        assertError("ERROR: Cannot negate string");
    }
}


void GenericDataTestFixture::testOperatorBinaryPlus() {
    // Boolean
    {
        // and boolean
        {
            Datum d = Datum(true) + Datum(false);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 1LL, d.getInteger() );
        }
        
        // and integer
        {
            Datum d = Datum(true) + Datum(3);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 4LL, d.getInteger() );
        }
        
        // and float
        {
            Datum d = Datum(true) + Datum(1.5);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 2.5, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(true) + Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot add boolean and string");
        }
    }
    
    // Integer
    {
        // and boolean
        {
            Datum d = Datum(3) + Datum(true);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 4LL, d.getInteger() );
        }
        
        // and integer
        {
            Datum d = Datum(3) + Datum(5);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 8LL, d.getInteger() );
        }
        
        // and float
        {
            Datum d = Datum(2) + Datum(1.5);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 3.5, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(3) + Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot add integer and string");
        }
    }
    
    // Float
    {
        // and boolean
        {
            Datum d = Datum(1.5) + Datum(true);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 2.5, d.getFloat() );
        }
        
        // and integer
        {
            Datum d = Datum(1.5) + Datum(-3);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( -1.5, d.getFloat() );
        }
        
        // and float
        {
            Datum d = Datum(1.5) + Datum(4.0);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 5.5, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(1.5) + Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot add float and string");
        }
    }
    
    // Other
    {
        Datum d = Datum("foo") + Datum(1);
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
        assertError("ERROR: Cannot add string and integer");
    }
}


void GenericDataTestFixture::testOperatorBinaryMinus() {
    // Boolean
    {
        // and boolean
        {
            Datum d = Datum(true) - Datum(false);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 1LL, d.getInteger() );
        }
        
        // and integer
        {
            Datum d = Datum(true) - Datum(3);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( -2LL, d.getInteger() );
        }
        
        // and float
        {
            Datum d = Datum(true) - Datum(1.5);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( -0.5, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(true) - Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot subtract boolean and string");
        }
    }
    
    // Integer
    {
        // and boolean
        {
            Datum d = Datum(3) - Datum(true);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 2LL, d.getInteger() );
        }
        
        // and integer
        {
            Datum d = Datum(3) - Datum(5);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( -2LL, d.getInteger() );
        }
        
        // and float
        {
            Datum d = Datum(2) - Datum(1.5);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 0.5, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(3) - Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot subtract integer and string");
        }
    }
    
    // Float
    {
        // and boolean
        {
            Datum d = Datum(1.5) - Datum(true);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 0.5, d.getFloat() );
        }
        
        // and integer
        {
            Datum d = Datum(1.5) - Datum(-3);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 4.5, d.getFloat() );
        }
        
        // and float
        {
            Datum d = Datum(1.5) - Datum(4.0);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( -2.5, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(1.5) - Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot subtract float and string");
        }
    }
    
    // Other
    {
        Datum d = Datum("foo") - Datum(1);
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
        assertError("ERROR: Cannot subtract string and integer");
    }
}


void GenericDataTestFixture::testOperatorTimes() {
    // Boolean
    {
        // and boolean
        {
            Datum d = Datum(true) * Datum(true);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 1LL, d.getInteger() );
        }
        
        // and integer
        {
            Datum d = Datum(true) * Datum(3);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 3LL, d.getInteger() );
        }
        
        // and float
        {
            Datum d = Datum(true) * Datum(1.5);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 1.5, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(true) * Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot multiply boolean and string");
        }
    }
    
    // Integer
    {
        // and boolean
        {
            Datum d = Datum(3) * Datum(true);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 3LL, d.getInteger() );
        }
        
        // and integer
        {
            Datum d = Datum(3) * Datum(5);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 15LL, d.getInteger() );
        }
        
        // and float
        {
            Datum d = Datum(3) * Datum(1.5);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 4.5, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(3) * Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot multiply integer and string");
        }
    }
    
    // Float
    {
        // and boolean
        {
            Datum d = Datum(1.5) * Datum(true);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 1.5, d.getFloat() );
        }
        
        // and integer
        {
            Datum d = Datum(1.5) * Datum(-3);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( -4.5, d.getFloat() );
        }
        
        // and float
        {
            Datum d = Datum(1.5) * Datum(4.0);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 6.0, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(1.5) * Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot multiply float and string");
        }
    }
    
    // Other
    {
        Datum d = Datum("foo") * Datum(1);
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
        assertError("ERROR: Cannot multiply string and integer");
    }
}


void GenericDataTestFixture::testOperatorDivide() {
    // Boolean
    {
        // and boolean
        {
            Datum d = Datum(true) / Datum(true);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 1.0, d.getFloat() );
        }
        
        // and integer
        {
            Datum d = Datum(true) / Datum(-1);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( -1.0, d.getFloat() );
        }
        
        // and float
        {
            Datum d = Datum(true) / Datum(2.0);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 0.5, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(true) / Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot divide boolean and string");
        }
        
        // by zero
        {
            Datum d = Datum(true) / Datum(false);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Division by zero");
        }
    }
    
    // Integer
    {
        // and boolean
        {
            Datum d = Datum(3) / Datum(true);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 3.0, d.getFloat() );
        }
        
        // and integer
        {
            Datum d = Datum(6) / Datum(3);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 2.0, d.getFloat() );
        }
        
        // and float
        {
            Datum d = Datum(2) / Datum(0.5);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 4.0, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(3) / Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot divide integer and string");
        }
        
        // by zero
        {
            Datum d = Datum(1) / Datum(0);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Division by zero");
        }
    }
    
    // Float
    {
        // and boolean
        {
            Datum d = Datum(1.5) / Datum(true);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 1.5, d.getFloat() );
        }
        
        // and integer
        {
            Datum d = Datum(1.5) / Datum(-3);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( -0.5, d.getFloat() );
        }
        
        // and float
        {
            Datum d = Datum(4.5) / Datum(1.5);
            CPPUNIT_ASSERT( d.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 3.0, d.getFloat() );
        }
        
        // and other
        {
            Datum d = Datum(1.5) / Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot divide float and string");
        }
        
        // by zero
        {
            Datum d = Datum(1.0) / Datum(0.0);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Division by zero");
        }
    }
    
    // Other
    {
        Datum d = Datum("foo") / Datum(1);
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
        assertError("ERROR: Cannot divide string and integer");
    }
}


void GenericDataTestFixture::testOperatorModulo() {
    // Boolean
    {
        // and boolean
        {
            Datum d = Datum(true) % Datum(true);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
        }
        
        // and integer
        {
            Datum d = Datum(true) % Datum(2);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 1LL, d.getInteger() );
        }
        
        // and float
        {
            Datum d = Datum(true) % Datum(2.9);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 1LL, d.getInteger() );
        }
        
        // and other
        {
            Datum d = Datum(true) % Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot mod boolean and string");
        }
        
        // by zero
        {
            Datum d = Datum(true) % Datum(false);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Division by zero");
        }
    }
    
    // Integer
    {
        // and boolean
        {
            Datum d = Datum(3) % Datum(true);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
        }
        
        // and integer
        {
            Datum d = Datum(8) % Datum(3);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 2LL, d.getInteger() );
        }
        
        // and float
        {
            Datum d = Datum(2) % Datum(3.9);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 2LL, d.getInteger() );
        }
        
        // and other
        {
            Datum d = Datum(3) % Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot mod integer and string");
        }
        
        // by zero
        {
            Datum d = Datum(1) % Datum(0);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Division by zero");
        }
    }
    
    // Float
    {
        // and boolean
        {
            Datum d = Datum(3.9) % Datum(true);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
        }
        
        // and integer
        {
            Datum d = Datum(8.9) % Datum(3);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 2LL, d.getInteger() );
        }
        
        // and float
        {
            Datum d = Datum(2.9) % Datum(3.9);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 2LL, d.getInteger() );
        }
        
        // and other
        {
            Datum d = Datum(3.0) % Datum("foo");
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Cannot mod float and string");
        }
        
        // by zero
        {
            Datum d = Datum(1.0) % Datum(0.0);
            CPPUNIT_ASSERT( d.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
            assertError("ERROR: Division by zero");
        }
    }
    
    // Other
    {
        Datum d = Datum("foo") % Datum(1);
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
        assertError("ERROR: Cannot mod string and integer");
    }
}


void GenericDataTestFixture::testOperatorGreaterThan() {
    // Boolean
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(true) > Datum(false) );
            CPPUNIT_ASSERT( !(Datum(true) > Datum(true)) );
            CPPUNIT_ASSERT( !(Datum(false) > Datum(true)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(true) > Datum(0) );
            CPPUNIT_ASSERT( !(Datum(true) > Datum(1)) );
            CPPUNIT_ASSERT( !(Datum(false) > Datum(1)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(true) > Datum(0.0) );
            CPPUNIT_ASSERT( !(Datum(true) > Datum(1.0)) );
            CPPUNIT_ASSERT( !(Datum(false) > Datum(1.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(true) > Datum("foo")) );
            assertError("ERROR: Cannot test ordering of boolean and string");
        }
    }
    
    // Integer
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(1) > Datum(false) );
            CPPUNIT_ASSERT( !(Datum(1) > Datum(true)) );
            CPPUNIT_ASSERT( !(Datum(0) > Datum(true)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(1) > Datum(0) );
            CPPUNIT_ASSERT( !(Datum(1) > Datum(1)) );
            CPPUNIT_ASSERT( !(Datum(0) > Datum(1)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(1) > Datum(0.0) );
            CPPUNIT_ASSERT( !(Datum(1) > Datum(1.0)) );
            CPPUNIT_ASSERT( !(Datum(0) > Datum(1.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(3) > Datum("foo")) );
            assertError("ERROR: Cannot test ordering of integer and string");
        }
    }
    
    // Float
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(1.0) > Datum(false) );
            CPPUNIT_ASSERT( !(Datum(1.0) > Datum(true)) );
            CPPUNIT_ASSERT( !(Datum(0.0) > Datum(true)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(1.0) > Datum(0) );
            CPPUNIT_ASSERT( !(Datum(1.0) > Datum(1)) );
            CPPUNIT_ASSERT( !(Datum(0.0) > Datum(1)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(1.0) > Datum(0.0) );
            CPPUNIT_ASSERT( !(Datum(1.0) > Datum(1.0)) );
            CPPUNIT_ASSERT( !(Datum(0.0) > Datum(1.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(1.5) > Datum("foo")) );
            assertError("ERROR: Cannot test ordering of float and string");
        }
    }
    
    // Other
    {
        CPPUNIT_ASSERT( !(Datum("foo") > Datum(1)) );
        assertError("ERROR: Cannot test ordering of string and integer");
    }
}


void GenericDataTestFixture::testOperatorGreaterThanOrEqual() {
    // Boolean
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(true) >= Datum(false) );
            CPPUNIT_ASSERT( Datum(true) >= Datum(true) );
            CPPUNIT_ASSERT( !(Datum(false) >= Datum(true)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(true) >= Datum(0) );
            CPPUNIT_ASSERT( Datum(true) >= Datum(1) );
            CPPUNIT_ASSERT( !(Datum(false) >= Datum(1)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(true) >= Datum(0.0) );
            CPPUNIT_ASSERT( Datum(true) >= Datum(1.0) );
            CPPUNIT_ASSERT( !(Datum(false) >= Datum(1.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(true) >= Datum("foo")) );
            assertError("ERROR: Cannot test ordering of boolean and string");
        }
    }
    
    // Integer
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(1) >= Datum(false) );
            CPPUNIT_ASSERT( Datum(1) >= Datum(true) );
            CPPUNIT_ASSERT( !(Datum(0) >= Datum(true)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(1) >= Datum(0) );
            CPPUNIT_ASSERT( Datum(1) >= Datum(1) );
            CPPUNIT_ASSERT( !(Datum(0) >= Datum(1)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(1) >= Datum(0.0) );
            CPPUNIT_ASSERT( Datum(1) >= Datum(1.0) );
            CPPUNIT_ASSERT( !(Datum(0) >= Datum(1.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(3) >= Datum("foo")) );
            assertError("ERROR: Cannot test ordering of integer and string");
        }
    }
    
    // Float
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(1.0) >= Datum(false) );
            CPPUNIT_ASSERT( Datum(1.0) >= Datum(true) );
            CPPUNIT_ASSERT( !(Datum(0.0) >= Datum(true)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(1.0) >= Datum(0) );
            CPPUNIT_ASSERT( Datum(1.0) >= Datum(1) );
            CPPUNIT_ASSERT( !(Datum(0.0) >= Datum(1)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(1.0) >= Datum(0.0) );
            CPPUNIT_ASSERT( Datum(1.0) >= Datum(1.0) );
            CPPUNIT_ASSERT( !(Datum(0.0) >= Datum(1.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(1.5) >= Datum("foo")) );
            assertError("ERROR: Cannot test ordering of float and string");
        }
    }
    
    // Other
    {
        CPPUNIT_ASSERT( !(Datum("foo") >= Datum(1)) );
        assertError("ERROR: Cannot test ordering of string and integer");
    }
}


void GenericDataTestFixture::testOperatorLessThan() {
    // Boolean
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(false) < Datum(true) );
            CPPUNIT_ASSERT( !(Datum(false) < Datum(false)) );
            CPPUNIT_ASSERT( !(Datum(true) < Datum(false)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(false) < Datum(1) );
            CPPUNIT_ASSERT( !(Datum(false) < Datum(0)) );
            CPPUNIT_ASSERT( !(Datum(true) < Datum(0)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(false) < Datum(1.0) );
            CPPUNIT_ASSERT( !(Datum(false) < Datum(0.0)) );
            CPPUNIT_ASSERT( !(Datum(true) < Datum(0.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(true) < Datum("foo")) );
            assertError("ERROR: Cannot test ordering of boolean and string");
        }
    }
    
    // Integer
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(0) < Datum(true) );
            CPPUNIT_ASSERT( !(Datum(0) < Datum(false)) );
            CPPUNIT_ASSERT( !(Datum(1) < Datum(false)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(0) < Datum(1) );
            CPPUNIT_ASSERT( !(Datum(0) < Datum(0)) );
            CPPUNIT_ASSERT( !(Datum(1) < Datum(0)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(0) < Datum(1.0) );
            CPPUNIT_ASSERT( !(Datum(0) < Datum(0.0)) );
            CPPUNIT_ASSERT( !(Datum(1) < Datum(0.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(3) < Datum("foo")) );
            assertError("ERROR: Cannot test ordering of integer and string");
        }
    }
    
    // Float
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(0.0) < Datum(true) );
            CPPUNIT_ASSERT( !(Datum(0.0) < Datum(false)) );
            CPPUNIT_ASSERT( !(Datum(1.0) < Datum(false)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(0.0) < Datum(1) );
            CPPUNIT_ASSERT( !(Datum(0.0) < Datum(0)) );
            CPPUNIT_ASSERT( !(Datum(1.0) < Datum(0)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(0.0) < Datum(1.0) );
            CPPUNIT_ASSERT( !(Datum(0.0) < Datum(0.0)) );
            CPPUNIT_ASSERT( !(Datum(1.0) < Datum(0.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(1.5) < Datum("foo")) );
            assertError("ERROR: Cannot test ordering of float and string");
        }
    }
    
    // Other
    {
        CPPUNIT_ASSERT( !(Datum("foo") < Datum(1)) );
        assertError("ERROR: Cannot test ordering of string and integer");
    }
}


void GenericDataTestFixture::testOperatorLessThanOrEqual() {
    // Boolean
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(false) <= Datum(true) );
            CPPUNIT_ASSERT( Datum(false) <= Datum(false) );
            CPPUNIT_ASSERT( !(Datum(true) <= Datum(false)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(false) <= Datum(1) );
            CPPUNIT_ASSERT( Datum(false) <= Datum(0) );
            CPPUNIT_ASSERT( !(Datum(true) <= Datum(0)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(false) <= Datum(1.0) );
            CPPUNIT_ASSERT( Datum(false) <= Datum(0.0) );
            CPPUNIT_ASSERT( !(Datum(true) <= Datum(0.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(true) <= Datum("foo")) );
            assertError("ERROR: Cannot test ordering of boolean and string");
        }
    }
    
    // Integer
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(0) <= Datum(true) );
            CPPUNIT_ASSERT( Datum(0) <= Datum(false) );
            CPPUNIT_ASSERT( !(Datum(1) <= Datum(false)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(0) <= Datum(1) );
            CPPUNIT_ASSERT( Datum(0) <= Datum(0) );
            CPPUNIT_ASSERT( !(Datum(1) <= Datum(0)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(0) <= Datum(1.0) );
            CPPUNIT_ASSERT( Datum(0) <= Datum(0.0) );
            CPPUNIT_ASSERT( !(Datum(1) <= Datum(0.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(3) <= Datum("foo")) );
            assertError("ERROR: Cannot test ordering of integer and string");
        }
    }
    
    // Float
    {
        // and boolean
        {
            CPPUNIT_ASSERT( Datum(0.0) <= Datum(true) );
            CPPUNIT_ASSERT( Datum(0.0) <= Datum(false) );
            CPPUNIT_ASSERT( !(Datum(1.0) <= Datum(false)) );
        }
        
        // and integer
        {
            CPPUNIT_ASSERT( Datum(0.0) <= Datum(1) );
            CPPUNIT_ASSERT( Datum(0.0) <= Datum(0) );
            CPPUNIT_ASSERT( !(Datum(1.0) <= Datum(0)) );
        }
        
        // and float
        {
            CPPUNIT_ASSERT( Datum(0.0) <= Datum(1.0) );
            CPPUNIT_ASSERT( Datum(0.0) <= Datum(0.0) );
            CPPUNIT_ASSERT( !(Datum(1.0) <= Datum(0.0)) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum(1.5) <= Datum("foo")) );
            assertError("ERROR: Cannot test ordering of float and string");
        }
    }
    
    // Other
    {
        CPPUNIT_ASSERT( !(Datum("foo") <= Datum(1)) );
        assertError("ERROR: Cannot test ordering of string and integer");
    }
}


void GenericDataTestFixture::testGenericIndexing() {
    // List
    {
        Datum list(M_LIST, 2);
        list.setElement(0, Datum(1));
        list.setElement(1, Datum(2.5));
        
        // Valid indices
        {
            Datum item = list[Datum(0)];
            CPPUNIT_ASSERT( item.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 1LL, item.getInteger() );
        }
        {
            Datum item = list[Datum(1)];
            CPPUNIT_ASSERT( item.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 2.5, item.getFloat() );
        }
        
        // Out-of-bounds indices
        {
            Datum item = list[Datum(-1)];
            CPPUNIT_ASSERT( item.isUndefined() );
            assertError("ERROR: Requested list index (-1) is out of bounds");
        }
        {
            Datum item = list[Datum(2)];
            CPPUNIT_ASSERT( item.isUndefined() );
            assertError("ERROR: Requested list index (2) is out of bounds");
        }
        
        // Non-numeric index
        {
            Datum item = list[Datum("foo")];
            // Datum::getInteger returns 0 on error, so the retrieved item is the first element
            CPPUNIT_ASSERT( item.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 1LL, item.getInteger() );
            assertError("ERROR: Cannot convert string to integer");
        }
    }
    
    // Dictionary
    {
        Datum dict(M_DICTIONARY, 2);
        dict.addElement(Datum("foo"), Datum(1));
        dict.addElement(Datum("bar"), Datum(2.5));
        
        // Valid keys
        {
            Datum item = dict[Datum("foo")];
            CPPUNIT_ASSERT( item.isInteger() );
            CPPUNIT_ASSERT_EQUAL( 1LL, item.getInteger() );
        }
        {
            Datum item = dict[Datum("bar")];
            CPPUNIT_ASSERT( item.isFloat() );
            CPPUNIT_ASSERT_EQUAL( 2.5, item.getFloat() );
        }
        
        // Non-existent key
        {
            Datum item = dict[Datum("blah")];
            CPPUNIT_ASSERT( item.isUndefined() );
            assertError("ERROR: Dictionary has no element for requested key (\"blah\")");
        }
    }
    
    // Other
    {
        Datum item = Datum(3)[Datum("blah")];
        CPPUNIT_ASSERT( item.isUndefined() );
        assertError("ERROR: Cannot subscript integer");
    }
}


void GenericDataTestFixture::handleNewMessage(const Datum &value, MWTime time) {
    if (value.isDictionary()) {
        messageValue = value;
    }
}


void GenericDataTestFixture::assertMessage(MessageType type, const std::string &msg) {
    CPPUNIT_ASSERT( messageValue.isDictionary() );
    
    {
        CPPUNIT_ASSERT( messageValue.hasKey(M_MESSAGE_TYPE) );
        Datum actualType = messageValue.getElement(M_MESSAGE_TYPE);
        CPPUNIT_ASSERT( actualType.isInteger() );
        CPPUNIT_ASSERT_EQUAL( type, MessageType(actualType.getInteger()) );
    }
    
    {
        CPPUNIT_ASSERT( messageValue.hasKey(M_MESSAGE) );
        Datum actualMsg = messageValue.getElement(M_MESSAGE);
        CPPUNIT_ASSERT( actualMsg.isString() );
        CPPUNIT_ASSERT_EQUAL( msg, std::string(actualMsg) );
    }
}


void GenericDataTestFixture::assertEqualStrings(const std::string &expected, const std::string &actual) {
    CPPUNIT_ASSERT_EQUAL( expected, actual );
}


void GenericDataTestFixture::testGetStringQuoted() {
    // Non-string
    assertEqualStrings("", Datum(1).getStringQuoted());
    assertError("ERROR: Cannot convert integer to string");
    
    // String
    assertEqualStrings(R"("a b c \a \b \f \n \r \t \v \\ \" \' d e f")",
                       Datum("a b c \a \b \f \n \r \t \v \\ \" ' d e f").getStringQuoted());
}


void GenericDataTestFixture::testSetStringQuoted() {
    Datum d;
    auto setQuoted = [&d](const std::string &s) {
        d.setStringQuoted(s);
        return d.getString();
    };
    
    // Fewer than 2 characters
    assertEqualStrings( "", setQuoted("") );
    assertEqualStrings( "'", setQuoted("'") );
    
    // Not fully quoted
    assertEqualStrings( "abc", setQuoted("abc") );
    assertEqualStrings( "\"abc", setQuoted("\"abc") );
    assertEqualStrings( "'abc", setQuoted("'abc") );
    assertEqualStrings( "abc\"", setQuoted("abc\"") );
    assertEqualStrings( "abc'", setQuoted("abc'") );
    
    // Last quoted char is backslash
    assertEqualStrings( R"('ab\')", setQuoted(R"('ab\')") );
    
    // Empty
    assertEqualStrings( "", setQuoted("\"\"") );
    assertEqualStrings( "", setQuoted("''") );
    
    // Non-empty
    assertEqualStrings("a b c \a \b \f \n \r \t \v ' \" \\ ? \\q d e f",
                       setQuoted(R"("a b c \a \b \f \n \r \t \v \' \" \\ \? \q d e f")"));
}


END_NAMESPACE_MW



























