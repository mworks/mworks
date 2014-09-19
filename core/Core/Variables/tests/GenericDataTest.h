#ifndef	GENERIC_DATA_TEST_H_
#define GENERIC_DATA_TEST_H_

/*
 *  GenericDataTest.h
 *  MWorksCore
 *
 *  Created by Ben Kennedy 07142006
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksCore/MWorksMacros.h"


BEGIN_NAMESPACE_MW


class GenericDataTestFixture : public CppUnit::TestFixture {
    
    CPPUNIT_TEST_SUITE( GenericDataTestFixture );
    
#if INTERNALLY_LOCKED_MDATA
    CPPUNIT_TEST( testMemoryManagement );
#endif
    CPPUNIT_TEST( testString );
    CPPUNIT_TEST( testList );
    CPPUNIT_TEST( testListLeakyness );
    CPPUNIT_TEST( testListOverwriteLeakyness );
    CPPUNIT_TEST( testDictOverwriteLeakyness );
    CPPUNIT_TEST( testListUnderADictionary );
    CPPUNIT_TEST( testDictLeakyness );
    CPPUNIT_TEST( testEmptyDictionary );
    CPPUNIT_TEST( testDictionary );
    CPPUNIT_TEST( testDictionaryStrings );
    CPPUNIT_TEST( testDictionaryAddGetElements );
    CPPUNIT_TEST( testDictionaryKey );
    CPPUNIT_TEST( testDoubleTeamOnADictionary );
    CPPUNIT_TEST( testDataEqual );
    CPPUNIT_TEST( testIs );
    
    CPPUNIT_TEST_SUITE_END();
    
public:
#if INTERNALLY_LOCKED_MDATA
    void testMemoryManagement();
#endif
    void testString();
    void testList();
    void testListLeakyness();
    void testListOverwriteLeakyness();
    void testDictOverwriteLeakyness();
    void testListUnderADictionary();
    void testDictLeakyness();
    void testEmptyDictionary();
    void testDictionary();
    void testDictionaryStrings();
    void testDictionaryAddGetElements();
    void testDictionaryKey();
    void testDoubleTeamOnADictionary();
    void testDataEqual();
    void testIs();
    
};


END_NAMESPACE_MW


#endif



























