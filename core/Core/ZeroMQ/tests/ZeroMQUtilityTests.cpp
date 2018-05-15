//
//  ZeroMQUtilityTests.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/16.
//
//

#include "ZeroMQUtilityTests.hpp"

#include "MWorksCore/ZeroMQUtilities.hpp"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ZeroMQUtilityTests, "Unit Test" );


void ZeroMQUtilityTests::testFormatTCPEndpoint() {
    CPPUNIT_ASSERT_EQUAL( std::string("tcp://www.mit.edu:1234"), zeromq::formatTCPEndpoint("www.mit.edu", 1234) );
}


void ZeroMQUtilityTests::testGetHostname() {
    std::string hostname;
    CPPUNIT_ASSERT( zeromq::getHostname(hostname) );
    CPPUNIT_ASSERT( !hostname.empty() );
}


void ZeroMQUtilityTests::testResolveHostname() {
    std::string address;
    
    // Local numeric
    CPPUNIT_ASSERT( zeromq::resolveHostname("127.0.0.1", address) );
    CPPUNIT_ASSERT_EQUAL( std::string("127.0.0.1"), address );
    
    // Local numeric (IPv6)
    CPPUNIT_ASSERT( zeromq::resolveHostname("::1", address) );
    CPPUNIT_ASSERT_EQUAL( std::string("::1"), address );
    
    // Local name
    CPPUNIT_ASSERT( zeromq::resolveHostname("localhost", address) );
    CPPUNIT_ASSERT_EQUAL( std::string("::1"), address );
    
    // Remote numeric
    CPPUNIT_ASSERT( zeromq::resolveHostname("8.8.4.4", address) );
    CPPUNIT_ASSERT_EQUAL( std::string("8.8.4.4"), address );
    
    // Remote name
    CPPUNIT_ASSERT( zeromq::resolveHostname("google-public-dns-b.google.com", address) );
    CPPUNIT_ASSERT_EQUAL( std::string("8.8.4.4"), address );
    
    // Unknown name
    CPPUNIT_ASSERT( !zeromq::resolveHostname("not.a.valid.hostname", address) );
    assertError("ERROR: Cannot resolve hostname \"not.a.valid.hostname\": nodename nor servname provided, or not known");
}


END_NAMESPACE_MW



























