/*
 *  SelectionVariableTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by David Cox on 3/30/06.
 *  Copyright 2006 MIT. All rights reserved.
 *
 */

#include "SelectionVariableTest.h"
#include "Selection.h"
#include "SequentialSelection.h"
#include "RandomWORSelection.h"
#include "RandomWithReplacementSelection.h"
#include "Clock.h"
using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( SelectionVariableTestFixture, "Unit Test" );
//CPPUNIT_TEST_SUITE_REGISTRATION( SelectionVariableTestFixture );


void SelectionVariableTestFixture::setUp(){
	shared_ptr <Clock> new_clock = shared_ptr<Clock>(new Clock(0));
	Clock::registerInstance(new_clock);
	
	a = shared_ptr<ConstantVariable>(new ConstantVariable(Data(10L)));
	b = shared_ptr<ConstantVariable>(new ConstantVariable(Data(20L)));
	c = shared_ptr<ConstantVariable>(new ConstantVariable(Data(12.34)));
	d = shared_ptr<ConstantVariable>(new ConstantVariable(Data(45.67)));
}

void SelectionVariableTestFixture::tearDown(){
	Clock::destroy();
}

void SelectionVariableTestFixture::testWithConstantVariables(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testWithConstantVariables()\n");
	
	SelectionVariable var(NULL);
	
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> selection( new SequentialSelection(4) );
	var.attachSelection(selection);
	
	CPPUNIT_ASSERT(var.getNDone() == 0);			
	CPPUNIT_ASSERT(var.getNLeft() == 4);			

	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	CPPUNIT_ASSERT(var.getNDone() == 1);			
	CPPUNIT_ASSERT(var.getNLeft() == 3);			
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	CPPUNIT_ASSERT(var.getNDone() == 2);			
	CPPUNIT_ASSERT(var.getNLeft() == 2);			
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	CPPUNIT_ASSERT(var.getNDone() == 3);			
	CPPUNIT_ASSERT(var.getNLeft() == 1);			
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	CPPUNIT_ASSERT(var.getNDone() == 4);			
	CPPUNIT_ASSERT(var.getNLeft() == 0);			
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	CPPUNIT_ASSERT(var.getNDone() == 4);			
	CPPUNIT_ASSERT(var.getNLeft() == 0);			
}

void SelectionVariableTestFixture::testWithConstantVariablesDescending(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testWithConstantVariablesDescending()\n");
	
	SelectionVariable var(NULL);
	
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new SequentialSelection(4, false) );
	var.attachSelection(sel);
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
}

void SelectionVariableTestFixture::testReset(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testReset()\n");
	
	SelectionVariable var(NULL);
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new SequentialSelection(4) );
	var.attachSelection(sel);
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	
				
	var.resetSelections();
	CPPUNIT_ASSERT(var.getNDone() == 0);			
	CPPUNIT_ASSERT(var.getNLeft() == 4);			

	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
}

void SelectionVariableTestFixture::testResetDescending(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testResetDescending()\n");
	
	SelectionVariable var(NULL);
	
	var.addValue(d);
	var.addValue(c);
	var.addValue(b);
	var.addValue(a);
	
	shared_ptr<Selection> sel( new SequentialSelection(4, false) );
	var.attachSelection(sel);
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	
				
	var.resetSelections();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
}


void SelectionVariableTestFixture::testPartialSamples(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testPartialSamples()\n");
	
	SelectionVariable var(NULL);
	
	shared_ptr<Selection> sel( new SequentialSelection(3) );
	var.attachSelection(sel);	
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)var == (long)(*b) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)var == (long)(*c) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)var == (long)(*c) );
	

}


void SelectionVariableTestFixture::testSurplusSamples(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testSurplusSamples()\n");
	
	SelectionVariable var(NULL);
	shared_ptr<Selection> sel( new SequentialSelection(5) );
	var.attachSelection(sel);	
	
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
}

void SelectionVariableTestFixture::testCycles(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testCycles()\n");
	
	SelectionVariable var(NULL);
	shared_ptr<Selection> sel( new SequentialSelection(4) );
	var.attachSelection(sel);	
		
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );	
}

void SelectionVariableTestFixture::testMultipleCycles(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testMultipleCycles()\n");
	SelectionVariable var(NULL);
	
	
	shared_ptr<Selection> sel( new SequentialSelection(8) );
	var.attachSelection(sel);
//	var.setSelectionType(M_SEQUENTIAL_ASCENDING);
//	var.setSamplingMethod(M_CYCLES);
//	var.setN(2);
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );

}


void SelectionVariableTestFixture::testAcceptSelections(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testAcceptSelections()\n");
	SelectionVariable var(NULL);
	shared_ptr<Selection> sel( new SequentialSelection(4) );
	var.attachSelection(sel);	
		
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.acceptSelections();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.acceptSelections();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
	var.acceptSelections();	
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
}

void SelectionVariableTestFixture::testRejectSelections(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRejectSelections()\n");
	SelectionVariable var(NULL);
	shared_ptr<Selection> sel( new SequentialSelection(4) );
	var.attachSelection(sel);	
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );			
	var.acceptSelections();
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );			
	
	// a & b are permanently selected
	
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	// but we reject it
	var.rejectSelections();
	
	// we get c again
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	// this time we accept it
	var.acceptSelections();
	
	// so the next draw:
	var.nextValue();
	
	// is "d"
	CPPUNIT_ASSERT( (long)(var) == (long)(*d) );
}

void SelectionVariableTestFixture::testRejectSelectionsAcrossCyclesUsingSequentialAscending(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRejectSelectionsAcrossCyclesUsingSequentialAscending()\n");
	SelectionVariable var(NULL);
	
	shared_ptr<Selection> sel( new SequentialSelection(6) ); // 2 cycles
	var.attachSelection(sel);	
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );			
	var.acceptSelections();
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );			
	
	// a & b are permanently selected
	
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
	// but we reject it
	var.rejectSelections();

	// we get c again
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	// this time we accept it
	var.acceptSelections();
	
	// so the next draw:
	var.nextValue();
	
	// is "a"
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
}

