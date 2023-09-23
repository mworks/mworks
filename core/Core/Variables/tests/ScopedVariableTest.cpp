/*
 *  ScopedVariableTest.cpp
 *  MWorksCore
 *
 *  Created by Ben Kennedy on 6/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ScopedVariableTest.h"
#include "Experiment.h"
#include "SequentialSelection.h"
#include "EventBuffer.h"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ScopedVariableTestFixture, "Unit Test" );

void ScopedVariableTestFixture::setUp(){
	FullCoreEnvironmentTestFixture::setUp();
	// Create a variable registry
	reg = shared_ptr<VariableRegistry>(new VariableRegistry(global_outgoing_event_buffer));
	
	// create an experiment (the "environment")
	GlobalCurrentExperiment = shared_ptr<Experiment>(new Experiment(reg));
	
	exp = GlobalCurrentExperiment;
	env = boost::dynamic_pointer_cast<ScopedVariableEnvironment, Experiment>(exp);

	c1 = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	c2 = shared_ptr<ConstantVariable>(new ConstantVariable(2L));

	
	
	v1 =  reg->createScopedVariable( env,
										    VariableProperties(
											Datum(0L), "test1",
											"Test",
											"Test",
											M_NEVER, M_WHEN_CHANGED,
											true, false,
											M_CONTINUOUS_INFINITE,""));
	
	v2 =  reg->createScopedVariable( env,
										    VariableProperties(
											Datum(0L), "test2",
											"Test",
											"Test",
											M_NEVER, M_WHEN_CHANGED,
											true, false,
											M_CONTINUOUS_INFINITE,""));
	

}

void ScopedVariableTestFixture::tearDown(){
	FullCoreEnvironmentTestFixture::tearDown();
}

void ScopedVariableTestFixture::test1(){

	context1 = boost::make_shared<ScopedVariableContext>();
	context2 = boost::make_shared<ScopedVariableContext>();
	
	context1->set(v1->getContextIndex(), 10L);
	context2->set(v1->getContextIndex(), 20L);

	context1->set(v2->getContextIndex(), 30L);
	context2->set(v2->getContextIndex(), 40L);
	
	env->setCurrentContext(context1);
	
	CPPUNIT_ASSERT( (long)(*v1) == 10L );
	CPPUNIT_ASSERT( (long)(*v2) == 30L );
	
	env->setCurrentContext(context2);
	
	CPPUNIT_ASSERT( (long)(*v1) == 20L );
	CPPUNIT_ASSERT( (long)(*v2) == 40L );
}

void ScopedVariableTestFixture::inheritedContextTest(){
	
	context1 = boost::make_shared<ScopedVariableContext>();
	context2 = boost::make_shared<ScopedVariableContext>();
	
	context1->set(v1->getContextIndex(), 10L);
	context2->set(v1->getContextIndex(), 20L);

	context1->set(v2->getContextIndex(), 30L);


	context2->inheritFrom(context1);
	
	env->setCurrentContext(context1);
	
	CPPUNIT_ASSERT( (long)(*v1) == 10L );
	CPPUNIT_ASSERT( (long)(*v2) == 30L );
	
	env->setCurrentContext(context2);
	
	CPPUNIT_ASSERT( (long)(*v1) == 20L );
	CPPUNIT_ASSERT( (long)(*v2) == 30L );
}


END_NAMESPACE_MW
