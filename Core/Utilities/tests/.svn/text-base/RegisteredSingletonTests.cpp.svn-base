/*
 *  RegisteredSingletonTests.cpp
 *  MonkeyWorksCore
 *
 *  Created by bkennedy on 10/31/08.
 *  Copyright 2008 mit. All rights reserved.
 *
 */

#include "RegisteredSingletonTests.h"
#include "MonkeyWorksCore/ConstantVariable.h"
using namespace mw;


template <> boost::shared_ptr<Dummy> RegisteredSingleton<Dummy>::singleton_instance = boost::shared_ptr<Dummy>();

#define NUMBER_OF_SINGLETON_TEST_LOOPS 10

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( RegisteredSingletonTestFixture, "Unit Test" );

void RegisteredSingletonTestFixture::setUp() {
	boost::shared_ptr <Dummy> dummy_instance = Dummy::instance(false);
	CPPUNIT_ASSERT(dummy_instance == 0);	
}

void RegisteredSingletonTestFixture::tearDown() {
	boost::shared_ptr <Dummy> dummy_instance = Dummy::instance(false);
	CPPUNIT_ASSERT(dummy_instance == 0);
}

void RegisteredSingletonTestFixture::simpleCreateTest(){
	boost::shared_ptr <Dummy> dummy_instance = Dummy::instance(false);
	CPPUNIT_ASSERT(dummy_instance == 0);
	
	boost::shared_ptr <Dummy> new_dummy = boost::shared_ptr<Dummy>(new Dummy());
	Dummy::registerInstance(new_dummy);
	
	for(int test_loop = 0; test_loop < NUMBER_OF_SINGLETON_TEST_LOOPS; ++test_loop) {
		dummy_instance = Dummy::instance(false);	
		CPPUNIT_ASSERT(dummy_instance != 0);
		
		dummy_instance = Dummy::instance(true);	
		CPPUNIT_ASSERT(dummy_instance != 0);
		
		dummy_instance = Dummy::instance();	
		CPPUNIT_ASSERT(dummy_instance != 0);
	}
	
	Dummy::destroy();
}

void RegisteredSingletonTestFixture::simpleCreateTestFromGenericComponent(){
	boost::shared_ptr <Dummy> dummy_instance = Dummy::instance(false);
	CPPUNIT_ASSERT(dummy_instance == 0);
	
	boost::shared_ptr <mw::Component> new_dummy = boost::shared_ptr<mw::Component>(new Dummy());
	Dummy::registerInstance(new_dummy);
	
	for(int test_loop = 0; test_loop < NUMBER_OF_SINGLETON_TEST_LOOPS; ++test_loop) {
		dummy_instance = Dummy::instance(false);	
		CPPUNIT_ASSERT(dummy_instance != 0);
		
		dummy_instance = Dummy::instance(true);	
		CPPUNIT_ASSERT(dummy_instance != 0);
		
		dummy_instance = Dummy::instance();	
		CPPUNIT_ASSERT(dummy_instance != 0);
	}
	
	Dummy::destroy();
}

void RegisteredSingletonTestFixture::testCreationFromWrongComponent(){
	boost::shared_ptr <Dummy> dummy_instance = Dummy::instance(false);
	CPPUNIT_ASSERT(dummy_instance == 0);
	
	for(int test_loop = 0; test_loop < NUMBER_OF_SINGLETON_TEST_LOOPS; ++test_loop) {
		boost::shared_ptr <mw::Component> new_component = boost::shared_ptr<mw::Component>(new mw::Component());
		
		bool threw_exception = false;
		try {
			Dummy::registerInstance(new_component);
		} catch (std::exception &e) {
			CPPUNIT_ASSERT(e.what() == std::string("attempting to register singleton instance for wrong type of object"));
			threw_exception = true;
		}
		
		CPPUNIT_ASSERT(threw_exception);				
	}

	// try with an subclass of mw::Component
	dummy_instance = Dummy::instance(false);
	CPPUNIT_ASSERT(dummy_instance == 0);
	
	for(int test_loop = 0; test_loop < NUMBER_OF_SINGLETON_TEST_LOOPS; ++test_loop) {
		boost::shared_ptr <ConstantVariable> new_constant_variable = boost::shared_ptr<ConstantVariable>(new ConstantVariable(0.0));
		
		bool threw_exception = false;
		try {
			Dummy::registerInstance(new_constant_variable);
		} catch (std::exception &e) {
			CPPUNIT_ASSERT(e.what() == std::string("attempting to register singleton instance for wrong type of object"));
			threw_exception = true;
		}
		
		CPPUNIT_ASSERT(threw_exception);				
	}

	dummy_instance = Dummy::instance(false);
	CPPUNIT_ASSERT(dummy_instance == 0);
}

