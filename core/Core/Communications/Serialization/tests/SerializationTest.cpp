/*
 *  SerializationTest.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 9/28/08.
 *  Copyright 2008 The Rowland Institute at Harvard. All rights reserved.
 *
 */

#include "SerializationTest.h"

#include "Serialization.h"

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

using std::ios_base;
using std::stringstream;
using std::istringstream;
using std::ostringstream;


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SerializationTestFixture, "Unit Test" );


void SerializationTestFixture::setUp(){
	shared_ptr <Clock> new_clock = shared_ptr<Clock>(new Clock(0));
	Clock::registerInstance(new_clock);
}

void SerializationTestFixture::tearDown(){
	Clock::destroy();
}

void SerializationTestFixture::testEventSerialization(){

    Datum the_data((double)4.0);
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
    Datum the_regurgitated_data = event.getData();
    
    CPPUNIT_ASSERT(the_data.getDataType() == the_regurgitated_data.getDataType());
    CPPUNIT_ASSERT(the_data.getFloat() == the_regurgitated_data.getFloat());

}

void SerializationTestFixture::testScarabSerialization(){

    Datum the_data((double)4.0);
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

void SerializationTestFixture::testStringDatumSerialization(){
    Datum the_data("blah");
    
    Event evt(0, the_data);
    
    stringstream output_stream_(ios_base::binary |  ios_base::out | ios_base::in);
    
    boost::archive::text_oarchive serialized_archive(output_stream_);
    serialized_archive << evt;    
    string data = output_stream_.str();
    
    stringstream input_stream(ios_base::binary |  ios_base::out | ios_base::in);
    input_stream.str(data);
    boost::archive::text_iarchive serialized_archive2(input_stream);
    
    Event evt2;
    serialized_archive2 >> evt2;
    
    CPPUNIT_ASSERT(evt.getEventCode() == evt2.getEventCode());
    Datum the_regurgitated_data = evt2.getData();
    
    CPPUNIT_ASSERT(the_data.getDataType() == the_regurgitated_data.getDataType());
    
    string a(the_regurgitated_data.getString());
    string b(the_data.getString());
    CPPUNIT_ASSERT(a == b);

}

void SerializationTestFixture::testDictSerialization(){
    
    Datum the_data(M_DICTIONARY,2);
    Datum four(4L);
    Datum five(5L);
    //the_data.addElement("blah1", four);
    //the_data.addElement("blah2", five);
    the_data.addElement(four, four);
    the_data.addElement(five, five);
    
    
    Event evt(0, the_data);
    
    stringstream output_stream_(ios_base::binary |  ios_base::out | ios_base::in);
    
    boost::archive::text_oarchive serialized_archive(output_stream_);
    serialized_archive << evt;    
    string data = output_stream_.str();
    
    stringstream input_stream(ios_base::binary |  ios_base::out | ios_base::in);
    input_stream.str(data);
    boost::archive::text_iarchive serialized_archive2(input_stream);
    
    Event evt2;
    serialized_archive2 >> evt2;
    
    CPPUNIT_ASSERT(evt.getEventCode() == evt2.getEventCode());
    Datum the_regurgitated_data = evt2.getData();
    
    CPPUNIT_ASSERT(the_data.getDataType() == the_regurgitated_data.getDataType());
    
    CPPUNIT_ASSERT(the_regurgitated_data.getElement(four).getInteger() == 4L);
    CPPUNIT_ASSERT(the_regurgitated_data.getElement(five).getInteger() == 5L);
}


void SerializationTestFixture::testListSerialization(){
    
    Datum the_data(M_LIST,2);
    Datum four(4L);
    Datum five(5L);
    //the_data.addElement("blah1", four);
    //the_data.addElement("blah2", five);
    the_data.addElement(four);
    the_data.addElement(five);
    
    
    Event evt(0, the_data);
    
    stringstream output_stream_(ios_base::binary |  ios_base::out | ios_base::in);
    
    boost::archive::text_oarchive serialized_archive(output_stream_);
    serialized_archive << evt;    
    string data = output_stream_.str();
    
    stringstream input_stream(ios_base::binary |  ios_base::out | ios_base::in);
    input_stream.str(data);
    boost::archive::text_iarchive serialized_archive2(input_stream);
    
    Event evt2;
    serialized_archive2 >> evt2;
    
    CPPUNIT_ASSERT(evt.getEventCode() == evt2.getEventCode());
    Datum the_regurgitated_data = evt2.getData();
    
    CPPUNIT_ASSERT(the_data.getDataType() == the_regurgitated_data.getDataType());
    
    CPPUNIT_ASSERT(the_regurgitated_data.getElement(0).getInteger() == 4L);
    CPPUNIT_ASSERT(the_regurgitated_data.getElement(1).getInteger() == 5L);
}


END_NAMESPACE_MW