void SelectionVariableTestFixture::testRejectSelectionsAcrossCyclesUsingSequentialDescending(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRejectSelectionsAcrossCyclesUsingSequentialDescending()\n");
	SelectionVariable var(NULL);
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	
	shared_ptr<Selection> sel( new SequentialSelection(6, false) );
	var.attachSelection(sel);	
//	var.setSelectionType(M_SEQUENTIAL_DESCENDING);
//	var.setSamplingMethod(M_CYCLES);
//	var.setN(2);
	
	CPPUNIT_ASSERT( (long)var == (long)(*c) );
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );			
	var.acceptSelections();
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );			
	
	// a & b are permanently selected
	
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	// but we reject it
	var.rejectSelections();
	
	// we get a again
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
	// this time we accept it
	var.acceptSelections();
	
	// so the next draw:
	var.nextValue();
	
	// is "c"
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
}

void SelectionVariableTestFixture::testRejectSelectionsAfterHittingEdge(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRejectSelectionAfterHittingEdge()\n");
	SelectionVariable var(NULL);
	
	shared_ptr<Selection> sel( new SequentialSelection(3) );
	var.attachSelection(sel);	
//	var.setSelectionType(M_SEQUENTIAL_ASCENDING);
//	var.setSamplingMethod(M_CYCLES);
//	var.setN(1);
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) );
	var.acceptSelections();
	CPPUNIT_ASSERT( (long)var == (long)(*a) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );			
	
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	// but we reject it
	var.rejectSelections();
	
	// we get b again
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	
	var.nextValue();
	
	// is "c"
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	var.nextValue();
	
	// is "c"
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
}

void SelectionVariableTestFixture::testRejectSelectionsAfterHittingEdgeDescending(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRejectSelectionsAfterHittingEdgeDescending()\n");
	SelectionVariable var(NULL);
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	
	shared_ptr<Selection> sel( new SequentialSelection(3, false) );
	var.attachSelection(sel);	
//	var.setSelectionType(M_SEQUENTIAL_DESCENDING);
//	var.setSamplingMethod(M_CYCLES);
//	var.setN(1);
	
	CPPUNIT_ASSERT( (long)var == (long)(*c) );
	var.acceptSelections();
	CPPUNIT_ASSERT( (long)var == (long)(*c) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );			
	
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
	// but we reject it
	var.rejectSelections();
	
	// we get b again
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	
	var.nextValue();
	
	// is "c"
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
	var.nextValue();
	
	// is "c"
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
}


void SelectionVariableTestFixture::testRejectSelectionsWithMutilpleCyclesAfterHittingEdge(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRejectSelectionsWithMutilpleCyclesAfterHittingEdge()\n");
	SelectionVariable var(NULL);
	
	shared_ptr<Selection> sel( new SequentialSelection(6) );
	var.attachSelection(sel);	
	//var.setSelectionType(M_SEQUENTIAL_ASCENDING);
//	var.setSamplingMethod(M_CYCLES);
//	var.setN(2);
//	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) );
	var.acceptSelections();
	CPPUNIT_ASSERT( (long)var == (long)(*a) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );			
	
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	// the next value is b ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	// but we reject it
	var.rejectSelections();
	
	// we get b again
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	
	var.nextValue();
	
	// is "c"
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	// the next value is b ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	
}

void SelectionVariableTestFixture::testRejectSelectionsWithMutilpleCyclesAfterHittingEdgeDescending(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRejectSelectionsWithMutilpleCyclesAfterHittingEdgeDescending()\n");
	SelectionVariable var(NULL);
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	
	shared_ptr<Selection> sel( new SequentialSelection(6, false) );
	var.attachSelection(sel);	
//	var.setSelectionType(M_SEQUENTIAL_DESCENDING);
//	var.setSamplingMethod(M_CYCLES);
//	var.setN(2);
	
	CPPUNIT_ASSERT( (long)var == (long)(*c) );
	var.acceptSelections();
	CPPUNIT_ASSERT( (long)var == (long)(*c) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );			
	
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	// the next value is b ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
	// but we reject it
	var.rejectSelections();
	
	// we get b again
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	
	var.nextValue();
	
	// is "a"
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
	var.nextValue();
	
	// the next value is c ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*c) );
	var.nextValue();
	
	// the next value is b ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*b) );
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	var.nextValue();
	
	// the next value is a ...
	CPPUNIT_ASSERT( (long)(var) == (long)(*a) );
	
}


void SelectionVariableTestFixture::testRandomWORSelections(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRandomWORSelections()\n");
	
	SelectionVariable var(NULL);
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new RandomWORSelection(4) );
	var.attachSelection(sel);	
