/*
 *  VariablePropertiesTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy on 8/28/07.
 *  Copyright 2007 MIT. All rights reserved.
 *
 */

#include "VariablePropertiesTest.h"
#include "EventBuffer.h"
using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( VariablePropertiesTestFixture, "Unit Test" );

void VariablePropertiesTestFixture::setUp(){
	// Create a variable registry
	registry = shared_ptr<VariableRegistry>(new VariableRegistry(GlobalBufferManager));
}

void VariablePropertiesTestFixture::tearDown(){
}

void VariablePropertiesTestFixture::testGroups(){
	
	shared_ptr <Variable> v0 = registry->createGlobalVariable( new VariableProperties(new Data((long)0), 
																						"v0", 
																						"v0", 
																						"the var 0", 
																						M_WHEN_CHANGED,
																						M_WHEN_CHANGED, 
																						true, 
																						false, 
																						M_INTEGER_INFINITE,
																						""));

	std::vector <std::string> groups0 = v0->getProperties()->getGroups();
	CPPUNIT_ASSERT(groups0.size() == 1);	
	CPPUNIT_ASSERT(groups0[0] == "# ALL VARIABLES");
	
	shared_ptr <Variable> v1 = registry->createGlobalVariable( new VariableProperties(new Data((long)0), 
																						"v1", 
																						"v1", 
																						"the var 1", 
																						M_WHEN_CHANGED,
																						M_WHEN_CHANGED, 
																						true, 
																						false, 
																						M_INTEGER_INFINITE,
																						"g1,g2"));

	std::vector <std::string> groups1 = v1->getProperties()->getGroups();
	CPPUNIT_ASSERT(groups1.size() == 3);
	CPPUNIT_ASSERT(groups1[0] == "# ALL VARIABLES");
	CPPUNIT_ASSERT(groups1[1] == "g1");
	CPPUNIT_ASSERT(groups1[2] == "g2");
				   
				   
	
	shared_ptr <Variable> v2 = registry->createGlobalVariable( new VariableProperties(new Data((long)0), 
																						"v2", 
																						"v2", 
																						"the var 2", 
																						M_WHEN_CHANGED,
																						M_WHEN_CHANGED, 
																						true, 
																						false, 
																						M_INTEGER_INFINITE,
																						"g3"));
	
	std::vector <std::string> groups2 = v2->getProperties()->getGroups();
	CPPUNIT_ASSERT(groups2.size() == 2);	
	CPPUNIT_ASSERT(groups2[0] == "# ALL VARIABLES");
	CPPUNIT_ASSERT(groups2[1] == "g3");
	
	shared_ptr <Variable> v2point5 = registry->createGlobalVariable( new VariableProperties(new Data((long)0), 
																						"v2point5", 
																						"v2point5", 
																						"the var 2.5", 
																						M_WHEN_CHANGED,
																						M_WHEN_CHANGED, 
																						true, 
																						false, 
																						M_INTEGER_INFINITE,
																						"  g3  "));
	
	std::vector <std::string> groups2point5 = v2point5->getProperties()->getGroups();
	CPPUNIT_ASSERT(groups2point5.size() == 2);	
	CPPUNIT_ASSERT(groups2point5[0] == "# ALL VARIABLES");
	CPPUNIT_ASSERT(groups2point5[1] == "g3");
	
	shared_ptr <Variable> v3 = registry->createGlobalVariable( new VariableProperties(new Data((long)0), 
																						"v3", 
																						"v3", 
																						"the var 3", 
																						M_WHEN_CHANGED,
																						M_WHEN_CHANGED, 
																						true, 
																						false, 
																						M_INTEGER_INFINITE,
																						"   g2   ,  g3  "));
	
	std::vector <std::string> groups3 = v3->getProperties()->getGroups();
	CPPUNIT_ASSERT(groups3.size() == 3);	
	CPPUNIT_ASSERT(groups3[0] == "# ALL VARIABLES");
	CPPUNIT_ASSERT(groups3[1] == "g2");
	CPPUNIT_ASSERT(groups3[2] == "g3");
	
	shared_ptr <Variable> v3point5 = registry->createGlobalVariable( new VariableProperties(new Data((long)0), 
																						"v3.5", 
																						"v3.5", 
																						"the var 3.5", 
																						M_WHEN_CHANGED,
																						M_WHEN_CHANGED, 
																						true, 
																						false, 
																						M_INTEGER_INFINITE,
																						"   g2   ,    "));
	
	std::vector <std::string> groups3point5 = v3point5->getProperties()->getGroups();
	CPPUNIT_ASSERT(groups3point5.size() == 2);	
	CPPUNIT_ASSERT(groups3point5[0] == "# ALL VARIABLES");
	CPPUNIT_ASSERT(groups3point5[1] == "g2");
	
	shared_ptr <Variable> v4 = registry->createGlobalVariable( new VariableProperties(new Data((long)0), 
																						"v4", 
																						"v4", 
																						"the var 4", 
																						M_WHEN_CHANGED,
																						M_WHEN_CHANGED, 
																						true, 
																						false, 
																						M_INTEGER_INFINITE,
																						"   "));
	
	std::vector <std::string> groups4 = v4->getProperties()->getGroups();
	CPPUNIT_ASSERT(groups4.size() == 1);	
	CPPUNIT_ASSERT(groups4[0] == "# ALL VARIABLES");

	shared_ptr <Variable> v5 = registry->createGlobalVariable( new VariableProperties(new Data((long)0), 
																						"v5", 
																						"v5", 
																						"the var 5", 
																						M_WHEN_CHANGED,
																						M_WHEN_CHANGED, 
																						true, 
																						false, 
																						M_INTEGER_INFINITE,
																						",,g1,,,g2,,"));
	
	std::vector <std::string> groups5 = v5->getProperties()->getGroups();
	CPPUNIT_ASSERT(groups5.size() == 3);	
	CPPUNIT_ASSERT(groups5[0] == "# ALL VARIABLES");
	CPPUNIT_ASSERT(groups5[1] == "g1");
	CPPUNIT_ASSERT(groups5[2] == "g2");
	
}
