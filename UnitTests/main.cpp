/*
 *  main.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


//#include "MonkeyWorksCoreTest/GenericDataTest.h"
//#include "MonkeyWorksCoreTest/SchedulerTest.h"
//#include "MonkeyWorksCoreTest/ClockTest.h"

#include "cppunit/ui/text/TestRunner.h"
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/HelperMacros.h>

// test runner
int main(int argc, char *argv[]) {
  
  CppUnit::TestFactoryRegistry &registry = 
    CppUnit::TestFactoryRegistry::getRegistry("Unit Test");
  
  CppUnit::TextUi::TestRunner runner;
  CppUnit::XmlOutputter* outputter =
    new CppUnit::XmlOutputter(&runner.result(), std::cout, std::string("MonkeyWorksTestRunner Output"));
  runner.setOutputter(outputter);
  
  runner.addTest( registry.makeTest() );
  //runner.addTest( GenericDataTestFixture::suite() ); //include this commected out 
  bool returnval = runner.run( "", false );    // Run all tests
  
  // return !returnval because it is 1 on success
  return !returnval;
}