//	var.setSelectionType(M_RANDOM_WOR);
//	var.setSamplingMethod(M_SAMPLES);
//	var.setN(4);	
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	shared_ptr<ConstantVariable> a2,b2,c2;
	
	
	if((long)var == (long)(*a)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));
		c2 = shared_ptr<ConstantVariable>(new ConstantVariable(d->getValue()));			
	} else if((long)var == (long)(*b)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));
		c2 = shared_ptr<ConstantVariable>(new ConstantVariable(d->getValue()));			
	} else if((long)var == (long)(*c)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
		c2 = shared_ptr<ConstantVariable>(new ConstantVariable(d->getValue()));			
	} else if((long)var == (long)(*d)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
		c2 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));			
	} else {
		CPPUNIT_ASSERT(0);
	}
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a2) || 
					(long)var == (long)(*b2) ||
					(long)var == (long)(*c2));
	
	shared_ptr<ConstantVariable> a3,b3;
	
	
	if((long)var == (long)(*a2)) {
		a3 = shared_ptr<ConstantVariable>(new ConstantVariable(b2->getValue()));
		b3 = shared_ptr<ConstantVariable>(new ConstantVariable(c2->getValue()));
	} else if((long)var == (long)(*b2)) {
		a3 = shared_ptr<ConstantVariable>(new ConstantVariable(a2->getValue()));
		b3 = shared_ptr<ConstantVariable>(new ConstantVariable(c2->getValue()));
	} else if((long)var == (long)(*c2)) {
		a3 = shared_ptr<ConstantVariable>(new ConstantVariable(a2->getValue()));
		b3 = shared_ptr<ConstantVariable>(new ConstantVariable(b2->getValue()));
	} else {
		CPPUNIT_ASSERT(0);
	}
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a3) || 
					(long)var == (long)(*b3));
	
	
	shared_ptr<ConstantVariable> a4;
	
	if((long)var == (long)(*a3)) {
		a4 = shared_ptr<ConstantVariable>(new ConstantVariable(b3->getValue()));
	} else if((long)var == (long)(*b3)) {
		a4 = shared_ptr<ConstantVariable>(new ConstantVariable(a3->getValue()));
	} else {
		CPPUNIT_ASSERT(0);
	}
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a4));
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a4));
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a4));		
}


void SelectionVariableTestFixture::testRandomWORCycles(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRandomWORCycles()\n");
	
	SelectionVariable var(NULL);
	
	var.addValue(a);
	var.addValue(b);
	
	shared_ptr<Selection> sel( new RandomWORSelection(4) );
	var.attachSelection(sel);
//	var.setSelectionType(M_RANDOM_WOR);
//	var.setSamplingMethod(M_CYCLES);
//	var.setN(2);	
	
	shared_ptr<ConstantVariable> a2 = shared_ptr<ConstantVariable>(new ConstantVariable(var.getValue()));
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b));
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b));
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b));
	
	var.nextValue();
	
	shared_ptr<ConstantVariable> a3 = shared_ptr<ConstantVariable>(new ConstantVariable(var.getValue()));
	
	for(int i=0; i<100; ++i) {
		var.nextValue();
		CPPUNIT_ASSERT((long)var == long(*a3));
	}	
}

void SelectionVariableTestFixture::testResetRandomWOR(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testResetRandomWOR()\n");
	
	SelectionVariable var(NULL);
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new RandomWORSelection(4) );
	var.attachSelection(sel);
	
	CPPUNIT_ASSERT(var.getNDone() == 0);			
	CPPUNIT_ASSERT(var.getNLeft() == 4);			

	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	shared_ptr<ConstantVariable> a2,b2,c2;
	
	
	if((long)var == (long)(*a)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));
		c2 = shared_ptr<ConstantVariable>(new ConstantVariable(d->getValue()));			
	} else if((long)var == (long)(*b)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));
		c2 = shared_ptr<ConstantVariable>(new ConstantVariable(d->getValue()));			
	} else if((long)var == (long)(*c)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
		c2 = shared_ptr<ConstantVariable>(new ConstantVariable(d->getValue()));			
	} else if((long)var == (long)(*d)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
		c2 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));			
	} else {
		CPPUNIT_ASSERT(0);
	}
	
	CPPUNIT_ASSERT(var.getNDone() == 1);			
	CPPUNIT_ASSERT(var.getNLeft() == 3);			
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a2) || 
					(long)var == (long)(*b2) ||
					(long)var == (long)(*c2));
	
	CPPUNIT_ASSERT(var.getNDone() == 2);			
	CPPUNIT_ASSERT(var.getNLeft() == 2);			
	
	shared_ptr<ConstantVariable> a3,b3;
	
	
	if((long)var == (long)(*a2)) {
		a3 = shared_ptr<ConstantVariable>(new ConstantVariable(b2->getValue()));
		b3 = shared_ptr<ConstantVariable>(new ConstantVariable(c2->getValue()));
	} else if((long)var == (long)(*b2)) {
		a3 = shared_ptr<ConstantVariable>(new ConstantVariable(a2->getValue()));
		b3 = shared_ptr<ConstantVariable>(new ConstantVariable(c2->getValue()));
	} else if((long)var == (long)(*c2)) {
		a3 = shared_ptr<ConstantVariable>(new ConstantVariable(a2->getValue()));
		b3 = shared_ptr<ConstantVariable>(new ConstantVariable(b2->getValue()));
	} else {
		CPPUNIT_ASSERT(0);
	}
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a3) || 
					(long)var == (long)(*b3));
	
	
	CPPUNIT_ASSERT(var.getNDone() == 3);			
	CPPUNIT_ASSERT(var.getNLeft() == 1);			
	
	shared_ptr<ConstantVariable> a4;
	
	if((long)var == (long)(*a3)) {
		a4 = shared_ptr<ConstantVariable>(new ConstantVariable(b3->getValue()));
	} else if((long)var == (long)(*b3)) {
		a4 = shared_ptr<ConstantVariable>(new ConstantVariable(a3->getValue()));
	} else {
		CPPUNIT_ASSERT(0);
	}
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a4));
	CPPUNIT_ASSERT(var.getNDone() == 4);			
	CPPUNIT_ASSERT(var.getNLeft() == 0);			
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a4));
	CPPUNIT_ASSERT(var.getNDone() == 4);			
	CPPUNIT_ASSERT(var.getNLeft() == 0);			
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a4));		
	CPPUNIT_ASSERT(var.getNDone() == 4);			
	CPPUNIT_ASSERT(var.getNLeft() == 0);			
	
				
	var.resetSelections();
	CPPUNIT_ASSERT(var.getNDone() == 0);			
	CPPUNIT_ASSERT(var.getNLeft() == 4);			
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	shared_ptr<ConstantVariable> a5,b5,c5;
	
	
	if((long)var == (long)(*a)) {
		a5 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
		b5 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));
		c5 = shared_ptr<ConstantVariable>(new ConstantVariable(d->getValue()));			
	} else if((long)var == (long)(*b)) {
		a5 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b5 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));
		c5 = shared_ptr<ConstantVariable>(new ConstantVariable(d->getValue()));			
	} else if((long)var == (long)(*c)) {
		a5 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b5 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
		c5 = shared_ptr<ConstantVariable>(new ConstantVariable(d->getValue()));			
	} else if((long)var == (long)(*d)) {
		a5 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b5 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
		c5 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));			
	} else {
		CPPUNIT_ASSERT(0);
	}
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a5) || 
					(long)var == (long)(*b5) ||
					(long)var == (long)(*c5));
	
	shared_ptr<ConstantVariable> a6,b6;
	
	
	if((long)var == (long)(*a5)) {
		a6 = shared_ptr<ConstantVariable>(new ConstantVariable(b5->getValue()));
		b6 = shared_ptr<ConstantVariable>(new ConstantVariable(c5->getValue()));
	} else if((long)var == (long)(*b5)) {
		a6 = shared_ptr<ConstantVariable>(new ConstantVariable(a5->getValue()));
		b6 = shared_ptr<ConstantVariable>(new ConstantVariable(c5->getValue()));
	} else if((long)var == (long)(*c5)) {
		a6 = shared_ptr<ConstantVariable>(new ConstantVariable(a5->getValue()));
		b6 = shared_ptr<ConstantVariable>(new ConstantVariable(b5->getValue()));
	} else {
		CPPUNIT_ASSERT(0);
	}
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a6) || 
					(long)var == (long)(*b6));
	
	
	shared_ptr<ConstantVariable> a7;
	
	if((long)var == (long)(*a6)) {
		a7 = shared_ptr<ConstantVariable>(new ConstantVariable(b6->getValue()));
	} else if((long)var == (long)(*b6)) {
		a7 = shared_ptr<ConstantVariable>(new ConstantVariable(a6->getValue()));
	} else {
		CPPUNIT_ASSERT(0);
	}
	
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a7));
	CPPUNIT_ASSERT(var.getNDone() == 4);			
	CPPUNIT_ASSERT(var.getNLeft() == 0);			
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a7));
	CPPUNIT_ASSERT(var.getNDone() == 4);			
	CPPUNIT_ASSERT(var.getNLeft() == 0);			
	
	var.nextValue();
	CPPUNIT_ASSERT( (long)var == (long)(*a7));		
	CPPUNIT_ASSERT(var.getNDone() == 4);			
	CPPUNIT_ASSERT(var.getNLeft() == 0);			
}



