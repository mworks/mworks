/*
 *  GenericDataTest.cpp
 *  MonkeyWorksCore
 *
 *  Created by Ben Kennedy 2006
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "GenericDataTest.h"
using namespace mw;

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( GenericDataTestFixture, "Unit Test" );
//CPPUNIT_TEST_SUITE_REGISTRATION( GenericDataTestFixture );


void *hammerit(void *thedatum){
	Data *datum = (Data *)thedatum;
	for(int i = 0; i < LOTS; i++){
		*datum = (long)i;
	}
	
	return 0;
}


void *hammerlist(void *thedatum){
	Data *datum = (Data *)thedatum;
	for(int i = 0; i < LOTS; i++){
		datum->setElement(0, Data((long)i));
	}

	return 0;
}

void *hammerdict(void *thedatum){
	Data *datum = (Data *)thedatum;
	for(int i = 0; i < LOTS; i++){
		datum->addElement("test", Data((long)i));
	}

	return 0;
}

void GenericDataTestFixture::testAnyScalarConstructorBool() {
	stx::AnyScalar the_bool_false(false);
	stx::AnyScalar the_bool_true(true);
	
	Data the_data_bool_false(the_bool_false);
	Data the_data_bool_true(the_bool_true);
	
	CPPUNIT_ASSERT(!the_data_bool_false.getBool());
	CPPUNIT_ASSERT(the_data_bool_false.isBool());
	CPPUNIT_ASSERT(the_data_bool_true.getBool());
	CPPUNIT_ASSERT(the_data_bool_true.isBool());
}

void GenericDataTestFixture::testAnyScalarConstructorInt() {
	int zero = 0;
	int minus_1 = -1;
	int six = 6;
	
	stx::AnyScalar the_int0(zero);
	stx::AnyScalar the_int6(six);
	stx::AnyScalar the_int_minus1(minus_1);	
	
	Data the_data_int0(the_int0);
	CPPUNIT_ASSERT(the_data_int0 == 0L);
	CPPUNIT_ASSERT(the_data_int0.isInteger());
	
	Data the_data_int6(the_int6);
	CPPUNIT_ASSERT(the_data_int6 == 6L);
	CPPUNIT_ASSERT(the_data_int6.isInteger());
	
	Data the_data_int_minus1(the_int_minus1);
	CPPUNIT_ASSERT(the_data_int_minus1 == -1L);
	CPPUNIT_ASSERT(the_data_int_minus1.isInteger());
	
}

void GenericDataTestFixture::testAnyScalarConstructorLong() {
	long zero = 0;
	long minus_1 = -1;
	long six = 6;
	
	stx::AnyScalar the_long0(zero);
	stx::AnyScalar the_long6(six);
	stx::AnyScalar the_long_minus1(minus_1);	
	
	Data the_data_long0(the_long0);
	CPPUNIT_ASSERT(the_data_long0 == 0L);
	CPPUNIT_ASSERT(the_data_long0.isInteger());
	
	Data the_data_long6(the_long6);
	CPPUNIT_ASSERT(the_data_long6 == 6L);
	CPPUNIT_ASSERT(the_data_long6.isInteger());
	
	Data the_data_long_minus1(the_long_minus1);
	CPPUNIT_ASSERT(the_data_long_minus1 == -1L);
	CPPUNIT_ASSERT(the_data_long_minus1.isInteger());
}

void GenericDataTestFixture::testAnyScalarConstructorByte() {
	unsigned char zero = 0x0;
	unsigned char ff = 0xff;
	
	stx::AnyScalar the_byte0(zero);
	stx::AnyScalar the_byte255(ff);
	
	Data the_data_byte0(the_byte0);
	CPPUNIT_ASSERT(the_data_byte0 == 0x0L);
	CPPUNIT_ASSERT(the_data_byte0.isInteger());
	
	Data the_data_byte255(the_byte255);
	CPPUNIT_ASSERT(the_data_byte255 == 0xFFL);
	CPPUNIT_ASSERT(the_data_byte255.isInteger());
}

void GenericDataTestFixture::testAnyScalarConstructorShort() {
	short zero = 0;
	short minus_1 = -1;
	short six = 6;

	stx::AnyScalar the_short0(zero);
	stx::AnyScalar the_short6(six);
	stx::AnyScalar the_short_minus1(minus_1);	
	
	Data the_data_short0(the_short0);
	CPPUNIT_ASSERT(the_data_short0 == 0L);
	CPPUNIT_ASSERT(the_data_short0.isInteger());
	
	Data the_data_short6(the_short6);
	CPPUNIT_ASSERT(the_data_short6 == 6L);
	CPPUNIT_ASSERT(the_data_short6.isInteger());
	
	Data the_data_short_minus1(the_short_minus1);
	CPPUNIT_ASSERT(the_data_short_minus1 == -1L);
	CPPUNIT_ASSERT(the_data_short_minus1.isInteger());
}

void GenericDataTestFixture::testAnyScalarConstructorWord() {}
void GenericDataTestFixture::testAnyScalarConstructorDWord() {}
void GenericDataTestFixture::testAnyScalarConstructorQWord() {}

void GenericDataTestFixture::testAnyScalarConstructorFloat() {
	float zero = 0;
	float minus_1 = -1;
	float six = 6;
	float six_point_5 = 6.5;
	
	stx::AnyScalar the_float0(zero);
	stx::AnyScalar the_float6(six);
	stx::AnyScalar the_float_minus1(minus_1);	
	stx::AnyScalar the_float_six_point_5(six_point_5);	
	
	Data the_data_float0(the_float0);
	CPPUNIT_ASSERT(the_data_float0 == 0.0);
	CPPUNIT_ASSERT(the_data_float0.isFloat());
	
	Data the_data_float6(the_float6);
	CPPUNIT_ASSERT(the_data_float6 == 6.0);
	CPPUNIT_ASSERT(the_data_float6.isFloat());
	
	Data the_data_float_minus1(the_float_minus1);
	CPPUNIT_ASSERT(the_data_float_minus1 == -1.0);	
	CPPUNIT_ASSERT(the_data_float_minus1.isFloat());

	Data the_data_float_six_point_5(the_float_six_point_5);
	CPPUNIT_ASSERT(the_data_float_six_point_5 == 6.5);	
	CPPUNIT_ASSERT(the_data_float_six_point_5.isFloat());
}

void GenericDataTestFixture::testAnyScalarConstructorDouble() {
	double zero = 0;
	double minus_1 = -1;
	double six = 6;
	double six_point_5 = 6.5;
	
	stx::AnyScalar the_double0(zero);
	stx::AnyScalar the_double6(six);
	stx::AnyScalar the_double_minus1(minus_1);	
	stx::AnyScalar the_double_six_point_5(six_point_5);	
	
	Data the_data_double0(the_double0);
	CPPUNIT_ASSERT(the_data_double0 == 0.0);
	CPPUNIT_ASSERT(the_data_double0.isFloat());
	
	Data the_data_double6(the_double6);
	CPPUNIT_ASSERT(the_data_double6 == 6.0);
	CPPUNIT_ASSERT(the_data_double6.isFloat());
	
	Data the_data_double_minus1(the_double_minus1);
	CPPUNIT_ASSERT(the_data_double_minus1 == -1.0);	
	CPPUNIT_ASSERT(the_data_double_minus1.isFloat());
	
	Data the_data_double_six_point_5(the_double_six_point_5);
	CPPUNIT_ASSERT(the_data_double_six_point_5 == 6.5);		
	CPPUNIT_ASSERT(the_data_double_six_point_5.isFloat());
}


void GenericDataTestFixture::testAnyScalarConstructorString() {
	std::string zero("0");
	std::string six("6");
	std::string minus_1("-1");
	
	stx::AnyScalar the_string0(zero);
	stx::AnyScalar the_string6(six);
	stx::AnyScalar the_string_minus1(minus_1);	
	
	Data the_data_string0(the_string0);
	CPPUNIT_ASSERT(the_data_string0 == "0");
	CPPUNIT_ASSERT(the_data_string0.isString());
	Data the_data_string6(the_string6);
	CPPUNIT_ASSERT(the_data_string6 == "6");
	CPPUNIT_ASSERT(the_data_string6.isString());
	Data the_data_string_minus1(the_string_minus1);
	CPPUNIT_ASSERT(the_data_string_minus1 == "-1");
	CPPUNIT_ASSERT(the_data_string_minus1.isString());
}

void GenericDataTestFixture::testAnyScalarConstructorChar() {}

void GenericDataTestFixture::testAnyScalarConstructorInvalid() {
	stx::AnyScalar invalid;

	Data the_data_invalid(invalid);
	CPPUNIT_ASSERT(the_data_invalid.isUndefined());
}



void GenericDataTestFixture::testAnyScalarEqualBool() {
	stx::AnyScalar the_bool_false(false);
	stx::AnyScalar the_bool_true(true);
	
	Data the_data_bool_false;
	CPPUNIT_ASSERT(the_data_bool_false.isUndefined());
	the_data_bool_false = the_bool_false;
	CPPUNIT_ASSERT(!the_data_bool_false.getBool());
	CPPUNIT_ASSERT(the_data_bool_false.isBool());

	Data the_data_bool_true;
	CPPUNIT_ASSERT(the_data_bool_true.isUndefined());
	the_data_bool_true = the_bool_true;	
	CPPUNIT_ASSERT(the_data_bool_true.getBool());
	CPPUNIT_ASSERT(the_data_bool_true.isBool());
}

void GenericDataTestFixture::testAnyScalarEqualInt() {
	int zero = 0;
	int minus_1 = -1;
	int six = 6;
	
	stx::AnyScalar the_int0(zero);
	stx::AnyScalar the_int6(six);
	stx::AnyScalar the_int_minus1(minus_1);	
	
	Data the_data_int0;
	CPPUNIT_ASSERT(the_data_int0.isUndefined());
	the_data_int0 = the_int0;
	CPPUNIT_ASSERT(the_data_int0 == 0L);
	CPPUNIT_ASSERT(the_data_int0.isInteger());
	
	Data the_data_int6;
	CPPUNIT_ASSERT(the_data_int6.isUndefined());
	the_data_int6 = the_int6;
	CPPUNIT_ASSERT(the_data_int6 == 6L);
	CPPUNIT_ASSERT(the_data_int6.isInteger());
	
	Data the_data_int_minus1;
	CPPUNIT_ASSERT(the_data_int_minus1.isUndefined());
	the_data_int_minus1 = the_int_minus1;
	CPPUNIT_ASSERT(the_data_int_minus1 == -1L);
	CPPUNIT_ASSERT(the_data_int_minus1.isInteger());
	
}

void GenericDataTestFixture::testAnyScalarEqualLong() {
	long zero = 0;
	long minus_1 = -1;
	long six = 6;
	
	stx::AnyScalar the_long0(zero);
	stx::AnyScalar the_long6(six);
	stx::AnyScalar the_long_minus1(minus_1);	
	
	Data the_data_long0;
	CPPUNIT_ASSERT(the_data_long0.isUndefined());
	the_data_long0 = the_long0;
	CPPUNIT_ASSERT(the_data_long0 == 0L);
	CPPUNIT_ASSERT(the_data_long0.isInteger());
	
	Data the_data_long6;
	CPPUNIT_ASSERT(the_data_long6.isUndefined());
	the_data_long6 = the_long6;
	CPPUNIT_ASSERT(the_data_long6 == 6L);
	CPPUNIT_ASSERT(the_data_long6.isInteger());
	
	Data the_data_long_minus1;
	CPPUNIT_ASSERT(the_data_long_minus1.isUndefined());
	the_data_long_minus1 = the_long_minus1;
	CPPUNIT_ASSERT(the_data_long_minus1 == -1L);
	CPPUNIT_ASSERT(the_data_long_minus1.isInteger());
}

void GenericDataTestFixture::testAnyScalarEqualByte() {
	unsigned char zero = 0x0;
	unsigned char ff = 0xff;
	
	stx::AnyScalar the_byte0(zero);
	stx::AnyScalar the_byte255(ff);
	
	Data the_data_byte0;
	CPPUNIT_ASSERT(the_data_byte0.isUndefined());
	the_data_byte0 = the_byte0;
	CPPUNIT_ASSERT(the_data_byte0 == 0x0L);
	CPPUNIT_ASSERT(the_data_byte0.isInteger());
	
	Data the_data_byte255;
	CPPUNIT_ASSERT(the_data_byte255.isUndefined());
	the_data_byte255 = the_byte255;
	CPPUNIT_ASSERT(the_data_byte255 == 0xFFL);
	CPPUNIT_ASSERT(the_data_byte255.isInteger());
}

void GenericDataTestFixture::testAnyScalarEqualShort() {
	short zero = 0;
	short minus_1 = -1;
	short six = 6;

	stx::AnyScalar the_short0(zero);
	stx::AnyScalar the_short6(six);
	stx::AnyScalar the_short_minus1(minus_1);	
	
	Data the_data_short0;
	CPPUNIT_ASSERT(the_data_short0.isUndefined());
	the_data_short0 = the_short0;
	CPPUNIT_ASSERT(the_data_short0 == 0L);
	CPPUNIT_ASSERT(the_data_short0.isInteger());
	
	Data the_data_short6;
	CPPUNIT_ASSERT(the_data_short6.isUndefined());
	the_data_short6 = the_short6;
	CPPUNIT_ASSERT(the_data_short6 == 6L);
	CPPUNIT_ASSERT(the_data_short6.isInteger());
	
	Data the_data_short_minus1;
	CPPUNIT_ASSERT(the_data_short_minus1.isUndefined());
	the_data_short_minus1 = the_short_minus1;
	CPPUNIT_ASSERT(the_data_short_minus1 == -1L);
	CPPUNIT_ASSERT(the_data_short_minus1.isInteger());
}

void GenericDataTestFixture::testAnyScalarEqualWord() {}
void GenericDataTestFixture::testAnyScalarEqualDWord() {}
void GenericDataTestFixture::testAnyScalarEqualQWord() {}

void GenericDataTestFixture::testAnyScalarEqualFloat() {
	float zero = 0;
	float minus_1 = -1;
	float six = 6;
	float six_point_5 = 6.5;
	
	stx::AnyScalar the_float0(zero);
	stx::AnyScalar the_float6(six);
	stx::AnyScalar the_float_minus1(minus_1);	
	stx::AnyScalar the_float_six_point_5(six_point_5);	
	
	Data the_data_float0;
	CPPUNIT_ASSERT(the_data_float0.isUndefined());
	the_data_float0 = the_float0;
	CPPUNIT_ASSERT(the_data_float0 == 0.0);
	CPPUNIT_ASSERT(the_data_float0.isFloat());
	
	Data the_data_float6;
	CPPUNIT_ASSERT(the_data_float6.isUndefined());
	the_data_float6 = the_float6;
	CPPUNIT_ASSERT(the_data_float6 == 6.0);
	CPPUNIT_ASSERT(the_data_float6.isFloat());
	
	Data the_data_float_minus1;
	CPPUNIT_ASSERT(the_data_float_minus1.isUndefined());
	the_data_float_minus1 = the_float_minus1;
	CPPUNIT_ASSERT(the_data_float_minus1 == -1.0);	
	CPPUNIT_ASSERT(the_data_float_minus1.isFloat());

	Data the_data_float_six_point_5;
	CPPUNIT_ASSERT(the_data_float_six_point_5.isUndefined());
	the_data_float_six_point_5 = the_float_six_point_5;
	CPPUNIT_ASSERT(the_data_float_six_point_5 == 6.5);	
	CPPUNIT_ASSERT(the_data_float_six_point_5.isFloat());
}

void GenericDataTestFixture::testAnyScalarEqualDouble() {
	double zero = 0;
	double minus_1 = -1;
	double six = 6;
	double six_point_5 = 6.5;
	
	stx::AnyScalar the_double0(zero);
	stx::AnyScalar the_double6(six);
	stx::AnyScalar the_double_minus1(minus_1);	
	stx::AnyScalar the_double_six_point_5(six_point_5);	
	
	Data the_data_double0;
	CPPUNIT_ASSERT(the_data_double0.isUndefined());
	the_data_double0 = the_double0;
	CPPUNIT_ASSERT(the_data_double0 == 0.0);
	CPPUNIT_ASSERT(the_data_double0.isFloat());
	
	Data the_data_double6;
	CPPUNIT_ASSERT(the_data_double6.isUndefined());
	the_data_double6 = the_double6;
	CPPUNIT_ASSERT(the_data_double6 == 6.0);
	CPPUNIT_ASSERT(the_data_double6.isFloat());
	
	Data the_data_double_minus1;
	CPPUNIT_ASSERT(the_data_double_minus1.isUndefined());
	the_data_double_minus1 = the_double_minus1;
	CPPUNIT_ASSERT(the_data_double_minus1 == -1.0);	
	CPPUNIT_ASSERT(the_data_double_minus1.isFloat());
	
	Data the_data_double_six_point_5;
	CPPUNIT_ASSERT(the_data_double_six_point_5.isUndefined());
	the_data_double_six_point_5 = the_double_six_point_5;
	CPPUNIT_ASSERT(the_data_double_six_point_5 == 6.5);		
	CPPUNIT_ASSERT(the_data_double_six_point_5.isFloat());
}


void GenericDataTestFixture::testAnyScalarEqualString() {
	std::string zero("0");
	std::string six("6");
	std::string minus_1("-1");
	
	stx::AnyScalar the_string0(zero);
	stx::AnyScalar the_string6(six);
	stx::AnyScalar the_string_minus1(minus_1);	
	
	Data the_data_string0;
	CPPUNIT_ASSERT(the_data_string0.isUndefined());
	the_data_string0 = the_string0;
	CPPUNIT_ASSERT(the_data_string0 == "0");
	CPPUNIT_ASSERT(the_data_string0.isString());
	Data the_data_string6;
	CPPUNIT_ASSERT(the_data_string6.isUndefined());
	the_data_string6 = the_string6;
	CPPUNIT_ASSERT(the_data_string6 == "6");
	CPPUNIT_ASSERT(the_data_string6.isString());
	Data the_data_string_minus1;
	CPPUNIT_ASSERT(the_data_string_minus1.isUndefined());
	the_data_string_minus1 = the_string_minus1;
	CPPUNIT_ASSERT(the_data_string_minus1 == "-1");
	CPPUNIT_ASSERT(the_data_string_minus1.isString());
}

void GenericDataTestFixture::testAnyScalarEqualChar() {}

void GenericDataTestFixture::testAnyScalarEqualInvalid() {
	stx::AnyScalar invalid;

	Data the_data_invalid;
	CPPUNIT_ASSERT(the_data_invalid.isUndefined());

	the_data_invalid = invalid;
	CPPUNIT_ASSERT(the_data_invalid.isUndefined());
}

void GenericDataTestFixture::testIs() {
	Data an_undefined;
	CPPUNIT_ASSERT(an_undefined.isUndefined());
	CPPUNIT_ASSERT(!an_undefined.isNumber());

	Data a_float(M_FLOAT, 0.0);
	CPPUNIT_ASSERT(a_float.isFloat());
	CPPUNIT_ASSERT(a_float.isNumber());

	Data an_int = 1L;
	CPPUNIT_ASSERT(an_int.isInteger());
	CPPUNIT_ASSERT(an_int.isNumber());

	Data a_bool(M_BOOLEAN, false);
	CPPUNIT_ASSERT(a_bool.isBool());
	CPPUNIT_ASSERT(a_bool.isNumber());

	Data a_string = "";
	CPPUNIT_ASSERT(a_string.isString());
	CPPUNIT_ASSERT(!a_string.isNumber());

	Data a_list(M_LIST, 1);
	CPPUNIT_ASSERT(a_list.isList());
	CPPUNIT_ASSERT(!a_list.isNumber());

	Data a_dict(M_DICTIONARY, 1);
	CPPUNIT_ASSERT(a_dict.isDictionary());
	CPPUNIT_ASSERT(!a_dict.isNumber());
}