void RegisteredSingletonTestFixture::destroyTest(){
	for(int test_loop = 0; test_loop < NUMBER_OF_SINGLETON_TEST_LOOPS; ++test_loop) {
		boost::shared_ptr <Dummy> dummy_instance = Dummy::instance(false);
		CPPUNIT_ASSERT(dummy_instance == 0);	
		
		boost::shared_ptr <Dummy> new_dummy = boost::shared_ptr<Dummy>(new Dummy());
		Dummy::registerInstance(new_dummy);
		
		Dummy::destroy();
		
		dummy_instance = Dummy::instance(false);	
		CPPUNIT_ASSERT(dummy_instance == 0);
	}
}

void RegisteredSingletonTestFixture::destroyUnregsiteredTest(){
	for(int test_loop = 0; test_loop < NUMBER_OF_SINGLETON_TEST_LOOPS; ++test_loop) {
		boost::shared_ptr <Dummy> dummy_instance = Dummy::instance(false);
		CPPUNIT_ASSERT(dummy_instance == 0);	
		
		bool threw_exception = false;
		try {
			Dummy::destroy();
		} catch (std::exception &e) {
			CPPUNIT_ASSERT(e.what() == std::string("trying to destroy singleton when there isn't one registered"));
			threw_exception = true;
		}

		CPPUNIT_ASSERT(threw_exception);				

		dummy_instance = Dummy::instance(false);		
		CPPUNIT_ASSERT(dummy_instance == 0);	
	}
}

void RegisteredSingletonTestFixture::tryToGetUnregsiteredTest(){
	boost::shared_ptr <Dummy> dummy_instance = Dummy::instance(false);
	CPPUNIT_ASSERT(dummy_instance == 0);
	
	for(int test_loop = 0; test_loop < NUMBER_OF_SINGLETON_TEST_LOOPS; ++test_loop) {
		dummy_instance = Dummy::instance(false);		
		CPPUNIT_ASSERT(dummy_instance == 0);	

		bool threw_exception = false;
		try {
			dummy_instance = Dummy::instance(false);		
		} catch (std::exception &e) {
			threw_exception = true;
		}

		CPPUNIT_ASSERT(!threw_exception);				
		
		threw_exception = false;
		try {
			dummy_instance = Dummy::instance(true);		
		} catch (std::exception &e) {
			threw_exception = true;
			CPPUNIT_ASSERT(e.what() == std::string("trying to access singleton instance before it is registered"));
		}
		
		CPPUNIT_ASSERT(threw_exception);		
		
		threw_exception = false;
		try {
			dummy_instance = Dummy::instance();		
		} catch (std::exception &e) {
			threw_exception = true;
			CPPUNIT_ASSERT(e.what() == std::string("trying to access singleton instance before it is registered"));
		}
		
		CPPUNIT_ASSERT(threw_exception);		
	}
	dummy_instance = Dummy::instance(false);		
	CPPUNIT_ASSERT(dummy_instance == 0);	
}

void RegisteredSingletonTestFixture::tryToRegisterMultipleSingletons(){
	boost::shared_ptr <Dummy> dummy_instance = Dummy::instance(false);
	CPPUNIT_ASSERT(dummy_instance == 0);
	for(int test_loop = 0; test_loop < NUMBER_OF_SINGLETON_TEST_LOOPS; ++test_loop) {		
		
		boost::shared_ptr <Dummy> new_dummy = boost::shared_ptr<Dummy>(new Dummy());
		Dummy::registerInstance(new_dummy);
		
		for(int test_loop2 = 0; test_loop2 < NUMBER_OF_SINGLETON_TEST_LOOPS; ++test_loop2) {		
			bool threw_exception = false;
			try {
				new_dummy = boost::shared_ptr<Dummy>(new Dummy());
				Dummy::registerInstance(new_dummy);
			} catch (std::exception &e) {
				threw_exception = true;
				CPPUNIT_ASSERT(e.what() == std::string("attempting to register singleton instance when there's already an instance"));
			}
			
			CPPUNIT_ASSERT(threw_exception);		
		}
		
		Dummy::destroy();
	}
}

void RegisteredSingletonTestFixture::tryToRegisterNULLInstance(){
	for(int test_loop = 0; test_loop < NUMBER_OF_SINGLETON_TEST_LOOPS; ++test_loop) {		
		boost::shared_ptr <Dummy> dummy_instance = Dummy::instance(false);
		CPPUNIT_ASSERT(dummy_instance == 0);
		
		bool threw_exception = false;
		try {
			boost::shared_ptr <Dummy> new_dummy = boost::shared_ptr<Dummy>();
			Dummy::registerInstance(new_dummy);
		} catch (std::exception &e) {
			threw_exception = true;
		}
		
		CPPUNIT_ASSERT(threw_exception);		
	}
}


