//
//  MessagePackAdaptorTests.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 7/29/16.
//
//

#include "MessagePackAdaptorTests.hpp"

#include <boost/static_assert.hpp>


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( MessagePackAdaptorTests, "Unit Test" );


void MessagePackAdaptorTests::testUndefined() {
    pack(Datum());
    auto o = unpack();
    CPPUNIT_ASSERT_EQUAL( msgpack::type::NIL, o.type );
    Datum d = o.as<Datum>();
    CPPUNIT_ASSERT( d.isUndefined() );
}


void MessagePackAdaptorTests::testBoolean() {
    // True
    {
        pack(Datum(true));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::BOOLEAN, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isBool() );
        CPPUNIT_ASSERT_EQUAL( true, d.getBool() );
    }
    
    // False
    {
        pack(Datum(false));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::BOOLEAN, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isBool() );
        CPPUNIT_ASSERT_EQUAL( false, d.getBool() );
    }
}


void MessagePackAdaptorTests::testInteger() {
    constexpr auto llmin = std::numeric_limits<long long>::min();
    BOOST_STATIC_ASSERT(llmin < 0);  // Sanity check
    constexpr auto llmax = std::numeric_limits<long long>::max();
    BOOST_STATIC_ASSERT(llmax > 0);  // Sanity check
    
    // Negative
    {
        pack(Datum(llmin));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::NEGATIVE_INTEGER, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( llmin, d.getInteger() );
    }
    
    // Zero
    {
        pack(Datum(0));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::POSITIVE_INTEGER, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( 0LL, d.getInteger() );
    }
    
    // Positive
    {
        pack(Datum(llmax));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::POSITIVE_INTEGER, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isInteger() );
        CPPUNIT_ASSERT_EQUAL( llmax, d.getInteger() );
    }
    
    // Out of bounds
    {
        pack(static_cast<unsigned long long>(llmax) + 1ULL);
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::POSITIVE_INTEGER, o.type );
        CPPUNIT_ASSERT_THROW( o.as<Datum>(), msgpack::type_error );
    }
}


void MessagePackAdaptorTests::testFloat() {
    // Negative
    {
        pack(Datum(-1.5));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::FLOAT, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isFloat() );
        CPPUNIT_ASSERT_EQUAL( -1.5, d.getFloat() );
    }
    
    // Zero
    {
        pack(Datum(0.0));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::FLOAT, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isFloat() );
        CPPUNIT_ASSERT_EQUAL( 0.0, d.getFloat() );
    }
    
    // Positive
    {
        pack(Datum(2.5));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::FLOAT, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isFloat() );
        CPPUNIT_ASSERT_EQUAL( 2.5, d.getFloat() );
    }
}


void MessagePackAdaptorTests::testString() {
    // Empty
    {
        pack(Datum(""));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::STR, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isString() );
        CPPUNIT_ASSERT( d.stringIsCString() );
        CPPUNIT_ASSERT_EQUAL( std::string(), d.getString() );
    }
    
    // Text
    {
        pack(Datum("abc 123"));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::STR, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isString() );
        CPPUNIT_ASSERT( d.stringIsCString() );
        CPPUNIT_ASSERT_EQUAL( std::string("abc 123"), d.getString() );
    }
    
    // Data
    {
        const std::string value("\1\0\2\0\3\0", 6);
        pack(Datum(value));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::BIN, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isString() );
        CPPUNIT_ASSERT( !d.stringIsCString() );
        CPPUNIT_ASSERT_EQUAL( value, d.getString() );
    }
}


void MessagePackAdaptorTests::testList() {
    // Empty
    {
        pack(Datum(Datum::list_value_type()));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::ARRAY, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isList() );
        CPPUNIT_ASSERT( d.getList().empty() );
    }
    
    // Non-empty
    {
        const Datum::list_value_type value = { Datum(true), Datum(1.5), Datum("foo") };
        pack(value);
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::ARRAY, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isList() );
        CPPUNIT_ASSERT( value == d.getList() );
    }
}


void MessagePackAdaptorTests::testDictionary() {
    // Empty
    {
        pack(Datum(Datum::dict_value_type()));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::MAP, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isDictionary() );
        CPPUNIT_ASSERT( d.getDict().empty() );
    }
    
    // Non-empty
    {
        const Datum::dict_value_type value = { { Datum("foo"), Datum(1.5) }, { Datum(2), Datum(false) } };
        pack(Datum(value));
        auto o = unpack();
        CPPUNIT_ASSERT_EQUAL( msgpack::type::MAP, o.type );
        auto d = o.as<Datum>();
        CPPUNIT_ASSERT( d.isDictionary() );
        CPPUNIT_ASSERT( value == d.getDict() );
    }
}


void MessagePackAdaptorTests::testInvalidType() {
    msgpack::packer<msgpack::sbuffer> packer(buffer);
    const std::string value("abc 123");
    packer.pack_ext(value.size(), 12);
    packer.pack_ext_body(value.data(), value.size());
    auto o = unpack();
    CPPUNIT_ASSERT_EQUAL( msgpack::type::EXT, o.type );
    CPPUNIT_ASSERT_THROW( o.as<Datum>(), msgpack::type_error );
}


END_NAMESPACE_MW



























