/*
 *  PythonBindingsTest.h
 *  DataFileIndexer
 *
 *  Created by David Cox on 12/8/09.
 *  Copyright 2009 Harvard University. All rights reserved.
 *
 */

#ifndef PYTHON_BINDINGS_TEST_H_
#define PYTHON_BINDINGS_TEST_H_

#include "PythonDataBindingsHelpers.h"
#define BOOST_TEST_MODULE PythonBindingsTest
#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_CASE( basic_indexer_test )
{
    PythonDataFile test_object( "/Users/davidcox/Desktop/test.mwk" );
    test_object.open();
    
    std::vector<EventWrapper> events = test_object.fetch_all_events();
    
    int nevents = events.size();
    BOOST_CHECK(nevents > 0);
    
    std::cerr << "nevents = " << nevents << std::endl;
    
    test_object.close();
}

BOOST_AUTO_TEST_CASE( basic_stream_test )
{
    PythonDataStream test_object( "ldobinary:file:///Users/davidcox/Documents/MWorks/Data/ettest1.mwk/ettest1.mwk" );
    test_object.open();
    
    shared_ptr<EventWrapper> event;
    int nevents = 0;
    while(!(event = test_object.read_event())->empty()){
        nevents++;
    }
    
    BOOST_CHECK(nevents > 0);
    
    std::cerr << "nevents = " << nevents << std::endl;
    
    test_object.close();
}





#endif
