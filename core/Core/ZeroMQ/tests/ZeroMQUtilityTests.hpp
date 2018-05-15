//
//  ZeroMQUtilityTests.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 8/3/16.
//
//

#ifndef ZeroMQUtilityTests_hpp
#define ZeroMQUtilityTests_hpp

#include "MessageTestFixture.hpp"


BEGIN_NAMESPACE_MW


class ZeroMQUtilityTests : public MessageTestFixture {
    
    CPPUNIT_TEST_SUITE( ZeroMQUtilityTests );
    
    CPPUNIT_TEST( testFormatTCPEndpoint );
    CPPUNIT_TEST( testGetHostname );
    CPPUNIT_TEST( testResolveHostname );
    
    CPPUNIT_TEST_SUITE_END();
    
public:
    void testFormatTCPEndpoint();
    void testGetHostname();
    void testResolveHostname();
    
};


END_NAMESPACE_MW


#endif /* ZeroMQUtilityTests_hpp */
