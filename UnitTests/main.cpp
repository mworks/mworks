/*
 *  main.cpp
 *  MWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


//#include "MWorksCoreTest/GenericDataTest.h"
//#include "MWorksCoreTest/SchedulerTest.h"
//#include "MWorksCoreTest/ClockTest.h"

#include <cppunit/TextTestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestResult.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/HelperMacros.h>

// test runner
int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " results_file [test_name ...]" << std::endl;
        return 2;
    }

    CppUnit::TextTestRunner runner;

    std::ofstream outfile( argv[1] );
    CppUnit::XmlOutputter *outputter = new CppUnit::XmlOutputter(&runner.result(), outfile);
    runner.setOutputter(outputter);
    
    CppUnit::BriefTestProgressListener listener;
    runner.eventManager().addListener( &listener );
    
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry("Unit Test");
    runner.addTest( registry.makeTest() );
    
    bool returnval = true;
    if (argc > 2) {
        // Run specified tests
        for (int i = 2; i < argc; i++) {
            returnval = runner.run( argv[i], false, false, false ) && returnval;
        }
    } else {
        // Run all tests
        returnval = runner.run( "", false, false, false );
    }
    
    outputter->write();
    outfile.close();
    
    // return !returnval because it is 1 on success
    return !returnval;
}