void SelectionVariableTestFixture::testRandomWORPartialSamples(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testRandomWORPartialSamples()\n");
	
	
	SelectionVariable var(NULL);
	
	long L10 = 1;
	long L20 = 1;
	long L12 = 1;
	long L45 = 1;
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new RandomWORSelection(3) );
	var.attachSelection(sel);
//	var.setSelectionType(M_RANDOM_WOR);
//	var.setSamplingMethod(M_SAMPLES);
//	var.setN(3);
	
	for(int i=0; i<3; ++i) {
		switch((long)var) {
			case 10:
				--L10;
				break;
			case 20:
				--L20;
				break;
			case 12:
				--L12;
				break;
			case 45:
				--L45;
				break;
			default:
				CPPUNIT_ASSERT(0);
				break;
		}
		
		CPPUNIT_ASSERT(L10 >= 0 &&
					   L20 >= 0 &&
					   L12 >= 0 &&
					   L45 >= 0);
		
		CPPUNIT_ASSERT(L10 * L20 * L12 * L45 == 0);
		
		var.nextValue();
	}
	
	// one of them should be equal to 1
	CPPUNIT_ASSERT((L10 == 1 && L20 == 0 && L12 == 0 && L45 == 0) ||
				   (L10 == 0 && L20 == 1 && L12 == 0 && L45 == 0) ||
				   (L10 == 0 && L20 == 0 && L12 == 1 && L45 == 0) ||
				   (L10 == 0 && L20 == 0 && L12 == 0 && L45 == 1));
	
	
	shared_ptr<ConstantVariable> a3 = shared_ptr<ConstantVariable>(new ConstantVariable(var.getValue()));
	
	for(int i=0; i<10; ++i) {
		var.nextValue();
		switch((long)var) {
			case 10:
				--L10;
				break;
			case 20:
				--L20;
				break;
			case 12:
				--L12;
				break;
			case 45:
				--L45;
				break;
			default:
				CPPUNIT_ASSERT(0);
				break;
		}
		CPPUNIT_ASSERT((long)var == long(*a3));
		CPPUNIT_ASSERT(L10 < 0 ||
					   L20 < 0 ||
					   L12 < 0 ||
					   L45 < 0);
		
		// one of them should be less than zero
		CPPUNIT_ASSERT((L10 < 0 && L20 >= 0 && L12 >= 0 && L45 >= 0) ||
					   (L10 >= 0 && L20 < 0 && L12 >= 0 && L45 >= 0) ||
					   (L10 >= 0 && L20 >= 0 && L12 < 0 && L45 >= 0) ||
					   (L10 >= 0 && L20 >= 0 && L12 >= 0 && L45 < 0));
	}	
	
}


void SelectionVariableTestFixture::testRandomWORSurplusSamples(){
		  fprintf(stderr, "Running SelectionVariableTestFixture::testRandomWORPartialSamples()\n");
	
	
	SelectionVariable var(NULL);
	
	long L10 = 2;
	long L20 = 2;
	long L12 = 2;
	long L45 = 2;
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new RandomWORSelection(5) );
	var.attachSelection(sel);
