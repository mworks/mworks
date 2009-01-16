/*
 *  ScarabEventTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 4/13/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "ScarabEventTest.h"
#include "EventBuffer.h"
using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ScarabEventTestFixture, "Unit Test" );

void ScarabEventTestFixture::setUp(){
	shared_ptr <Clock> shared_clock = Clock::instance(false);
	CPPUNIT_ASSERT(shared_clock == 0);
	if(shared_clock == 0) {
		shared_ptr <Clock> new_clock = shared_ptr<Clock>(new Clock(0));
		Clock::registerInstance(new_clock);
	}
	shared_clock = Clock::instance(false);
	CPPUNIT_ASSERT(shared_clock != 0);
	
	
	reg = new VariableRegistry(GlobalBufferManager);
	
	Data defaultBool((bool)false);
	VariableProperties props(&defaultBool, 
						"test",
						"Test test",
						"Testy test test test",
						M_NEVER, 
						M_WHEN_CHANGED,
						true, 
						false,
						M_CONTINUOUS_INFINITE,
							  "");
	
	reg->createGlobalVariable(&props);
}

void ScarabEventTestFixture::tearDown(){
	delete reg;
	
	Clock::destroy();
	
	shared_ptr<Clock>shared_clock = Clock::instance(false);
	CPPUNIT_ASSERT(shared_clock == 0);
}

void ScarabEventTestFixture::testToFromScarabDatum(){
	fprintf(stderr, "Running ScarabEventTestFixture::testToFromScarabDatum()\n");
	
	Data codec(reg->getCodec());
	Event test_event(RESERVED_CODEC_CODE, codec.getScarabDatum());
	
	ScarabDatum *serialized = test_event.toScarabDatum();
	
	CPPUNIT_ASSERT( serialized != NULL );
	
	shared_ptr<Event> twice_baked = shared_ptr<Event>(new Event(serialized));
	
	CPPUNIT_ASSERT( test_event.getEventCode() == 
					twice_baked->getEventCode() );
	
	CPPUNIT_ASSERT( test_event.getData() == 
					twice_baked->getData() );
	
	ScarabDatum *payload = twice_baked->getData().getScarabDatum();
	
	CPPUNIT_ASSERT( payload != NULL );
	CPPUNIT_ASSERT( payload->type == SCARAB_DICT );			
	
	scarab_free_datum(serialized);
}
