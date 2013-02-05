#ifndef	TRIGGER_TEST_H_
#define TRIGGER_TEST_H_

/*
 *  TriggerTest.h
 *  MWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MWorksCore/Trigger.h"
#include "MWorksCore/Experiment.h"
#include "MWorksCore/VariableRegistry.h"
#include "MWorksCore/ConstantVariable.h"
#include "MWorksCore/EventBuffer.h"


BEGIN_NAMESPACE_MW


class TriggerTestFixture : public CppUnit::TestFixture {
	
	
	CPPUNIT_TEST_SUITE( TriggerTestFixture );
	
	CPPUNIT_TEST( testSquareTrigger );
	
	CPPUNIT_TEST_SUITE_END();
	
	
private:
		
		Trigger *trig;
	shared_ptr<VariableRegistry> reg;
	Experiment *exp;
	shared_ptr<Variable> centerX, centerY, width;
	shared_ptr<Variable> watchVar1;
	shared_ptr<Variable> watchVar2;
	shared_ptr<Variable> triggerVar;
	
public:
		
		void setUp(){
			
			global_outgoing_event_buffer = shared_ptr<EventBuffer>(new EventBuffer());
			reg = shared_ptr<VariableRegistry>(new VariableRegistry(global_outgoing_event_buffer));
			
			
			
			centerX = shared_ptr<Variable>(new ConstantVariable(Datum(5L)));
			centerY = shared_ptr<Variable>(new ConstantVariable(Datum(5L)));
			width = shared_ptr<Variable>(new ConstantVariable(Datum(2L)));
			
		 Datum defaultValue(0.0);
			
			VariableProperties props1(&defaultValue, "test1",
									   "Test test",
									   "Testy test test test",
									   M_NEVER, M_WHEN_CHANGED,
									   true, false, 
									   M_CONTINUOUS_INFINITE,"");
				watchVar1 =
				reg->createGlobalVariable(&props1);
			
			VariableProperties props2(&defaultValue, "test2",
									   "Test test",
									   "Testy test test test",
									   M_NEVER, M_WHEN_CHANGED,
									   true, false, 
									   M_CONTINUOUS_INFINITE,"");
			watchVar2 = reg->createGlobalVariable(&props2);
			
		 Datum defaultBool((bool)false);
			
			VariableProperties triggerProps(&defaultBool, "triggerVar",
											 "Test test",
											 "Testy test test test",
											 M_NEVER, M_WHEN_CHANGED,
											 true, false,
											 M_CONTINUOUS_INFINITE,"");
			triggerVar = reg->createGlobalVariable(&triggerProps);
			
			shared_ptr<Experiment> exp(new Experiment(reg));
			exp->createVariableContexts(); // DDC added
			GlobalCurrentExperiment = exp;
		}
	
	void tearDown(){
		
		
		
		//delete reg;
		GlobalCurrentExperiment = shared_ptr<Experiment>();
		
	}
	
	void testSquareTrigger(){
		trig = new SquareRegionTrigger(centerX,
										centerY,
										width,
										watchVar1,
										watchVar2,
										triggerVar);
		
		CPPUNIT_ASSERT( (bool)(*triggerVar) == false );
		
		(*watchVar1) =  -5.0;
		(*watchVar2) =  -5.0;
		
		// still false
		CPPUNIT_ASSERT( (bool)(*triggerVar) == false );
		
		(*watchVar1) = 5.0;
		(*watchVar2) = 5.0;
		
		// should go true now
		CPPUNIT_ASSERT( (bool)(*triggerVar) == true );
		
		(*watchVar1) = 5.5;
		(*watchVar2) = 5.5;
		
		// should stay true
		CPPUNIT_ASSERT( (bool)(*triggerVar) == true );
		
		(*watchVar1) = 20.0;
		
		// should go false;
		CPPUNIT_ASSERT( (bool)(*triggerVar) == false );
		
		(*watchVar2) = 20.0;
		
		// should stay false;
		CPPUNIT_ASSERT( (bool)(*triggerVar) == false );
		
		(*watchVar2) = 5.5;
		
		// should stay false;
		CPPUNIT_ASSERT( (bool)(*triggerVar) == false );
		
		(*watchVar1) = 5.0;
		
		// should go true;
		CPPUNIT_ASSERT( (bool)(*triggerVar) == true );
		
		
	}
	
	
};


END_NAMESPACE_MW


#endif