//	var.setSelectionType(M_RANDOM_WOR);
//	var.setSamplingMethod(M_SAMPLES);
//	var.setN(5);
	
	for(int i=0; i<5; ++i) {
		switch((long)var) {
			case 10:
				--L10;
				break;
			case 20:
				--L20;
				break;
			case 12:
				--L12;
				break;
			case 45:
				--L45;
				break;
			default:
				CPPUNIT_ASSERT(0);
				break;
		}
		
		CPPUNIT_ASSERT(L10 >= 0 &&
					   L20 >= 0 &&
					   L12 >= 0 &&
					   L45 >= 0);
		
		
		var.nextValue();
	}
	
	CPPUNIT_ASSERT(L10 + L20 + L12 + L45 == 3);
	
	// at least one of them should be equal to zero
	CPPUNIT_ASSERT(L10 * L20 * L12 * L45 == 0);
	
	
	
	shared_ptr<ConstantVariable> a3 = shared_ptr<ConstantVariable>(new ConstantVariable(var.getValue()));
	
	for(int i=0; i<10; ++i) {
		var.nextValue();
		switch((long)var) {
			case 10:
				--L10;
				break;
			case 20:
				--L20;
				break;
			case 12:
				--L12;
				break;
			case 45:
				--L45;
				break;
			default:
				CPPUNIT_ASSERT(0);
				break;
		}
		CPPUNIT_ASSERT((long)var == long(*a3));
		
		// at least one of them should be greater than 0
		CPPUNIT_ASSERT(L10 > 0 || L12 > 0 || L20 > 0 || L45 > 0);
	}	
}

void SelectionVariableTestFixture::testRandomWORRejects(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRandomWORRejects()\n");
	
	SelectionVariable var(NULL);
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	
	shared_ptr<Selection> sel( new RandomWORSelection(3) );
	var.attachSelection(sel);
//	var.setSelectionType(M_RANDOM_WOR);
//	var.setSamplingMethod(M_CYCLES);
//	var.setN(1);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c));
	
	shared_ptr<ConstantVariable> a2,b2,a3;
	
	
	if((long)var == (long)(*a)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));
	} else if((long)var == (long)(*b)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(c->getValue()));
	} else if((long)var == (long)(*c)) {
		a2 = shared_ptr<ConstantVariable>(new ConstantVariable(a->getValue()));
		b2 = shared_ptr<ConstantVariable>(new ConstantVariable(b->getValue()));
	} else {
		CPPUNIT_ASSERT(0);
	}
	
	var.acceptSelections();
	
	
	for(int i=0; i<10; ++i) {
		var.rejectSelections();
		
		CPPUNIT_ASSERT( (long)var == (long)(*a2) || 
						(long)var == (long)(*b2));
		
		
		
		if((long)var == (long)(*a2)) {
			a3 = shared_ptr<ConstantVariable>(new ConstantVariable(b2->getValue()));
		} else if((long)var == (long)(*b2)) {
			a3 = shared_ptr<ConstantVariable>(new ConstantVariable(a2->getValue()));
		} else {
			CPPUNIT_ASSERT(0);
		}
		
		
		var.nextValue();
		CPPUNIT_ASSERT( (long)var == (long)(*a3));	
	}	
	
	for(int i=0; i < 10; ++i) {
		var.nextValue();
		CPPUNIT_ASSERT( (long)var == (long)(*a3));	
	}
}

void SelectionVariableTestFixture::testRandomWORRejectsOverCycleEdge1(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRandomWORRejectsOverCycleEdge()\n");
	
	// how many times each of them is called
	long L10 = 2;
	long L20 = 2;
	
	SelectionVariable var(NULL);
	
	var.addValue(a);
	var.addValue(b);
	
	shared_ptr<Selection> sel( new RandomWORSelection(4) );
	var.attachSelection(sel);
//	var.setSelectionType(M_RANDOM_WOR);
//	var.setSamplingMethod(M_CYCLES);
//	var.setN(2);
	
	
	if((long)var == 10) {
		--L10;	
	} else {
		--L20;
	}
	
	CPPUNIT_ASSERT(L10 >= 0 && L20 >= 0);
	CPPUNIT_ASSERT(L10 <= 2 && L20 <= 2);
	
	var.acceptSelections();
	
	
	for(int i=0; i< 10; ++i) {
		std::vector<long> tentative_selections;
		
		
		
		var.nextValue();
		
		tentative_selections.push_back((long)var);
		if((long)var == 10) {
			--L10;
		} else {
			--L20;
		}
		
		CPPUNIT_ASSERT(L10 >= 0 && L20 >= 0);
		CPPUNIT_ASSERT(L10 <= 2 && L20 <= 2);
		
		var.nextValue();
		
		tentative_selections.push_back((long)var);	
		if((long)var == 10) {
			--L10;
		} else {
			--L20;
		}
		
		CPPUNIT_ASSERT(L10 >= 0 && L20 >= 0);
		CPPUNIT_ASSERT(L10 <= 2 && L20 <= 2);
		
		for(std::vector<long>::iterator j = tentative_selections.begin();
			j != tentative_selections.end();
			++j) {
			
			if(*j == 10) {
				++L10;
			} else {
				++L20;	
			}
		}
		
		CPPUNIT_ASSERT(L10 >= 0 && L20 >= 0);
		CPPUNIT_ASSERT(L10 <= 2 && L20 <= 2);
		
		var.rejectSelections();
	}
	
	var.nextValue();
	var.nextValue();	
	var.nextValue();
	
	shared_ptr<ConstantVariable> a3 = shared_ptr<ConstantVariable>(new ConstantVariable(var.getValue()));
	
	for(int i=0; i<10; ++i) {
		var.nextValue();
		CPPUNIT_ASSERT((long)var == long(*a3));
	}	
}


