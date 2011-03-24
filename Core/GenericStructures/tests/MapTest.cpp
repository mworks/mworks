/*
 *  MapTest.cpp
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#include "MapTest.h"

#include <stdexcept>


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( mw::MapTestFixture, "Unit Test" );


BEGIN_NAMESPACE(mw)


void MapTestFixture::testOperators() {
    Map<int> m;
    
    CPPUNIT_ASSERT(m.end() == m.find("a"));
    CPPUNIT_ASSERT(m.end() == m.find("b"));
    CPPUNIT_ASSERT(m.end() == m.find("c"));
    CPPUNIT_ASSERT(m.end() == m.find("d"));
    
    m["a"] = 1;
    m["b"] = 2;
    
    CPPUNIT_ASSERT_EQUAL(1, m["a"]);
    CPPUNIT_ASSERT_EQUAL(2, m["b"]);

    // non-const map should insert non-existent key
    CPPUNIT_ASSERT_EQUAL(0, m["c"]);

    // const map should throw on non-existent key
    CPPUNIT_ASSERT_THROW(static_cast<const Map<int>&>(m)["d"], std::out_of_range);
}


END_NAMESPACE(mw)






















