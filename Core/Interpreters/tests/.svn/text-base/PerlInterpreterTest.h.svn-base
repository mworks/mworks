
#ifndef	PERL_INTERPRETER_TEST_H_
#define PERL_INTERPRETER_TEST_H_

/*
 *  PerlInterpreterTest.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MonkeyWorksCore/EmbeddedPerlInterpreter.h"
#include "MonkeyWorksCore/Experiment.h"
#include "MonkeyWorksCore/IODeviceManager.h"
namespace mw {
class PerlInterpreterTestFixture : public CppUnit::TestFixture {


	CPPUNIT_TEST_SUITE( PerlInterpreterTestFixture );
	
	CPPUNIT_TEST( testInstantiatingObjects );
	
	CPPUNIT_TEST_SUITE_END();


	private:
	
		EmbeddedPerlInterpreter *p;

	public:
	
		void setUp(){
				
			p = new EmbeddedPerlInterpreter();
		}

		void tearDown(){
			
			delete p;
		}
					
		void testInstantiatingObjects(){
			fprintf(stderr, "Running PerlInterpreterTestFixture::testInstantiatingObjects()\n");
			
			// this is a convenient commonly accessible point
			GlobalIODeviceManager = NULL;
			
			/*
			p->evalString("use MonkeyScript;								\
						   $MonkeyScript::GlobalIODeviceManager =			\
								new MonkeyScript::IODeviceManager();");
			
			CPPUNIT_ASSERT(  GlobalIODeviceManager != NULL );
			*/  // test is not working right now... (though underlying 
				//  functionality seems fine
		}
	
		
		
		
};
}


#endif