void SelectionVariableTestFixture::testRandomWORRejectsOverCycleEdge2(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRandomWORRejectsOverCycleEdge()\n");
	
	// how many times each of them is called
	std::vector<int> table;
	long L10 = 4;
	long L20 = 4;
	long L12 = 4;
	long L45 = 4;
	
	
	bool testRunning = true;
	
	SelectionVariable var(NULL);
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new RandomWORSelection(16) );
	var.attachSelection(sel);
	
	bool overEdge = false;
	std::vector <long> tentative_selections;
	
	for (int i=0; testRunning; ++i) {
		
		tentative_selections.push_back((long)var);
		
		if(!overEdge) {
			switch ((long)var) {
				case 10L:
					--L10;
					break;
				case 20L:
					--L20;
					break;
				case 12L:
					--L12;
					break;
				case 45L:
					--L45;
					break;
				default:
					CPPUNIT_ASSERT(0);
					break;
			}
		}
		CPPUNIT_ASSERT((L10 >= 0 &&
						L20 >= 0 &&
						L12 >= 0 &&
						L45 >= 0));
		
		switch (i) {
			case 0:
				CPPUNIT_ASSERT(var.getNDone() == 1);			
				CPPUNIT_ASSERT(var.getNLeft() == 15);			
				break;
			case 1:
				CPPUNIT_ASSERT(var.getNDone() == 2);			
				CPPUNIT_ASSERT(var.getNLeft() == 14);			
				break;
			case 2:
				CPPUNIT_ASSERT(var.getNDone() == 3);			
				CPPUNIT_ASSERT(var.getNLeft() == 13);			
				var.acceptSelections();			
				tentative_selections.clear();
				CPPUNIT_ASSERT(var.getNDone() == 3);			
				CPPUNIT_ASSERT(var.getNLeft() == 13);								
				break;
			case 3:
				CPPUNIT_ASSERT(var.getNDone() == 4);			
				CPPUNIT_ASSERT(var.getNLeft() == 12);			
				var.rejectSelections();						
				CPPUNIT_ASSERT(var.getNDone() == 4);			
				CPPUNIT_ASSERT(var.getNLeft() == 12);			
				for(std::vector<long>::iterator j = tentative_selections.begin();
					j != tentative_selections.end();
					++j) {
					
					switch(*j) {
						case  10L:
							++L10;
							break;
						case  20L:
							++L20;
							break;
						case  12L:
							++L12;
							break;
						case  45L:
							++L45;
							break;
						default:
							CPPUNIT_ASSERT(0);
							break;
					}
				}
					
					tentative_selections.clear();
				break;
			case 4:
				CPPUNIT_ASSERT(var.getNDone() == 5);			
				CPPUNIT_ASSERT(var.getNLeft() == 11);			
				break;
			case 5:
				CPPUNIT_ASSERT(var.getNDone() == 6);			
				CPPUNIT_ASSERT(var.getNLeft() == 10);			
				break;
			case 6:
				CPPUNIT_ASSERT(var.getNDone() == 7);			
				CPPUNIT_ASSERT(var.getNLeft() == 9);			
				var.rejectSelections();						
				CPPUNIT_ASSERT(var.getNDone() == 4);			
				CPPUNIT_ASSERT(var.getNLeft() == 12);			
				for(std::vector<long>::iterator j = tentative_selections.begin();
					j != tentative_selections.end();
					++j) {
					
					switch(*j) {
						case  10L:
							++L10;
							break;
						case  20L:
							++L20;
							break;
						case  12L:
							++L12;
							break;
						case  45L:
							++L45;
							break;
						default:
							CPPUNIT_ASSERT(0);
							break;
					}
				}
					
					tentative_selections.clear();
				break;
			case 7:
				CPPUNIT_ASSERT(var.getNDone() == 5);			
				CPPUNIT_ASSERT(var.getNLeft() == 11);			
				break;
			case 8:
				CPPUNIT_ASSERT(var.getNDone() == 6);			
				CPPUNIT_ASSERT(var.getNLeft() == 10);			
				break;
			case 9:
				CPPUNIT_ASSERT(var.getNDone() == 7);			
				CPPUNIT_ASSERT(var.getNLeft() == 9);			
				break;
			case 10:
				CPPUNIT_ASSERT(var.getNDone() == 8);			
				CPPUNIT_ASSERT(var.getNLeft() == 8);			
				break;
			case 11:
				CPPUNIT_ASSERT(var.getNDone() == 9);			
				CPPUNIT_ASSERT(var.getNLeft() == 7);			
				break;
			case 12:
				CPPUNIT_ASSERT(var.getNDone() == 10);			
				CPPUNIT_ASSERT(var.getNLeft() == 6);			
				break;
			case 13:
				CPPUNIT_ASSERT(var.getNDone() == 11);			
				CPPUNIT_ASSERT(var.getNLeft() == 5);			
				break;
			case 14:
				CPPUNIT_ASSERT(var.getNDone() == 12);			
				CPPUNIT_ASSERT(var.getNLeft() == 4);			
				break;
			case 15:
				CPPUNIT_ASSERT(var.getNDone() == 13);			
				CPPUNIT_ASSERT(var.getNLeft() == 3);			
				break;
			case 16:
				CPPUNIT_ASSERT(var.getNDone() == 14);			
				CPPUNIT_ASSERT(var.getNLeft() == 2);			
				break;
			case 17:
				CPPUNIT_ASSERT(var.getNDone() == 15);			
				CPPUNIT_ASSERT(var.getNLeft() == 1);			
				var.rejectSelections();						
				CPPUNIT_ASSERT(var.getNDone() == 4);			
				CPPUNIT_ASSERT(var.getNLeft() == 12);			
				for(std::vector<long>::iterator j = tentative_selections.begin();
					j != tentative_selections.end();
					++j) {
					
					switch(*j) {
						case  10L:
							++L10;
							break;
						case  20L:
							++L20;
							break;
						case  12L:
							++L12;
							break;
						case  45L:
							++L45;
							break;
						default:
							CPPUNIT_ASSERT(0);
							break;
					}
				}
					break;
				tentative_selections.clear();
				
			case 18:
				CPPUNIT_ASSERT(var.getNDone() == 5);			
				CPPUNIT_ASSERT(var.getNLeft() == 11);
				var.acceptSelections();			
				tentative_selections.clear();
				CPPUNIT_ASSERT(var.getNDone() == 5);			
				CPPUNIT_ASSERT(var.getNLeft() == 11);
				break;
			case 19:
				CPPUNIT_ASSERT(var.getNDone() == 6);			
				CPPUNIT_ASSERT(var.getNLeft() == 10);			
				var.rejectSelections();						
				CPPUNIT_ASSERT(var.getNDone() == 6);			
				CPPUNIT_ASSERT(var.getNLeft() == 10);			
				for(std::vector<long>::iterator j = tentative_selections.begin();
					j != tentative_selections.end();
					++j) {
					
					switch(*j) {
						case  10L:
							++L10;
							break;
						case  20L:
							++L20;
							break;
						case  12L:
							++L12;
							break;
						case  45L:
							++L45;
							break;
						default:
							CPPUNIT_ASSERT(0);
							break;
					}
				}
					tentative_selections.clear();
				break;
			case 20:
				CPPUNIT_ASSERT(var.getNDone() == 7);			
				CPPUNIT_ASSERT(var.getNLeft() == 9);			
				var.rejectSelections();						
				CPPUNIT_ASSERT(var.getNDone() == 6);			
				CPPUNIT_ASSERT(var.getNLeft() == 10);			
				for(std::vector<long>::iterator j = tentative_selections.begin();
					j != tentative_selections.end();
					++j) {
					
					switch(*j) {
						case  10L:
							++L10;
							break;
						case  20L:
							++L20;
							break;
						case  12L:
							++L12;
							break;
						case  45L:
							++L45;
							break;
						default:
							CPPUNIT_ASSERT(0);
							break;
					}
				}
					tentative_selections.clear();
				
				break;
			case 21:
				CPPUNIT_ASSERT(var.getNDone() == 7);			
				CPPUNIT_ASSERT(var.getNLeft() == 9);			
				var.rejectSelections();						
				CPPUNIT_ASSERT(var.getNDone() == 6);			
				CPPUNIT_ASSERT(var.getNLeft() == 10);			
				for(std::vector<long>::iterator j = tentative_selections.begin();
					j != tentative_selections.end();
					++j) {
					
					switch(*j) {
						case  10L:
							++L10;
							break;
						case  20L:
							++L20;
							break;
						case  12L:
							++L12;
							break;
						case  45L:
							++L45;
							break;
						default:
							CPPUNIT_ASSERT(0);
							break;
					}
				}
					tentative_selections.clear();
				
				break;
			case 22:
				CPPUNIT_ASSERT(var.getNDone() == 7);			
				CPPUNIT_ASSERT(var.getNLeft() == 9);			
				break;
				//			case 22:
				//				CPPUNIT_ASSERT(var.getNDone() == 8);			
				//				CPPUNIT_ASSERT(var.getNLeft() == 8);			
				//				break;
				//			case 23:
				//				CPPUNIT_ASSERT(var.getNDone() == 9);			
				//				CPPUNIT_ASSERT(var.getNLeft() == 7);			
				//				break;
				//			case 19:
				//				CPPUNIT_ASSERT(var.getNDone() == 16);			
				//				CPPUNIT_ASSERT(var.getNLeft() == 0);			
				//				CPPUNIT_ASSERT(L10 == 0 &&
				//							   L20 == 0 &&
				//							   L12 == 0 &&
				//							   L45 == 0);
				//				overEdge = true;
				//				
				//				break;
				//			case 20:
				//				CPPUNIT_ASSERT(var.getNDone() == 16);			
				//				CPPUNIT_ASSERT(var.getNLeft() == 0);			
				//				break;
				//			case 21:
				//				CPPUNIT_ASSERT(var.getNDone() == 16);			
				//				CPPUNIT_ASSERT(var.getNLeft() == 0);			
				//				break;
				
			default:
				testRunning = false;
				break;
		}
		
		var.nextValue();
	}
	
}



