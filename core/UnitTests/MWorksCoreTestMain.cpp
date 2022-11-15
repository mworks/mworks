//
//  MWorksCoreTestMain.cpp
//  MWorksCore
//
//  Created by Christopher Stawarz on 1/25/12.
//  Copyright (c) 2012 MIT. All rights reserved.
//

#include "MWorksCoreTestMain.h"

#include <unistd.h>

#include <random>

#include <cppunit/TextTestRunner.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFailure.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestResult.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/HelperMacros.h>

#include <MWorksCore/MachUtilities.h>


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


static void getTests(CppUnit::Test *testOrSuite, std::vector<CppUnit::Test *> &tests) {
    for (int childIndex = 0; childIndex < testOrSuite->getChildTestCount(); childIndex++) {
        auto childTest = testOrSuite->getChildTestAt(childIndex);
        if (auto childSuite = dynamic_cast<CppUnit::TestSuite *>(childTest)) {
            getTests(childSuite, tests);
        } else {
            tests.push_back(childTest);
        }
    }
}


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
    
    // Set the current thread's priority to TaskPriority::Default, so that the tested code
    // runs at the minimum priority it would likely run at inside MWServer.  This is
    // particularly important for timing-related tests, which may not perform as expected
    // when executed at the system's default thread priority.
    if (!mw::MachThreadSelf("MWorksCoreTestMain").setPriority(mw::TaskPriority::Default)) {
        std::cerr << "Cannot set thread priority" << std::endl;
        return 1;
    }
    
    CppUnit::TextTestRunner runner;
    
    std::ofstream outfile( argv[1] );
    CppUnit::XmlOutputter *outputter = new CppUnit::XmlOutputter(&runner.result(), outfile);
    runner.setOutputter(outputter);
    
    MWorksTestProgressListener listener;
    runner.eventManager().addListener( &listener );
    
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry("Unit Test");
    auto topLevelTest = registry.makeTest();
    
    bool returnval = true;
    if (argc > 2) {
        // Add all tests and test suites to test runner
        runner.addTest(topLevelTest);
        
        // Run specified tests
        for (int i = 2; i < argc; i++) {
            returnval = runner.run( argv[i], false, false, false ) && returnval;
        }
    } else {
        // Collect all tests (but not test suites)
        std::vector<CppUnit::Test *> tests;
        getTests(topLevelTest, tests);
        
        // Randomize test order to help catch unintended dependencies and bad interactions
        // between components
        std::random_device randDev;
        std::mt19937 randGen(randDev());
        std::shuffle(tests.begin(), tests.end(), randGen);
        
        // Add all tests to test runner
        for (auto &test : tests) {
            runner.addTest(test);
        }
        
        // Run all tests
        returnval = runner.run( "", false, false, false );
    }
    
    outputter->write();
    outfile.close();
    
    // return !returnval because it is 1 on success
    return !returnval;
}
