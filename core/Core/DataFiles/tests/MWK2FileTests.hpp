//
//  MWK2FileTests.hpp
//  MWorksCoreTest
//
//  Created by Christopher Stawarz on 9/7/18.
//

#ifndef MWK2FileTests_hpp
#define MWK2FileTests_hpp

#include "MessageTestFixture.hpp"


BEGIN_NAMESPACE_MW


class MWK2FileTests : public MessageTestFixture {
    
    CPPUNIT_TEST_SUITE( MWK2FileTests );
    
    CPPUNIT_TEST( testFileAlreadyExists );
    CPPUNIT_TEST( testFileDoesNotExist );
    CPPUNIT_TEST( testInvalidFile );
    CPPUNIT_TEST( testWriterLock );
    CPPUNIT_TEST( testEmptyFile );
    CPPUNIT_TEST( testDataTypes );
    CPPUNIT_TEST( testEventConcatenation );
    CPPUNIT_TEST( testSorting );
    CPPUNIT_TEST( testSelection );
    CPPUNIT_TEST( testWriteEvent );
    CPPUNIT_TEST( testWriteEvents );
    CPPUNIT_TEST( testInvalidEventData );
    CPPUNIT_TEST( testEventDataCompression );
    CPPUNIT_TEST( testEventConcatenationWithCompression );
    CPPUNIT_TEST( testInvalidCompressedData );
    
    CPPUNIT_TEST_SUITE_END();
    
public:
    void testFileAlreadyExists();
    void testFileDoesNotExist();
    void testInvalidFile();
    void testWriterLock();
    void testEmptyFile();
    void testDataTypes();
    void testEventConcatenation();
    void testSorting();
    void testSelection();
    void testWriteEvent();
    void testWriteEvents();
    void testInvalidEventData();
    void testEventDataCompression();
    void testEventConcatenationWithCompression();
    void testInvalidCompressedData();
    
private:
    int code;
    MWTime time;
    Datum data;
    
};


END_NAMESPACE_MW


#endif /* MWK2FileTests_hpp */
