/*
 *  GenericDataTest.cpp
 *  MWorksCore
 *
 *  Created by Ben Kennedy 2006
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "GenericDataTest.h"


BEGIN_NAMESPACE_MW


CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( GenericDataTestFixture, "Unit Test" );


void *hammerit(void *thedatum){
 Datum *datum = (Datum *)thedatum;
	for(int i = 0; i < LOTS; i++){
		*datum = (long)i;
	}
	
	return 0;
}


void *hammerlist(void *thedatum){
 Datum *datum = (Datum *)thedatum;
	for(int i = 0; i < LOTS; i++){
		datum->setElement(0, Datum((long)i));
	}

	return 0;
}

void *hammerdict(void *thedatum){
 Datum *datum = (Datum *)thedatum;
	for(int i = 0; i < LOTS; i++){
		datum->addElement("test", Datum((long)i));
	}

	return 0;
}


void GenericDataTestFixture::testIs() {
 Datum an_undefined;
	CPPUNIT_ASSERT(an_undefined.isUndefined());
	CPPUNIT_ASSERT(!an_undefined.isNumber());

 Datum a_float(M_FLOAT, 0.0);
	CPPUNIT_ASSERT(a_float.isFloat());
	CPPUNIT_ASSERT(a_float.isNumber());

 Datum an_int = 1L;
	CPPUNIT_ASSERT(an_int.isInteger());
	CPPUNIT_ASSERT(an_int.isNumber());

 Datum a_bool(M_BOOLEAN, false);
	CPPUNIT_ASSERT(a_bool.isBool());
	CPPUNIT_ASSERT(a_bool.isNumber());

 Datum a_string = "";
	CPPUNIT_ASSERT(a_string.isString());
	CPPUNIT_ASSERT(!a_string.isNumber());

 Datum a_list(M_LIST, 1);
	CPPUNIT_ASSERT(a_list.isList());
	CPPUNIT_ASSERT(!a_list.isNumber());

 Datum a_dict(M_DICTIONARY, 1);
	CPPUNIT_ASSERT(a_dict.isDictionary());
	CPPUNIT_ASSERT(!a_dict.isNumber());
}


END_NAMESPACE_MW
