#ifndef	SCOPED_VARIABLE_TEST_H_
#define SCOPED_VARIABLE_TEST_H_

/*
 *  ScopedVariableTest.h
 *  MWorksCore
 *
 *  Created by Ben Kennedy on 6/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksCore/ScarabServices.h"
#include "MWorksCore/VariableRegistry.h"

#include "MWorksCore/ScopedVariableEnvironment.h"
#include "MWorksCore/Experiment.h"
#include "FullCoreEnvironmentTest.h"


BEGIN_NAMESPACE_MW


class ScopedVariableTestFixture : public FullCoreEnvironmentTestFixture {
	
	
	CPPUNIT_TEST_SUITE( ScopedVariableTestFixture );

	CPPUNIT_TEST( test1 );
	CPPUNIT_TEST( inheritedContextTest );
	CPPUNIT_TEST( invitroTest );

	CPPUNIT_TEST_SUITE_END();
	
	
private:
	
	shared_ptr<VariableRegistry> reg;
	
	shared_ptr<ScopedVariableEnvironment> env;
	shared_ptr<Experiment> exp;
	shared_ptr<ScopedVariableContext> context1;
	shared_ptr<ScopedVariableContext> context2;
	
	shared_ptr<ScopedVariable> v1;
	shared_ptr<ScopedVariable> v2;
	shared_ptr<ConstantVariable> c1;
	shared_ptr<ConstantVariable> c2;
	
public:
		
		
	void setUp();
	void tearDown();
	
	void test1();
	void inheritedContextTest();
	void invitroTest();
	
	
};


END_NAMESPACE_MW


#endif