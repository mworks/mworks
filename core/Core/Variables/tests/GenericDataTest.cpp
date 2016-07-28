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

#include "MWorksCore/GenericData.h"

using std::ostringstream;


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( GenericDataTestFixture, "Unit Test" );


void GenericDataTestFixture::testString() {
    Datum data("Test string");
    const char * test = "Test string";
    
    for(unsigned int i = 0; i<strlen(test); i++)
    {
        const char * result = data.getString().c_str();
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
        const char * result2 = data2.getString().c_str();
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
    
    Datum list2(M_LIST, 1);
    
    CPPUNIT_ASSERT(list1 == list2);
    
    list1.setElement(0, testInt1);
    
    CPPUNIT_ASSERT(list1.getNElements() == 1);
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
    CPPUNIT_ASSERT(list3.getElement(0) == testInt1);
    CPPUNIT_ASSERT(list3.getElement(1) == testInt2);
    
    CPPUNIT_ASSERT(list3.getElement(1) != testInt1);
    CPPUNIT_ASSERT(list3.getElement(0) != testInt2);
    
    Datum list4(M_LIST, 2);
    list4.setElement(0, testInt1);
    list4.setElement(1, testInt2);
    list4.addElement(testInt3);
    
    CPPUNIT_ASSERT(list4.getNElements() == 3);
    CPPUNIT_ASSERT(list4.getElement(0) == testInt1);
    CPPUNIT_ASSERT(list4.getElement(1) == testInt2);
    CPPUNIT_ASSERT(list4.getElement(2) == testInt3);
    
    Datum list5(M_LIST, 1);
    list5.setElement(0, testInt1);
    list5.setElement(10, testInt2);
    
    CPPUNIT_ASSERT(list5.getNElements() == 11);
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
    CPPUNIT_ASSERT(list7.getNElements() == 3);
    CPPUNIT_ASSERT(list7.getElement(0) == undefined);
    CPPUNIT_ASSERT(list7.getElement(1) == testInt1);
    CPPUNIT_ASSERT(list7.getElement(2) == testInt2);
    
    Datum list8(M_LIST, 3);
    list8.setElement(1, testInt1);
    list8.addElement(testInt2);
    CPPUNIT_ASSERT(list8.getNElements() == 3);
    CPPUNIT_ASSERT(list8.getElement(0) == undefined);
    CPPUNIT_ASSERT(list8.getElement(1) == testInt1);
    CPPUNIT_ASSERT(list8.getElement(2) == testInt2);
    
    Datum list9(M_LIST, 12);
    list9.setElement(1, testInt1);
    list9.addElement(testInt2);
    CPPUNIT_ASSERT(list9.getNElements() == 3);
    CPPUNIT_ASSERT(list9.getElement(0) == undefined);
    CPPUNIT_ASSERT(list9.getElement(1) == testInt1);
    CPPUNIT_ASSERT(list9.getElement(2) == testInt2);
    
    Datum list10(M_LIST, 1);
    list10.addElement(testInt1);
    CPPUNIT_ASSERT(list10.getNElements() == 1);
    CPPUNIT_ASSERT(list10.getElement(0) == testInt1);
    
    Datum list11(M_LIST, 1);
    list11.setElement(0, testInt1);
    list11.addElement(testInt2);
    CPPUNIT_ASSERT(list11.getNElements() == 2);
    CPPUNIT_ASSERT(list11.getElement(0) == testInt1);
    CPPUNIT_ASSERT(list11.getElement(1) == testInt2);
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
    
    Datum *dic3 = new Datum(*dic1);
    delete dic1;
    
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
    
    auto &dictValue = dic_d.getDict();
    CPPUNIT_ASSERT(dictValue.size() == num_elem);
    
    for(long i=0; i<num_elem; i++) {
        key[0]=(char)('a' + i);
        key[1]='a';
        key[2]='\0';
        
        CPPUNIT_ASSERT(dic_d.hasKey(key));
        CPPUNIT_ASSERT(dictValue.find(key) != dictValue.end());
    }
    
    for(long i=0; i<num_elem; i++) {
        key[0]=(char)('a' + i);
        key[1]='b';
        key[2]='\0';
        
        CPPUNIT_ASSERT(!dic_d.hasKey(key));
    }
    
}


void GenericDataTestFixture::testDictionaryWithDictionaryKeys() {
    //
    // NOTE: Although dictionaries really should *not* be used as dictionary keys, we nonetheless want
    // to ensure that doing so works as expected (even if very inefficiently)
    //
    
    const Datum key1 { Datum::dict_value_type { { Datum("a"), Datum(1) } } };
    const Datum key2 { Datum::dict_value_type { { Datum("b"), Datum(2) } } };
    const Datum key3 { Datum::dict_value_type { { Datum("c"), Datum(3) } } };
    
    // All dicts should have the same hash value
    CPPUNIT_ASSERT_EQUAL( key1.getHash(), key2.getHash() );
    CPPUNIT_ASSERT_EQUAL( key2.getHash(), key3.getHash() );
    
    const Datum d { Datum::dict_value_type { { key1, Datum(1.5) }, { key2, Datum(2.5) }, { key3, Datum(3.5) } } };
    
    CPPUNIT_ASSERT( d.isDictionary() );
    CPPUNIT_ASSERT_EQUAL( 3, d.getNElements() );
    CPPUNIT_ASSERT_EQUAL( Datum(1.5), d.getElement(key1) );
    CPPUNIT_ASSERT_EQUAL( Datum(2.5), d.getElement(key2) );
    CPPUNIT_ASSERT_EQUAL( Datum(3.5), d.getElement(key3) );
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
    CPPUNIT_ASSERT_EQUAL( std::string(""), Datum("").getString() );
    CPPUNIT_ASSERT_EQUAL( std::string("foo"), Datum("foo").getString() );
    
    // Other
    CPPUNIT_ASSERT_EQUAL( std::string(), Datum(3).getString() );
    assertError("ERROR: Cannot convert integer to string");
}


void GenericDataTestFixture::testGetList() {
    const Datum::list_value_type l1 { };
    const Datum::list_value_type l2 { Datum(false), Datum(1.5), Datum("foo") };
    
    // List
    CPPUNIT_ASSERT( l1 == Datum(l1).getList() );
    CPPUNIT_ASSERT( l2 == Datum(l2).getList() );
    
    // Other
    CPPUNIT_ASSERT( l1 == Datum(3).getList() );
    assertError("ERROR: Cannot convert integer to list");
}


void GenericDataTestFixture::testGetDict() {
    const Datum::dict_value_type d1 { };
    const Datum::dict_value_type d2 { { Datum("foo"), Datum(false) }, { Datum(1.5), Datum("bar") } };
    
    // Dict
    CPPUNIT_ASSERT( d1 == Datum(d1).getDict() );
    CPPUNIT_ASSERT( d2 == Datum(d2).getDict() );
    
    // Other
    CPPUNIT_ASSERT( d1 == Datum(3).getDict() );
    assertError("ERROR: Cannot convert integer to dictionary");
}


void GenericDataTestFixture::testGetElement() {
    // List
    {
        const Datum d { Datum::list_value_type { Datum(false), Datum(1.5), Datum("foo") } };
        
        CPPUNIT_ASSERT( Datum(false) == d.getElement(Datum(0)) );
        CPPUNIT_ASSERT( Datum(1.5) == d.getElement(Datum(1)) );
        CPPUNIT_ASSERT( Datum("foo") == d.getElement(Datum(2)) );
        
        // Non-integer index
        CPPUNIT_ASSERT( d.getElement(Datum("foo")).isUndefined() );
        assertError("ERROR: Cannot index list with string");
        
        // Out-of-bounds index
        CPPUNIT_ASSERT( d.getElement(Datum(-1)).isUndefined() );
        assertError("ERROR: Requested list index (-1) is out of bounds");
        CPPUNIT_ASSERT( d.getElement(Datum(3)).isUndefined() );
        assertError("ERROR: Requested list index (3) is out of bounds");
    }
    
    // Dict
    {
        const Datum d { Datum::dict_value_type { { Datum("foo"), Datum(1.5) }, { Datum(2), Datum("bar") } } };
        
        CPPUNIT_ASSERT( Datum(1.5) == d.getElement(Datum("foo")) );
        CPPUNIT_ASSERT( Datum("bar") == d.getElement(Datum(2)) );
        
        // Non-existent key (not an error)
        CPPUNIT_ASSERT( d.getElement(Datum("bar")).isUndefined() );
    }
    
    // Other
    {
        CPPUNIT_ASSERT( Datum("abc").getElement(Datum(0)).isUndefined() );
        assertError("ERROR: Cannot get element from string");
    }
    
    // Nested
    {
        const Datum d_bar_1_abc(1234);
        const Datum d_bar_1_def(5678);
        
        const Datum d_bar_0(123);
        const Datum d_bar_1 { Datum::dict_value_type { { Datum("abc"), d_bar_1_abc }, { Datum("def"), d_bar_1_def } } };
        const Datum d_bar { Datum::list_value_type { d_bar_0, d_bar_1 } };
        
        const Datum d_foo(1.5);
        
        const Datum d { Datum::dict_value_type { { Datum("foo"), d_foo }, { Datum("bar"), d_bar } } };
        
        CPPUNIT_ASSERT( d == d.getElement(std::vector<Datum>{}) );
        
        CPPUNIT_ASSERT( d_foo == d.getElement({ Datum("foo") }) );
        CPPUNIT_ASSERT( d.getElement({ Datum("foo"), Datum(0) }).isUndefined() );
        assertError("ERROR: Cannot get element from float");
        
        CPPUNIT_ASSERT( d_bar == d.getElement({ Datum("bar") }) );
        CPPUNIT_ASSERT( d_bar_0 == d.getElement({ Datum("bar"), Datum(0) }) );
        CPPUNIT_ASSERT( d_bar_1 == d.getElement({ Datum("bar"), Datum(1) }) );
        CPPUNIT_ASSERT( d.getElement({ Datum("bar"), Datum(2) }).isUndefined() );
        assertError("ERROR: Requested list index (2) is out of bounds");
        CPPUNIT_ASSERT( d.getElement({ Datum("bar"), Datum(2), Datum("abc") }).isUndefined() );
        assertError("ERROR: Requested list index (2) is out of bounds");
        
        CPPUNIT_ASSERT( d_bar_1_abc == d.getElement({ Datum("bar"), Datum(1), Datum("abc") }) );
        CPPUNIT_ASSERT( d_bar_1_def == d.getElement({ Datum("bar"), Datum(1), Datum("def") }) );
        CPPUNIT_ASSERT( d.getElement({ Datum("bar"), Datum(1), Datum("ghi") }).isUndefined() );
        CPPUNIT_ASSERT( d.getElement({ Datum("bar"), Datum(1), Datum("ghi"), Datum(2) }).isUndefined() );
    }
}


void GenericDataTestFixture::testSetElement() {
    // List
    {
        Datum d { Datum::list_value_type { Datum(1.5), Datum("foo") } };
        
        CPPUNIT_ASSERT( Datum(1.5) == d.getList().at(0) );
        d.setElement(Datum(0), Datum(2.5));
        CPPUNIT_ASSERT( Datum(2.5) == d.getList().at(0) );
        
        CPPUNIT_ASSERT( Datum("foo") == d.getList().at(1) );
        d.setElement(Datum(1), Datum("bar"));
        CPPUNIT_ASSERT( Datum("bar") == d.getList().at(1) );
        
        // Non-integer index
        d.setElement(Datum("foo"), Datum(3));
        assertError("ERROR: Cannot index list with string");
        
        // Negative index
        d.setElement(Datum(-1), Datum("foo"));
        assertError("ERROR: List index cannot be negative");
        
        // Automatic expansion
        CPPUNIT_ASSERT( 2 == d.getList().size() );
        d.setElement(Datum(3), Datum("bar"));
        CPPUNIT_ASSERT( 4 == d.getList().size() );
        CPPUNIT_ASSERT( d.getList().at(2).isUndefined() );
        CPPUNIT_ASSERT( Datum("bar") == d.getList().at(3) );
    }
    
    // Dict
    {
        Datum d { Datum::dict_value_type { { Datum("foo"), Datum(1.5) }, { Datum(2), Datum("bar") } } };
        
        CPPUNIT_ASSERT( Datum(1.5) == d.getDict().at(Datum("foo")) );
        d.setElement(Datum("foo"), Datum(2.5));
        CPPUNIT_ASSERT( Datum(2.5) == d.getDict().at(Datum("foo")) );
        
        CPPUNIT_ASSERT( Datum("bar") == d.getDict().at(Datum(2)) );
        d.setElement(Datum(2), Datum("blah"));
        CPPUNIT_ASSERT( Datum("blah") == d.getDict().at(Datum(2)) );
        
        // New key
        CPPUNIT_ASSERT( 2 == d.getDict().size() );
        CPPUNIT_ASSERT( d.getDict().find(Datum("abc")) == d.getDict().end() );
        d.setElement(Datum("abc"), Datum(123));
        CPPUNIT_ASSERT( 3 == d.getDict().size() );
        CPPUNIT_ASSERT( d.getDict().find(Datum("abc")) != d.getDict().end() );
        CPPUNIT_ASSERT( Datum(123) == d.getDict().at(Datum("abc")) );
    }
    
    // Other
    {
        Datum d("abc");
        d.setElement(Datum(0), Datum("A"));
        assertError("ERROR: Cannot set element in string");
    }
    
    // Nested
    {
        Datum d { Datum::dict_value_type {
            { Datum("foo"), Datum(1.5) },
            { Datum("bar"), Datum { Datum::list_value_type {
                Datum(123),
                Datum { Datum::dict_value_type {
                    { Datum("abc"), Datum(1234) },
                    { Datum("def"), Datum(5678) }
                } }
            } } }
        } };
        
        auto &d_foo = d.getDict().at(Datum("foo"));
        CPPUNIT_ASSERT( Datum(1.5) == d_foo );
        d.setElement({ Datum("foo") }, Datum(2.5) );
        CPPUNIT_ASSERT( Datum(2.5) == d_foo );
        
        auto &d_bar = d.getDict().at(Datum("bar"));
        
        CPPUNIT_ASSERT( Datum(123) == d_bar.getList().at(0) );
        d.setElement({ Datum("bar"), Datum(0) }, Datum(456) );
        CPPUNIT_ASSERT( Datum(456) == d_bar.getList().at(0) );
        
        CPPUNIT_ASSERT( 2 == d_bar.getList().size() );
        d.setElement({ Datum("bar"), Datum(3) }, Datum(3.5) );
        CPPUNIT_ASSERT( 4 == d_bar.getList().size() );
        CPPUNIT_ASSERT( d_bar.getList().at(2).isUndefined() );
        CPPUNIT_ASSERT( Datum(3.5) == d_bar.getList().at(3) );
        
        d.setElement({ Datum("bar"), Datum(-1) }, Datum(0) );
        assertError("ERROR: List index cannot be negative");
        d.setElement({ Datum("bar"), Datum(-1), Datum(0) }, Datum(0) );
        assertError("ERROR: List index cannot be negative");
        
        auto &d_bar_1 = d_bar.getList().at(1);
        
        CPPUNIT_ASSERT( Datum(5678) == d_bar_1.getDict().at(Datum("def")) );
        d.setElement({ Datum("bar"), Datum(1), Datum("def") }, Datum(8765) );
        CPPUNIT_ASSERT( Datum(8765) == d_bar_1.getDict().at(Datum("def")) );
        
        CPPUNIT_ASSERT( 2 == d_bar_1.getDict().size() );
        CPPUNIT_ASSERT( d_bar_1.getDict().find(Datum("ghi")) == d_bar_1.getDict().end() );
        d.setElement({ Datum("bar"), Datum(1), Datum("ghi") }, Datum(4321) );
        CPPUNIT_ASSERT( 3 == d_bar_1.getDict().size() );
        CPPUNIT_ASSERT( d_bar_1.getDict().find(Datum("ghi")) != d_bar_1.getDict().end() );
        CPPUNIT_ASSERT( Datum(4321) == d_bar_1.getDict().at(Datum("ghi")) );
        
        d.setElement({ Datum("bar"), Datum(1), Datum("abc"), Datum(0) }, Datum(1) );
        assertError("ERROR: Cannot set element in integer");
        d.setElement({ Datum("bar"), Datum(1), Datum("abc"), Datum(0), Datum("foo") }, Datum(1) );
        assertError("ERROR: Cannot set element in integer");
        
        d.setElement(std::vector<Datum>{}, Datum(3));
        CPPUNIT_ASSERT( Datum(3) == d );
    }
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
    
    // String
    {
        // and string
        {
            CPPUNIT_ASSERT( Datum("b") > Datum("a") );
            CPPUNIT_ASSERT( !(Datum("a") > Datum("a")) );
            CPPUNIT_ASSERT( !(Datum("a") > Datum("b")) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum("foo") > Datum(1.5)) );
            assertError("ERROR: Cannot test ordering of string and float");
        }
    }
    
    // Other
    {
        CPPUNIT_ASSERT( !(Datum() > Datum(1)) );
        assertError("ERROR: Cannot test ordering of undefined and integer");
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
    
    // String
    {
        // and string
        {
            CPPUNIT_ASSERT( Datum("b") >= Datum("a") );
            CPPUNIT_ASSERT( Datum("a") >= Datum("a") );
            CPPUNIT_ASSERT( !(Datum("a") >= Datum("b")) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum("foo") >= Datum(1.5)) );
            assertError("ERROR: Cannot test ordering of string and float");
        }
    }
    
    // Other
    {
        CPPUNIT_ASSERT( !(Datum() >= Datum(1)) );
        assertError("ERROR: Cannot test ordering of undefined and integer");
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
    
    // String
    {
        // and string
        {
            CPPUNIT_ASSERT( Datum("a") < Datum("b") );
            CPPUNIT_ASSERT( !(Datum("a") < Datum("a")) );
            CPPUNIT_ASSERT( !(Datum("b") < Datum("a")) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum("foo") < Datum(1.5)) );
            assertError("ERROR: Cannot test ordering of string and float");
        }
    }
    
    // Other
    {
        CPPUNIT_ASSERT( !(Datum() < Datum(1)) );
        assertError("ERROR: Cannot test ordering of undefined and integer");
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
    
    // String
    {
        // and string
        {
            CPPUNIT_ASSERT( Datum("a") <= Datum("b") );
            CPPUNIT_ASSERT( Datum("a") <= Datum("a") );
            CPPUNIT_ASSERT( !(Datum("b") <= Datum("a")) );
        }
        
        // and other
        {
            CPPUNIT_ASSERT( !(Datum("foo") <= Datum(1.5)) );
            assertError("ERROR: Cannot test ordering of string and float");
        }
    }
    
    // Other
    {
        CPPUNIT_ASSERT( !(Datum() <= Datum(1)) );
        assertError("ERROR: Cannot test ordering of undefined and integer");
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
            CPPUNIT_ASSERT( item.isUndefined() );
            assertError("ERROR: Cannot index list with string");
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
        assertError("ERROR: Cannot get element from integer");
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



























