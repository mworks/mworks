/*
 *  SerializationTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 9/28/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "SerializationTest.h"

#include "Serialization.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SerializationTestFixture, "Unit Test" );


void SerializationTestFixture::setUp(){
	shared_ptr <Clock> new_clock = shared_ptr<Clock>(new Clock(0));
	Clock::registerInstance(new_clock);
}

void SerializationTestFixture::tearDown(){
	Clock::destroy();
}

void SerializationTestFixture::testEventSerialization(){

    Data the_data((double)4.0);
    Event event(1, the_data);
    
    ostringstream output_stream_;
    
    boost::archive::text_oarchive serialized_archive(output_stream_);
    serialized_archive << event;    
    string data = output_stream_.str();

    istringstream input_stream;
    input_stream.str(data);
    boost::archive::text_iarchive serialized_archive2(input_stream);
    
    Event event2;
    
    serialized_archive2 >> event2;
    
    
    CPPUNIT_ASSERT(event.getEventCode() == event.getEventCode());
    Data the_regurgitated_data = event.getData();
    
    CPPUNIT_ASSERT(the_data.getDataType() == the_regurgitated_data.getDataType());
    CPPUNIT_ASSERT(the_data.getFloat() == the_regurgitated_data.getFloat());

}

void SerializationTestFixture::testScarabSerialization(){

    Data the_data((double)4.0);
    ScarabDatum *datum = the_data.getScarabDatum();

    ostringstream output_stream_;
    
    boost::archive::text_oarchive serialized_archive(output_stream_);
    serialized_archive << *datum;    
    string data = output_stream_.str();

    istringstream input_stream;
    input_stream.str(data);
    boost::archive::text_iarchive serialized_archive2(input_stream);
    
    ScarabDatum datum2;
    serialized_archive2 >> datum2;
    
    CPPUNIT_ASSERT(datum->type == datum2.type);
    CPPUNIT_ASSERT(datum->data.floatp == datum2.data.floatp);

}
