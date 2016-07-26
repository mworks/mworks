//
//  MessagePackAdaptorTests.hpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/29/16.
//
//

#ifndef MessagePackAdaptorTests_hpp
#define MessagePackAdaptorTests_hpp

#include "MessageTestFixture.hpp"

#include "MWorksCore/MessagePackAdaptors.hpp"


BEGIN_NAMESPACE_MW


class MessagePackAdaptorTests : public MessageTestFixture {
    
    CPPUNIT_TEST_SUITE( MessagePackAdaptorTests );
    
    CPPUNIT_TEST( testUndefined );
    CPPUNIT_TEST( testBoolean );
    CPPUNIT_TEST( testInteger );
    CPPUNIT_TEST( testFloat );
    CPPUNIT_TEST( testString );
    CPPUNIT_TEST( testList );
    CPPUNIT_TEST( testDictionary );
    CPPUNIT_TEST( testInvalidType );
    
    CPPUNIT_TEST_SUITE_END();
    
public:
    void testUndefined();
    void testBoolean();
    void testInteger();
    void testFloat();
    void testString();
    void testList();
    void testDictionary();
    void testInvalidType();
    
private:
    template<typename T>
    void pack(T&& value) {
        buffer.clear();
        msgpack::pack(buffer, std::forward<T>(value));
    }
    
    msgpack::object unpack() {
        handle = msgpack::unpack(buffer.data(), buffer.size());
        return handle.get();
    }
    
    msgpack::sbuffer buffer;
    msgpack::object_handle handle;
    
};


END_NAMESPACE_MW


#endif /* MessagePackAdaptorTests_hpp */



