void SelectionVariableTestFixture::testRandomWithReplacementSelections(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRandomWithReplacementSelections()\n");
	
	SelectionVariable var(NULL);
	
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new RandomWithReplacementSelection(4) );
	var.attachSelection(sel);
	//	var.setSelectionType(M_RANDOM_WITH_REPLACEMENT);
	//	var.setN(1);
	//	var.setSamplingMethod(M_CYCLES);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	CPPUNIT_ASSERT(var.getNLeft() == 3);
	CPPUNIT_ASSERT(var.getNDone() == 1);
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 2);
	CPPUNIT_ASSERT(var.getNDone() == 2);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 1);
	CPPUNIT_ASSERT(var.getNDone() == 3);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	shared_ptr<ConstantVariable> a3 = 
		shared_ptr<ConstantVariable>(new ConstantVariable(var.getValue()));
	
	for(int i=0; i<10; ++i) {
		var.nextValue();
		CPPUNIT_ASSERT((long)var == long(*a3));
		CPPUNIT_ASSERT(var.getNLeft() == 0);
		CPPUNIT_ASSERT(var.getNDone() == 4);
		
	}	
}


void SelectionVariableTestFixture::testRandomWithReplacementCycles(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRandomWithReplacementCycles()\n");
	
	SelectionVariable var(NULL);
	
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new RandomWithReplacementSelection(8) );
	var.attachSelection(sel);
