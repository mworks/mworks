//
//  ZeroMQSocketTests.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/2/16.
//
//

#ifndef ZeroMQSocketTests_hpp
#define ZeroMQSocketTests_hpp

#include "MessageTestFixture.hpp"


BEGIN_NAMESPACE_MW


class ZeroMQSocketTests : public MessageTestFixture {
    
    CPPUNIT_TEST_SUITE( ZeroMQSocketTests );
    
    CPPUNIT_TEST( testBadType );
    CPPUNIT_TEST( testGetLastEndpoint );
    CPPUNIT_TEST( testSetOptionFailure );
    CPPUNIT_TEST( testBindFailure );
    CPPUNIT_TEST( testUnbindFailure );
    CPPUNIT_TEST( testConnectFailure );
    CPPUNIT_TEST( testDisconnectFailure );
    CPPUNIT_TEST( testBasicSendReceive );
    CPPUNIT_TEST( testMultipartSendReceive );
    CPPUNIT_TEST( testEventSendReceive );
    CPPUNIT_TEST( testIPV6 );
    
    CPPUNIT_TEST_SUITE_END();
    
public:
    void testBadType();
    void testGetLastEndpoint();
    void testSetOptionFailure();
    void testBindFailure();
    void testUnbindFailure();
    void testConnectFailure();
    void testDisconnectFailure();
    void testBasicSendReceive();
    void testMultipartSendReceive();
    void testEventSendReceive();
    void testIPV6();
    
};


END_NAMESPACE_MW


#endif /* ZeroMQSocketTests_hpp */



























