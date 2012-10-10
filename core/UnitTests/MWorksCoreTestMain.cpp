//
//  MWorksCoreTestMain.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/25/12.
//  Copyright (c) 2012 MIT. All rights reserved.
//

#include "MWorksCoreTestMain.h"

#include <unistd.h>

#include <cppunit/TextTestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestResult.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/HelperMacros.h>


class MWorksTestProgressListener : public CppUnit::BriefTestProgressListener {
    
public:
    void addFailure(const CppUnit::TestFailure &failure) {
        CppUnit::BriefTestProgressListener::addFailure(failure);
        
        CppUnit::Exception *exception = failure.thrownException();
        std::cout << std::endl << std::endl;
        
        CppUnit::SourceLine sourceLine = exception->sourceLine();
        if (sourceLine.isValid()) {
            std::cout << "File: " << sourceLine.fileName() << std::endl
                      << "Line: " << sourceLine.lineNumber() << std::endl;
        }
        
        std::cout << "Message: " << exception->what();
    }
    
};


int MWorksCoreTestMain(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " results_file [test_name ...]" << std::endl;
        return 2;
    }
    
    if (NULL != getenv("MWORKS_PAUSE_FOR_DEBUGGER")) {
        std::cerr << "Waiting for debugger to attach...";
        pause();
        std::cerr << " continuing" << std::endl;
    }
    
    CppUnit::TextTestRunner runner;
    
    std::ofstream outfile( argv[1] );
    CppUnit::XmlOutputter *outputter = new CppUnit::XmlOutputter(&runner.result(), outfile);
    runner.setOutputter(outputter);
    
    MWorksTestProgressListener listener;
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