//	var.setSelectionType(M_RANDOM_WITH_REPLACEMENT);
//	var.setN(2);
//	var.setSamplingMethod(M_CYCLES);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	CPPUNIT_ASSERT(var.getNLeft() == 7);
	CPPUNIT_ASSERT(var.getNDone() == 1);
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 6);
	CPPUNIT_ASSERT(var.getNDone() == 2);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 5);
	CPPUNIT_ASSERT(var.getNDone() == 3);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 4);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 3);
	CPPUNIT_ASSERT(var.getNDone() == 5);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();

	CPPUNIT_ASSERT(var.getNLeft() == 2);
	CPPUNIT_ASSERT(var.getNDone() == 6);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();

	CPPUNIT_ASSERT(var.getNLeft() == 1);
	CPPUNIT_ASSERT(var.getNDone() == 7);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 8);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );

	shared_ptr<ConstantVariable> a3 = shared_ptr<ConstantVariable>(new ConstantVariable(var.getValue()));
	
	for(int i=0; i<10; ++i) {
		var.nextValue();
		CPPUNIT_ASSERT((long)var == long(*a3));
		CPPUNIT_ASSERT(var.getNLeft() == 0);
		CPPUNIT_ASSERT(var.getNDone() == 8);
		
	}	
}

void SelectionVariableTestFixture::testResetRandomWithReplacement(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testResetRandomWithReplacement()\n");
	
	SelectionVariable var(NULL);
	
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new RandomWithReplacementSelection(4) );
	var.attachSelection(sel);
	//	var.setSelectionType(M_RANDOM_WITH_REPLACEMENT);
	//	var.setN(1);
	//	var.setSamplingMethod(M_CYCLES);
	
	CPPUNIT_ASSERT(var.getNLeft() == 4);
	CPPUNIT_ASSERT(var.getNDone() == 0);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	CPPUNIT_ASSERT(var.getNLeft() == 3);
	CPPUNIT_ASSERT(var.getNDone() == 1);
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 2);
	CPPUNIT_ASSERT(var.getNDone() == 2);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 1);
	CPPUNIT_ASSERT(var.getNDone() == 3);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	shared_ptr<ConstantVariable> a3 = 
		shared_ptr<ConstantVariable>(new ConstantVariable(var.getValue()));
	
	for(int i=0; i<10; ++i) {
		var.nextValue();
		CPPUNIT_ASSERT((long)var == long(*a3));
		CPPUNIT_ASSERT(var.getNLeft() == 0);
		CPPUNIT_ASSERT(var.getNDone() == 4);
		
	}	
	
	var.resetSelections();

	CPPUNIT_ASSERT(var.getNLeft() == 4);
	CPPUNIT_ASSERT(var.getNDone() == 0);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	CPPUNIT_ASSERT(var.getNLeft() == 3);
	CPPUNIT_ASSERT(var.getNDone() == 1);
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 2);
	CPPUNIT_ASSERT(var.getNDone() == 2);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 1);
	CPPUNIT_ASSERT(var.getNDone() == 3);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	shared_ptr<ConstantVariable> a4 = 
		shared_ptr<ConstantVariable>(new ConstantVariable(var.getValue()));
	
	for(int i=0; i<10; ++i) {
		var.nextValue();
		CPPUNIT_ASSERT((long)var == long(*a4));
		CPPUNIT_ASSERT(var.getNLeft() == 0);
		CPPUNIT_ASSERT(var.getNDone() == 4);
		
	}		
}



void SelectionVariableTestFixture::testRandomWithReplacementRejects(){
	fprintf(stderr, "Running SelectionVariableTestFixture::testRandomWithReplacementRejects()\n");
	
	SelectionVariable var(NULL);
	
	
	var.addValue(a);
	var.addValue(b);
	var.addValue(c);
	var.addValue(d);
	
	shared_ptr<Selection> sel( new RandomWithReplacementSelection(4) );
	var.attachSelection(sel);
//	var.setSelectionType(M_RANDOM_WITH_REPLACEMENT);
//	var.setN(1);
//	var.setSamplingMethod(M_CYCLES);
	
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	CPPUNIT_ASSERT(var.getNLeft() == 3);
	CPPUNIT_ASSERT(var.getNDone() == 1);
	var.acceptSelections();	
	CPPUNIT_ASSERT(var.getNLeft() == 3);
	CPPUNIT_ASSERT(var.getNDone() == 1);
	
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 2);
	CPPUNIT_ASSERT(var.getNDone() == 2);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.rejectSelections();	
	
	CPPUNIT_ASSERT(var.getNLeft() == 2);
	CPPUNIT_ASSERT(var.getNDone() == 2);	

	
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 1);
	CPPUNIT_ASSERT(var.getNDone() == 3);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );

	var.rejectSelections();	
	
	CPPUNIT_ASSERT(var.getNLeft() == 2);
	CPPUNIT_ASSERT(var.getNDone() == 2);	
	

	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 1);
	CPPUNIT_ASSERT(var.getNDone() == 3);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );

	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	var.rejectSelections();	
	
	CPPUNIT_ASSERT(var.getNLeft() == 2);
	CPPUNIT_ASSERT(var.getNDone() == 2);	

	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 1);
	CPPUNIT_ASSERT(var.getNDone() == 3);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	
	var.nextValue();
	
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	CPPUNIT_ASSERT( (long)var == (long)(*a) || 
					(long)var == (long)(*b) ||
					(long)var == (long)(*c) ||
					(long)var == (long)(*d) );
	
	
	shared_ptr<ConstantVariable> a3 = shared_ptr<ConstantVariable>(new ConstantVariable(var.getValue()));
	
	CPPUNIT_ASSERT((long)var == long(*a3));
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	var.acceptSelections();
	
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	var.nextValue();
	
	CPPUNIT_ASSERT((long)var == long(*a3));
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
	
	var.rejectSelections();	
	CPPUNIT_ASSERT((long)var == long(*a3));
	CPPUNIT_ASSERT(var.getNLeft() == 0);
	CPPUNIT_ASSERT(var.getNDone() == 4);
}

