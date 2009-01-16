
#ifndef	SELECTION_VARIABLE_TEST_H_
#define SELECTION_VARIABLE_TEST_H_

/*
 *  SelectionVariableTest.h
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */


#include <cppunit/TestCase.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include "MonkeyWorksCore/SelectionVariable.h"
#include "MonkeyWorksCore/ConstantVariable.h"
namespace mw {
class SelectionVariableTestFixture : public CppUnit::TestFixture  {


	CPPUNIT_TEST_SUITE( SelectionVariableTestFixture );
	
	CPPUNIT_TEST( testWithConstantVariables );
	CPPUNIT_TEST( testWithConstantVariablesDescending );
	CPPUNIT_TEST( testReset );
	CPPUNIT_TEST( testResetDescending );
	CPPUNIT_TEST( testPartialSamples );
	CPPUNIT_TEST( testCycles );
	CPPUNIT_TEST( testMultipleCycles );
	CPPUNIT_TEST( testSurplusSamples );
	CPPUNIT_TEST( testAcceptSelections );
	CPPUNIT_TEST( testRejectSelections );
	CPPUNIT_TEST( testRejectSelectionsAcrossCyclesUsingSequentialAscending );
	CPPUNIT_TEST( testRejectSelectionsAcrossCyclesUsingSequentialDescending );
	CPPUNIT_TEST( testRejectSelectionsAfterHittingEdge );	
	CPPUNIT_TEST( testRejectSelectionsAfterHittingEdgeDescending );	
	CPPUNIT_TEST( testRejectSelectionsWithMutilpleCyclesAfterHittingEdge );	
	CPPUNIT_TEST( testRejectSelectionsWithMutilpleCyclesAfterHittingEdgeDescending );	
	CPPUNIT_TEST( testRandomWORSelections );
	CPPUNIT_TEST( testRandomWORCycles );
	CPPUNIT_TEST( testResetRandomWOR );
	CPPUNIT_TEST( testRandomWORPartialSamples );
	CPPUNIT_TEST( testRandomWORSurplusSamples );
	CPPUNIT_TEST( testRandomWORRejects );
	CPPUNIT_TEST( testRandomWORRejectsOverCycleEdge1 );
	CPPUNIT_TEST( testRandomWORRejectsOverCycleEdge2 );
	CPPUNIT_TEST( testRandomWithReplacementSelections );
	CPPUNIT_TEST( testRandomWithReplacementCycles );
	CPPUNIT_TEST( testResetRandomWithReplacement );
	CPPUNIT_TEST( testRandomWithReplacementRejects );
	
	
	CPPUNIT_TEST_SUITE_END();


	private:
	
		shared_ptr<ConstantVariable> a,b,c,d;

	public:
	
		void setUp();

	void tearDown();
					
	void testWithConstantVariables();
	void testWithConstantVariablesDescending();
	void testReset();
	void testResetDescending();
	void testPartialSamples();
	void testSurplusSamples();
	void testCycles();
	void testMultipleCycles();
	void testAcceptSelections();
	void testRejectSelections();
	void testRejectSelectionsAcrossCyclesUsingSequentialAscending();
	void testRejectSelectionsAcrossCyclesUsingSequentialDescending();
	void testRejectSelectionsAfterHittingEdge();
	void testRejectSelectionsAfterHittingEdgeDescending();
	void testRejectSelectionsWithMutilpleCyclesAfterHittingEdge();
	void testRejectSelectionsWithMutilpleCyclesAfterHittingEdgeDescending();
	void testRandomWORSelections();
	void testRandomWORCycles();
	void testResetRandomWOR();
	void testRandomWORPartialSamples();
	void testRandomWORSurplusSamples();
	void testRandomWORRejects();
	void testRandomWORRejectsOverCycleEdge1();
	void testRandomWORRejectsOverCycleEdge2();
	void testRandomWithReplacementSelections();
	void testRandomWithReplacementCycles();
	void testResetRandomWithReplacement();
	void testRandomWithReplacementRejects();
	
	
};
}


#endif


