/*
 *  RegisteredSingletonTests.h
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 10/31/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#ifndef REGSITERED_SINGLETON_TESTS_H
#define REGSITERED_SINGLETON_TESTS_H

#include "RegisteredSingleton.h"
#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace mw {
class Dummy : public mw::Component, public RegisteredSingleton<Dummy> {
};

class RegisteredSingletonTestFixture : public CppUnit::TestFixture {
	
	CPPUNIT_TEST_SUITE( RegisteredSingletonTestFixture );	
	CPPUNIT_TEST (simpleCreateTest);
	CPPUNIT_TEST (simpleCreateTestFromGenericComponent);
	CPPUNIT_TEST (testCreationFromWrongComponent);
	
	CPPUNIT_TEST (destroyTest);
	CPPUNIT_TEST (destroyUnregsiteredTest);
	CPPUNIT_TEST (tryToGetUnregsiteredTest);
	CPPUNIT_TEST (tryToRegisterMultipleSingletons);
	CPPUNIT_TEST (tryToRegisterNULLInstance);
	
	CPPUNIT_TEST_SUITE_END();
	
	
public:
	void setUp();
	void tearDown();
	
	void simpleCreateTest();
	void simpleCreateTestFromGenericComponent();
	void testCreationFromWrongComponent();
	void destroyTest();
	void destroyUnregsiteredTest();
	void tryToGetUnregsiteredTest();
	void tryToRegisterMultipleSingletons();
	void tryToRegisterNULLInstance();
};
}
#endif

