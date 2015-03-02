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

#include "MWorksCore/EventConstants.h"
#include "MWorksCore/VariableNotification.h"


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
    
    CPPUNIT_TEST( testGetDataTypeName );
    CPPUNIT_TEST( testGetBool );
    CPPUNIT_TEST( testGetInteger );
    CPPUNIT_TEST( testGetFloat );
    CPPUNIT_TEST( testGetString );
    CPPUNIT_TEST( testGetStringQuoted );
    CPPUNIT_TEST( testSetStringQuoted );
    CPPUNIT_TEST( testOperatorUnaryMinus );
    CPPUNIT_TEST( testOperatorBinaryPlus );
    CPPUNIT_TEST( testOperatorBinaryMinus );
    CPPUNIT_TEST( testOperatorTimes );
    CPPUNIT_TEST( testOperatorDivide );
    CPPUNIT_TEST( testOperatorModulo );
    CPPUNIT_TEST( testOperatorGreaterThan );
    CPPUNIT_TEST( testOperatorGreaterThanOrEqual );
    CPPUNIT_TEST( testOperatorLessThan );
    CPPUNIT_TEST( testOperatorLessThanOrEqual );
    CPPUNIT_TEST( testGenericIndexing );
    
    CPPUNIT_TEST_SUITE_END();
    
public:
    void setUp();
    void tearDown();
    
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
    
    void testGetDataTypeName();
    void testGetBool();
    void testGetInteger();
    void testGetFloat();
    void testGetString();
    void testGetStringQuoted();
    void testSetStringQuoted();
    void testOperatorUnaryMinus();
    void testOperatorBinaryPlus();
    void testOperatorBinaryMinus();
    void testOperatorTimes();
    void testOperatorDivide();
    void testOperatorModulo();
    void testOperatorGreaterThan();
    void testOperatorGreaterThanOrEqual();
    void testOperatorLessThan();
    void testOperatorLessThanOrEqual();
    void testGenericIndexing();
    
private:
    void handleNewMessage(const Datum &value, MWTime time);
    void assertError(const std::string &msg) { assertMessage(M_ERROR_MESSAGE, msg); }
    void assertMessage(MessageType type, const std::string &msg);
    void assertEqualStrings(const std::string &expected, const std::string &actual);
    
    shared_ptr<VariableNotification> messageNotification;
    Datum messageValue;
    
};


END_NAMESPACE_MW


#endif



























