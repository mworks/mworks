/*
 *  MapTest.h
 *  MWorksCore
 *
 *  Created by Christopher Stawarz on 3/24/11.
 *  Copyright 2011 MIT. All rights reserved.
 *
 */

#ifndef _MW_MAP_TEST_H
#define _MW_MAP_TEST_H

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksCore/Map.h"


BEGIN_NAMESPACE_MW


class MapTestFixture : public CppUnit::TestFixture {
    
	CPPUNIT_TEST_SUITE( MapTestFixture );
	CPPUNIT_TEST( testOperators );
	CPPUNIT_TEST_SUITE_END();
    
public:
    void testOperators();

};


END_NAMESPACE_MW


#endif
