/*
 *  FullCoreEnvironmentTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 9/19/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _FULL_CORE_ENVIRONMENT_TEST_H_
#define _FULL_CORE_ENVIRONMENT_TEST_H_

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksMacros.h"
#include "MWorksCore/TestBedCoreBuilder.h"
#include "MWorksCore/ComponentRegistry.h"


BEGIN_NAMESPACE_MW


class FullCoreEnvironmentTestFixture : public CppUnit::TestFixture {
	
 private:
	
	TestBedCoreBuilder *builder;
	
 public:
	
	void setUp();
	void tearDown();
};


END_NAMESPACE_MW


#endif

