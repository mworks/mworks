/*
 *  ScopedVariableTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy on 6/11/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "ScopedVariableTest.h"
#include "Experiment.h"
#include "SequentialSelection.h"
#include "EventBuffer.h"
using namespace mw;


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ScopedVariableTestFixture, "Unit Test" );

void ScopedVariableTestFixture::setUp(){
	FullCoreEnvironmentTestFixture::setUp();
	// Create a variable registry
	reg = shared_ptr<VariableRegistry>(new VariableRegistry(GlobalBufferManager));
	
	// create an experiment (the "environment")
	GlobalCurrentExperiment = shared_ptr<Experiment>(new Experiment(reg));
	
	exp = GlobalCurrentExperiment;
	env = dynamic_pointer_cast<ScopedVariableEnvironment, Experiment>(exp);

	c1 = shared_ptr<ConstantVariable>(new ConstantVariable(1L));
	c2 = shared_ptr<ConstantVariable>(new ConstantVariable(2L));

	
	
	v1 =  reg->createScopedVariable( env,
										new VariableProperties(
											new Data(0L), "test1",
											"Test",
											"Test",
											M_NEVER, M_WHEN_CHANGED,
											true, false,
											M_CONTINUOUS_INFINITE,""));
	
	v2 =  reg->createScopedVariable( env,
										new VariableProperties(
											new Data(0L), "test2",
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

	context1 = env->createNewContext();
	context2 = env->createNewContext();
	
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
	
	context1 = env->createNewContext();
	context2 = env->createNewContext();
	
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

void ScopedVariableTestFixture::invitroTest(){
//
//	// HERE: Experiment is set up in setUp()
//	// In MW: XMLParser.pl instantiates the experiment
//	
//	
//	// HERE: Variables are set up in setUp()
//	// In MW: Variables are parsed in XMLParser_Variables.pm
//	
//	
//	// (aside: a red-herring context for later)
//	context1 = exp->createNewContext();
//	// set the values in each context
//	context1->set(v1->getContextIndex(), -10L);
//	context1->set(v2->getContextIndex(), -20L);
//
//	
//	// This gets called in XMLparser.pl, after variables are parsed
//	exp->createVariableContexts();
//	
//	// This happens in the "instantiate" parser pass
//	// create a protocol & a block subordinate to it
//	shared_ptr<mw::Protocol> prot(new mw::Protocol());
//	shared_ptr<Selection> psel(new SequentialSelection(1));
//	prot->attachSelection(psel);
//	
//	shared_ptr<Block> block(new Block());
//	shared_ptr<Selection> bsel(new SequentialSelection(1));
//	prot->attachSelection(bsel);
//	
//	// This part doesn't actually happen in a real experiment;
//	// it is here as a test:
//	// Set the values in each context
//	prot->setValue(v1->getContextIndex(), 1L);
//	prot->setValue(v2->getContextIndex(), 2L);
//	block->setValue(v1->getContextIndex(), 3L);
//	block->setValue(v2->getContextIndex(), 4L);
//	
//	
//	// In the real system, we'l actually be working with instances, which have
//	// their own contexts
//	// This happens in the "reference" parser pass
//	shared_ptr<State> prot_inst(prot->getStateInstance());
//	shared_ptr<State> block_inst(block->getStateInstance());
//
//	// Set the values in each context
//	// this happens in parseVariableAssignments, in XMLParser_states.pm
//	// which is called from ...
//	prot_inst->setValue(v1->getContextIndex(), 10L);
//	prot_inst->setValue(v2->getContextIndex(), 20L);
//
//	block_inst->setValue(v1->getContextIndex(), 30L);
//	// purposely not setting v2 in the block context.  It should inherit from
//	// the protocol's context
//	
//	
//	
//	// Do quick test of the contexts in isolation
//	// This does not happen in the real system
//	exp->setCurrentContext(prot->getLocalScopedVariableContext());
//	CPPUNIT_ASSERT( (long)(*v1) == 1L );
//	CPPUNIT_ASSERT( (long)(*v2) == 2L );
//	
//	// test the instances in isolation too
//	// This does not happen in the real system
//	exp->setCurrentContext(prot_inst->getLocalScopedVariableContext());
//	CPPUNIT_ASSERT( (long)(*v1) == 10L );
//	CPPUNIT_ASSERT( (long)(*v2) == 20L );
//	
//	
//	// Now do a more "realistic" test
//	
//	// Connect things up
//	// This happens in the "Connect" parser pass
//	prot->addChild(parameters, registry, block_inst);
//	exp->addChild(parameters, registry, prot_inst);
//	
//	// A quick check to see that things are okay
//	// This does not happen in the real system
//	exp->setCurrentContext(exp->getLocalScopedVariableContext());
//	CPPUNIT_ASSERT( (long)(*v1) == 0L ); // Default values should be zero
//	CPPUNIT_ASSERT( (long)(*v2) == 0L );
//	
//	// clear the current context to something else
//	// so we'll know if things are getting set correctly
//	// This does not happen in the real system
//	exp->setCurrentContext(context1);
//	
//	
//	// Now the realism...
//	// Manually push through state system
//	// The state system calls this when the experiment is the first state
//	exp->action();
//	
//	// (checks)
//	CPPUNIT_ASSERT( (long)(*v1) == 0L );
//	CPPUNIT_ASSERT( (long)(*v2) == 0L );
//	
//	// The state system then gets the next state
//	shared_ptr<State> next_state(exp->next());
//	
//	// next one should be the protocol (check)
//	CPPUNIT_ASSERT(prot_inst.get() == next_state.get());
//	
//	// This state's action is called, orchestrating the shift in contexts
//	// (among other things)
//	next_state->action();
//	
//	// (checks)
//	CPPUNIT_ASSERT( (long)(*v1) == 10L );
//	CPPUNIT_ASSERT( (long)(*v2) == 20L );
//	
//	
//	// Then the state system moves on
//	next_state = shared_ptr<State>(next_state->next());
//	
//	// next one should be the block (check)
//	CPPUNIT_ASSERT(block_inst.get() == next_state.get());
//	
//	// the block's action is called; variable contexts shift
//	next_state->action();
//
//	// (checks)
//	CPPUNIT_ASSERT( (long)(*v1) == 30L );
//	CPPUNIT_ASSERT( (long)(*v2) == 20L );  // this one is inherited
}


