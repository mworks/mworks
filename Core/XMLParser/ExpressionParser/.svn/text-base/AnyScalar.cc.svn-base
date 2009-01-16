// $Id: AnyScalar.cc 59 2007-07-17 14:43:23Z tb $

/*
 * STX Expression Parser C++ Framework v0.7
 * Copyright (C) 2007 Timo Bingmann
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation; either version 2.1 of the License, or (at your
 * option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/** \file AnyScalar.cc
 * Implementation of the typed scalar value class AnyScalar used by the parser
 * to represent values.
 */

#include "AnyScalar.h"
#include "ExpressionParser.h"

#include <stdlib.h>
#include <functional>
#include <algorithm>

#include <boost/lexical_cast.hpp>

namespace stx {

bool AnyScalar::operator==(const AnyScalar &a) const
{
    if (atype != a.atype) return false;

    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
	return (val._int == a.val._int);

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
	return (val._uint == a.val._uint);

    case ATTRTYPE_LONG:
	return (val._long == a.val._long);

    case ATTRTYPE_QWORD:
	return (val._ulong == a.val._ulong);

    case ATTRTYPE_FLOAT:
	return (val._float == a.val._float);

    case ATTRTYPE_DOUBLE:
	return (val._double == a.val._double);

    case ATTRTYPE_STRING:
	assert(val._string && a.val._string);
	return (*val._string == *a.val._string);
    }

    assert(0);
    return false;
}

AnyScalar::attrtype_t AnyScalar::stringToType(const char* s)
{
    std::string str = s;
    std::transform(str.begin(), str.end(), str.begin(), tolower);

    // Stupid method, but maybe faster than using a biggy std::map with a
    // case-insensitive equality.

    if (str == "bool")
	return ATTRTYPE_BOOL;

    if (str == "char")
	return ATTRTYPE_CHAR;
    if (str == "short")
	return ATTRTYPE_SHORT;
    if (str == "integer" || str == "int")
	return ATTRTYPE_INTEGER;
    if (str == "long")
	return ATTRTYPE_LONG;

    if (str == "byte")
	return ATTRTYPE_BYTE;
    if (str == "word")
	return ATTRTYPE_WORD;
    if (str == "dword")
	return ATTRTYPE_DWORD;
    if (str == "qword")
	return ATTRTYPE_QWORD;
    
    if (str == "float")
	return ATTRTYPE_FLOAT;
    if (str == "double")
	return ATTRTYPE_DOUBLE;

    if (str == "string")
	return ATTRTYPE_STRING;

    throw(ConversionException("Invalid type name."));
}

bool AnyScalar::isValidAttrtype(attrtype_t at)
{
    switch(at)
    {
    default:
    case ATTRTYPE_INVALID:
	return false;

    case ATTRTYPE_BOOL:
    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
    case ATTRTYPE_LONG:
    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
    case ATTRTYPE_QWORD:
    case ATTRTYPE_FLOAT:
    case ATTRTYPE_DOUBLE:
    case ATTRTYPE_STRING:
	return true;
    }
}

std::string AnyScalar::getTypeString(attrtype_t at)
{
    switch(at)
    {
    case ATTRTYPE_INVALID:	return "invalid";
    case ATTRTYPE_BOOL:		return "bool";
    case ATTRTYPE_CHAR:		return "char";
    case ATTRTYPE_SHORT:	return "short";
    case ATTRTYPE_INTEGER:	return "integer";
    case ATTRTYPE_LONG:		return "long";
    case ATTRTYPE_BYTE:		return "byte";
    case ATTRTYPE_WORD:		return "word";
    case ATTRTYPE_DWORD:	return "dword";
    case ATTRTYPE_QWORD:	return "qword";
    case ATTRTYPE_FLOAT:	return "float";
    case ATTRTYPE_DOUBLE:	return "double";
    case ATTRTYPE_STRING:	return "string";
    }
    return "unknown";
}

int AnyScalar::getTypeLength(attrtype_t t)
{
    switch(t)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return 0;

    case ATTRTYPE_BOOL:
	// weird value. beware!
	return 0;

    case ATTRTYPE_CHAR:
	return sizeof(char);

    case ATTRTYPE_BYTE:
	return sizeof(unsigned char);

    case ATTRTYPE_SHORT:
	return sizeof(short);

    case ATTRTYPE_WORD:
	return sizeof(unsigned short);

    case ATTRTYPE_INTEGER:
	return sizeof(int);

    case ATTRTYPE_DWORD:
	return sizeof(unsigned int);

    case ATTRTYPE_LONG:
	return sizeof(long long);

    case ATTRTYPE_QWORD:
	return sizeof(unsigned long long);

    case ATTRTYPE_FLOAT:
	return sizeof(float);

    case ATTRTYPE_DOUBLE:
	return sizeof(double);

    case ATTRTYPE_STRING:
	return -1;

    }
    assert(0);
    return -1;
}

unsigned int AnyScalar::getValueLength() const
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return 0;

    case ATTRTYPE_BOOL:
	// weird value. beware!
	return 0;

    case ATTRTYPE_CHAR:
	return sizeof(char);

    case ATTRTYPE_BYTE:
	return sizeof(unsigned char);

    case ATTRTYPE_SHORT:
	return sizeof(short);

    case ATTRTYPE_WORD:
	return sizeof(unsigned short);

    case ATTRTYPE_INTEGER:
	return sizeof(int);

    case ATTRTYPE_DWORD:
	return sizeof(unsigned int);

    case ATTRTYPE_LONG:
	return sizeof(long long);

    case ATTRTYPE_QWORD:
	return sizeof(unsigned long long);

    case ATTRTYPE_FLOAT:
	return sizeof(float);

    case ATTRTYPE_DOUBLE:
	return sizeof(double);

    case ATTRTYPE_STRING:
	assert(val._string);
	return sizeof(unsigned char) + static_cast<unsigned int>(val._string->size());
    }

    assert(0);
    return 0;
}

bool AnyScalar::setInteger(int i)
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
	val._int = (i != 0) ? 1 : 0;
	return true;

    case ATTRTYPE_CHAR:
	if (SCHAR_MIN <= i && i <= SCHAR_MAX) {
	    val._int = i;
	    return true;
	}
	if (SCHAR_MIN > i) val._int = SCHAR_MIN;
	if (i > SCHAR_MAX) val._int = SCHAR_MAX;
	return false;

    case ATTRTYPE_BYTE:
	if (i <= UCHAR_MAX) {
	    val._uint = static_cast<unsigned char>(i);
	    return true;
	}
	if (0 > i) val._uint = 0;
	if (i > UCHAR_MAX) val._uint = UCHAR_MAX;
	return false;

    case ATTRTYPE_SHORT:
	if (SHRT_MIN <= i && i <= SHRT_MAX) {
	    val._int = i;
	    return true;
	}
	if (SHRT_MIN > i) val._int = SHRT_MIN;
	if (i > SHRT_MAX) val._int = SHRT_MAX;
	return false;

    case ATTRTYPE_WORD:
	if (i <= USHRT_MAX) {
	    val._uint = static_cast<unsigned short>(i);
	    return true;
	}
	if (0 > i) val._uint = 0;
	if (i > USHRT_MAX) val._uint = USHRT_MAX;
	return false;

    case ATTRTYPE_INTEGER:
	if (INT_MIN <= i && i <= INT_MAX) {
	    val._int = i;
	    return true;
	}
	if (INT_MIN > i) val._int = INT_MIN;
	if (i > INT_MAX) val._int = INT_MAX;
	return false;

    case ATTRTYPE_DWORD:
	if (static_cast<unsigned int>(i) <= UINT_MAX) {
	    val._uint = i;
	    return true;
	}
	if (0 > i) val._uint = 0;
	return false;

    case ATTRTYPE_LONG:
	val._long = i;
	return true;

    case ATTRTYPE_QWORD:
	val._ulong = i;
	return true;

    case ATTRTYPE_FLOAT:
	val._float = static_cast<float>(i);
	return true;

    case ATTRTYPE_DOUBLE:
	val._double = static_cast<double>(i);
	return true;

    case ATTRTYPE_STRING:
	assert(val._string);
	*val._string = boost::lexical_cast<std::string>(i);
	return true;
    }

    assert(0);
    return false;
}

bool AnyScalar::setLong(long long l)
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
	val._int = (l != 0) ? 1 : 0;
	return true;

    case ATTRTYPE_CHAR:
	if (SCHAR_MIN <= l && l <= SCHAR_MAX) {
	    val._int = static_cast<char>(l);
	    return true;
	}
	if (SCHAR_MIN > l) val._int = SCHAR_MIN;
	if (l > SCHAR_MAX) val._int = SCHAR_MAX;
	return false;

    case ATTRTYPE_BYTE:
	if (0 <= l && l <= UCHAR_MAX) {
	    val._uint = static_cast<unsigned char>(l);
	    return true;
	}
	if (0 > l) val._uint = 0;
	if (l > UCHAR_MAX) val._uint = UCHAR_MAX;
	return false;

    case ATTRTYPE_SHORT:
	if (SHRT_MIN <= l && l <= SHRT_MAX) {
	    val._int = static_cast<short>(l);
	    return true;
	}
	if (SHRT_MIN > l) val._int = SHRT_MIN;
	if (l > SHRT_MAX) val._int = SHRT_MAX;
	return false;

    case ATTRTYPE_WORD:
	if (l <= USHRT_MAX) {
	    val._uint = static_cast<unsigned short>(l);
	    return true;
	}
	if (0 > l) val._uint = 0;
	if (l > USHRT_MAX) val._uint = USHRT_MAX;
	return false;

    case ATTRTYPE_INTEGER:
	if (INT_MIN <= l && l <= INT_MAX) {
	    val._int = static_cast<int>(l);
	    return true;
	}
	if (INT_MIN > l) val._int = INT_MIN;
	if (l > INT_MAX) val._int = INT_MAX;
	return false;

    case ATTRTYPE_DWORD:
	if (static_cast<unsigned int>(l) <= UINT_MAX) {
	    val._uint = static_cast<unsigned int>(l);
	    return true;
	}
	if (0 > l) val._uint = 0;
	if (l > UINT_MAX) val._uint = UINT_MAX;
	return false;

    case ATTRTYPE_LONG:
	val._long = l;
	return true;

    case ATTRTYPE_QWORD:
	val._ulong = l;
	return true;

    case ATTRTYPE_FLOAT:
	val._float = static_cast<float>(l);
	return true;

    case ATTRTYPE_DOUBLE:
	val._double = static_cast<double>(l);
	return true;

    case ATTRTYPE_STRING:
	assert(val._string);
	*val._string = boost::lexical_cast<std::string>(l);
	return true;
    }

    assert(0);
    return false;
}

bool AnyScalar::setDouble(double d)
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
	if (0 <= d && d < 0.5) {
	    val._int = 0;
	    return true;
	}
	if (0.5 <= d && d <= 1) {
	    val._int = 1;
	    return true;
	}
	val._int = 1;
	return false;

    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
	return setInteger( static_cast<int>(d) );

    case ATTRTYPE_LONG:
	return setLong( static_cast<long long>(d) );

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
	return setInteger( static_cast<unsigned int>(d) );

    case ATTRTYPE_QWORD:
	return setLong( static_cast<unsigned long long>(d) );

    case ATTRTYPE_FLOAT:
	val._float = static_cast<float>(d);
	return true;

    case ATTRTYPE_DOUBLE:
	val._double = d;
	return true;

    case ATTRTYPE_STRING:
	assert(val._string);
	*val._string = boost::lexical_cast<std::string>(d);
	return true;
    }

    assert(0);
    return false;
}

bool AnyScalar::setString(const std::string &s)
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
	if (s == "0" || s == "f" || s == "false" || s == "n" || s == "no") {
	    val._int = 0;
	    return true;
	}
	if (s == "1" || s == "t" || s == "true" || s == "y" || s == "yes") {
	    val._int = 1;
	    return true;
	}
	return false;

    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
    {
	char *endptr;
	long i = strtol(s.c_str(), &endptr, 10);
	if (endptr != NULL && *endptr == 0)
	    return setInteger(i);
	return false;
    }

    case ATTRTYPE_LONG:
    {
	char *endptr;
#ifndef _MSC_VER
	long long l = strtoll(s.c_str(), &endptr, 10);
#else
	long long l = _strtoi64(s.c_str(), &endptr, 10);
#endif
	if (endptr != NULL && *endptr == 0)
	    return setLong(l);
	return false;
    }

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
    {
	char *endptr;
	unsigned long u = strtoul(s.c_str(), &endptr, 10);
	if (endptr != NULL && *endptr == 0)
	    return setInteger(u);
	return false;
    }
    
    case ATTRTYPE_QWORD:
    {
	char *endptr;
#ifndef _MSC_VER
	unsigned long long u = strtoull(s.c_str(), &endptr, 10);
#else
	unsigned long long u = _strtoui64(s.c_str(), &endptr, 10);
#endif
	if (endptr != NULL && *endptr == 0)
	    return setLong(u);
	return false;
    }

    case ATTRTYPE_FLOAT:
    {
	char *endptr;
	float f = static_cast<float>(strtod(s.c_str(), &endptr));
	if (endptr != NULL && *endptr == 0) {
	    val._float = f;
	    return true;
	}
	return false;
    }

    case ATTRTYPE_DOUBLE:
    {
	char *endptr;
	double d = strtod(s.c_str(), &endptr);
	if (endptr != NULL && *endptr == 0) {
	    val._double = d;
	    return true;
	}
	return false;
    }

    case ATTRTYPE_STRING:
    {
	assert(val._string);
	*val._string = s;
	return true;
    }
    }

    assert(0);
    return false;
}

bool AnyScalar::setStringQuoted(const std::string &s)
{
    // unescape string
    if (s.size() < 2) return false;
    if (s[0] != '"') return false;
    if (s[s.size()-1] != '"') return false;

    std::string t;
    t.reserve(s.size() + s.size() / 32);

    for(unsigned int i = 1; i + 1 < s.size(); i++)
    {
	if (s[i] == '\\')
	{
	    i++;
	    if (i >= s.size() - 1) return false;

	    switch(s[i])
	    {
	    case 'a':	t += '\a';	break;
	    case 'b':	t += '\b';	break;
	    case 'f':	t += '\f';	break;
	    case 'n':	t += '\n';	break;
	    case 'r':	t += '\r';	break;
	    case 't':	t += '\t';	break;
	    case 'v':	t += '\v';	break;
	    case '\'':	t += '\'';	break;
	    case '"':	t += '"';	break;
	    case '\\':	t += '\\';	break;
	    case '?':	t += '?';	break;

	    default:
		t += '\\';
		t += s[i];
		break;
	    }
	}
	else {
	    t += s[i];
	}
    }

    return setString(t);
}

AnyScalar& AnyScalar::setAutoString(const std::string &input)
{
    // - int: input was readable by strtoll and is small enough.
    // - long: input was readable by strtoll
    {
	char *endptr;
#ifndef _MSC_VER
	long long l = strtoll(input.c_str(), &endptr, 10);
#else
	long long l = _strtoi64(input.c_str(), &endptr, 10);
#endif
	if (endptr != NULL && *endptr == 0)
	{
	    if (INT_MIN <= l && l <= INT_MAX)
	    {
		resetType(ATTRTYPE_INTEGER);
		val._int = l;
		return *this;
	    }
	    else
	    {
		resetType(ATTRTYPE_LONG);
		val._long = l;
		return *this;
	    }
	}
    }

    // - double: input was readble by strtod
    {
	char *endptr;
	double d = strtod(input.c_str(), &endptr);
	if (endptr != NULL && *endptr == 0)
	{
	    resetType(ATTRTYPE_DOUBLE);
	    val._double = d;
	    return *this;
	}
    }

    // - string: all above failed.
    resetType(ATTRTYPE_STRING);
    *val._string = input;

    return *this;
}

bool AnyScalar::getBoolean() const
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
	return (val._int != 0);

    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
	return (val._int != 0);

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
	return (val._uint != 0);

    case ATTRTYPE_LONG:
	return (val._long != 0);

    case ATTRTYPE_QWORD:
	return (val._ulong != 0);

    case ATTRTYPE_FLOAT:
	return (val._float != 0.0f);

    case ATTRTYPE_DOUBLE:
	return (val._double != 0.0f);

    case ATTRTYPE_STRING:
    {
	assert(val._string);

	if (*val._string == "0" || *val._string == "f" || *val._string == "false"
	    || *val._string == "n" || *val._string == "no") {
	    return false;
	}
	if (*val._string == "1" || *val._string == "t" || *val._string == "true"
	    || *val._string == "y" || *val._string == "yes") {
	    return true;
	}

	throw(ConversionException("Cannot convert string to boolean."));
    }
    }

    assert(0);
    return false;
}

int AnyScalar::getInteger() const
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
	return val._int;

    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
	return val._int;

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
	return val._uint;

    case ATTRTYPE_LONG:
	if (val._long > INT_MAX) return INT_MAX;
	if (val._long < INT_MIN) return INT_MIN;

	return static_cast<int>(val._long);

    case ATTRTYPE_QWORD:
	if (val._ulong > UINT_MAX) return UINT_MAX;
	return static_cast<int>(val._ulong);

    case ATTRTYPE_FLOAT:
	return static_cast<int>(val._float);

    case ATTRTYPE_DOUBLE:
	return static_cast<int>(val._double);

    case ATTRTYPE_STRING:
    {
	assert(val._string);
	char *endptr;
	long i = strtol(val._string->c_str(), &endptr, 10);
	if (endptr != NULL && *endptr == 0)
	    return i;

	throw(ConversionException("Cannot convert string to integer."));
    }
    }

    assert(0);
    return false;
}

unsigned int AnyScalar::getUnsignedInteger() const
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
	return static_cast<unsigned int>(val._int);

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
	return val._uint;

    case ATTRTYPE_LONG:
	return static_cast<unsigned int>(val._long);

    case ATTRTYPE_QWORD:
	return static_cast<unsigned int>(val._ulong);

    case ATTRTYPE_FLOAT:
	return static_cast<unsigned int>(val._float);

    case ATTRTYPE_DOUBLE:
	return static_cast<unsigned int>(val._double);

    case ATTRTYPE_STRING:
    {
	assert(val._string);
	char *endptr;
	unsigned long i = strtoul(val._string->c_str(), &endptr, 10);
	if (endptr != NULL && *endptr == 0)
	    return i;

	throw(ConversionException("Cannot convert string to unsigned integer."));
    }
    }

    assert(0);
    return false;
}

long long AnyScalar::getLong() const
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
	return static_cast<long long>(val._int);

    case ATTRTYPE_LONG:
	return val._long;

    case ATTRTYPE_QWORD:
	return val._ulong;

    case ATTRTYPE_FLOAT:
	return static_cast<long long>(val._float);

    case ATTRTYPE_DOUBLE:
	return static_cast<long long>(val._double);

    case ATTRTYPE_STRING:
    {
	assert(val._string);
	char *endptr;
#ifndef _MSC_VER
	long long l = strtoll(val._string->c_str(), &endptr, 10);
#else
	long long l = _strtoi64(val._string->c_str(), &endptr, 10);
#endif
	if (endptr != NULL && *endptr == 0)
	    return l;

	throw(ConversionException("Cannot convert string to long long."));
    }
    }

    assert(0);
    return false;
}

unsigned long long AnyScalar::getUnsignedLong() const
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
	return static_cast<unsigned long long>(val._int);

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
	return static_cast<unsigned long long>(val._uint);

    case ATTRTYPE_LONG:
	return static_cast<unsigned long long>(val._long);

    case ATTRTYPE_QWORD:
	return val._ulong;

    case ATTRTYPE_FLOAT:
	return static_cast<unsigned long long>(val._float);

    case ATTRTYPE_DOUBLE:
	return static_cast<unsigned long long>(val._double);

    case ATTRTYPE_STRING:
    {
	assert(val._string);
	char *endptr;
#ifndef _MSC_VER
	unsigned long long u = strtoull(val._string->c_str(), &endptr, 10);
#else
	unsigned long long u = _strtoui64(val._string->c_str(), &endptr, 10);
#endif
	if (endptr != NULL && *endptr == 0)
	    return u;

	throw(ConversionException("Cannot convert string to unsigned long long."));
    }
    }
    assert(0);
    return false;
}

double AnyScalar::getDouble() const
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
	return static_cast<double>(val._int);

    case ATTRTYPE_LONG:
	return static_cast<double>(val._long);

    case ATTRTYPE_QWORD:
	return static_cast<double>(val._ulong);

    case ATTRTYPE_FLOAT:
	return static_cast<double>(val._float);

    case ATTRTYPE_DOUBLE:
	return val._double;

    case ATTRTYPE_STRING:
    {
	assert(val._string);
	char *endptr;
	double d = strtod(val._string->c_str(), &endptr);
	if (endptr != NULL && *endptr == 0)
	    return d;

	throw(ConversionException("Cannot convert string to double."));
    }
    }
    assert(0);
    return false;
}

std::string AnyScalar::getString() const
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
	if (val._int == 0) return "false";
	if (val._int == 1) return "true";
	assert(0);
	return "invalid";

    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
    {
	return boost::lexical_cast<std::string>(val._int);
    }

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
    {
	return boost::lexical_cast<std::string>(val._uint);
    }

    case ATTRTYPE_LONG:
    {
	return boost::lexical_cast<std::string>(val._long);
    }

    case ATTRTYPE_QWORD:
    {
	return boost::lexical_cast<std::string>(val._ulong);
    }

    case ATTRTYPE_FLOAT:
    {
	return boost::lexical_cast<std::string>(val._float);
    }

    case ATTRTYPE_DOUBLE:
    {
	return boost::lexical_cast<std::string>(val._double);
    }

    case ATTRTYPE_STRING:
    {
	assert(val._string);
	return *val._string;
    } 
    }
    assert(0);
    return false;
}

std::string AnyScalar::getStringQuoted() const
{
    std::string str = getString();
    std::string os = "\"";

    os.reserve(2 + str.size() + str.size() / 16);

    // escape string s
    for(std::string::const_iterator si = str.begin(); si != str.end(); ++si)
    {
	switch(*si)
	{
	case '\a':
	    os += "\\a";
	    break;

	case '\b':
	    os += "\\b";
	    break;

	case '\f':
	    os += "\\f";
	    break;

	case '\n':
	    os += "\\n";
	    break;

	case '\r':
	    os += "\\r";
	    break;

	case '\t':
	    os += "\\t";
	    break;
    
	case '\v':
	    os += "\\v";
	    break;

	case '\\':
	    os += "\\\\";
	    break;

	case '"':
	    os += "\\\"";
	    break;

	case '\'':
	    os += "\\'";
	    break;

	default:
	    os += *si;
	    break;
	}
    }

    os += "\"";
    return os;
}

void AnyScalar::resetType(attrtype_t t)
{
    // if setting to a string and this is not a string, create the object
    if (t == ATTRTYPE_STRING) {
	if (atype != ATTRTYPE_STRING) {
	    val._string = new std::string;
	}
    }
    else {
	if (atype == ATTRTYPE_STRING) {
	    delete val._string;
	}

	val._ulong = 0;
    }

    atype = t;
}

bool AnyScalar::convertType(attrtype_t t)
{
    // same source and target type?
    if (atype == t) return true;

    // special case if this object was initialised with the ATTRTYPE_INVALID:
    // all the get...() below would assert.
    if (atype == ATTRTYPE_INVALID)
    {
	atype = t;
	if (atype == ATTRTYPE_STRING) {
	    val._string = new std::string;
	}
	return true;
    }
    
    switch(t)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	return false;

    case ATTRTYPE_BOOL:
    {
	bool v = getBoolean();
	if (atype == ATTRTYPE_STRING) delete val._string;
	val._int = v;
	atype = t;
	return true;
    }

    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
    {
	int v = getInteger();
	if (atype == ATTRTYPE_STRING) delete val._string;
	val._int = v;
	atype = t;
	return true;
    }

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
    {
	unsigned int v = getUnsignedInteger();
	if (atype == ATTRTYPE_STRING) delete val._string;
	val._uint = v;
	atype = t;
	return true;
    }

    case ATTRTYPE_LONG:
    {
	long long v = getLong();
	if (atype == ATTRTYPE_STRING) delete val._string;
	val._long = v;
	atype = t;
	return true;
    }

    case ATTRTYPE_QWORD:
    {
	unsigned long long v = getLong();
	if (atype == ATTRTYPE_STRING) delete val._string;
	val._ulong = v;
	atype = t;
	return true;
    }

    case ATTRTYPE_FLOAT:
    {
	float f = static_cast<float>(getDouble());
	if (atype == ATTRTYPE_STRING) delete val._string;
	val._float = f;
	atype = t;
	return true;
    }

    case ATTRTYPE_DOUBLE:
    {
	double d = getDouble();
	if (atype == ATTRTYPE_STRING) delete val._string;
	val._double = d;
	atype = t;
	return true;
    }

    case ATTRTYPE_STRING:
    {
	val._string = new std::string(getString());
	atype = t;
	return true;
    } 
    }
    assert(0);
    return false;
}

AnyScalar AnyScalar::operator-() const
{
    AnyScalar at = *this;
    
    switch(at.atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	throw(ConversionException("Negating invalid type."));
	
    case ATTRTYPE_BOOL:
	if (at.val._int > 0) at.val._int = 0;
	else at.val._int = 1;
	break;

    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
	at.val._int = - at.val._int;
	break;

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
	at.val._uint = - at.val._uint;
	break;

    case ATTRTYPE_LONG:
	at.val._long = - at.val._long;
	break;

    case ATTRTYPE_QWORD:
	at.val._ulong = - at.val._ulong;
	break;

    case ATTRTYPE_FLOAT:
	at.val._float = - at.val._float;
	break;

    case ATTRTYPE_DOUBLE:
	at.val._double = - at.val._double;
	break;

    case ATTRTYPE_STRING:
    {
	if (!at.convertType(ATTRTYPE_DOUBLE))
	    throw(ConversionException("Cannot convert string to double for negation."));

	at.val._double = - at.val._double;
	break;
    }
    }

    return at;
}

template <template <typename Type> class Operator, char OpName>
AnyScalar AnyScalar::binary_arith_op(const AnyScalar &b) const
{
    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	throw(ConversionException(std::string("Invalid type for first operand of binary operator ")+OpName+"."));

    case ATTRTYPE_BOOL:
	throw(ConversionException("No binary operators are allowed on bool values."));

    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpName+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpName+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<int> op;
	    if (OpName == '/' && b.val._int == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(val._int, b.val._int) );
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<int> op;
	    if (OpName == '/' && b.val._uint == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(val._int, static_cast<signed int>(b.val._uint)) );
	}

	case ATTRTYPE_LONG:
	{
	    Operator<long long> op;
	    if (OpName == '/' && b.val._long == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(val._int, b.val._long) );
	}
	    
	case ATTRTYPE_QWORD:
	{
	    Operator<long long> op;
	    if (OpName == '/' && b.val._ulong == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(static_cast<long long>(val._int), static_cast<long long>(b.val._ulong)) );
	}
	    
	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return AnyScalar( op(static_cast<float>(val._int), b.val._float));
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return AnyScalar( op(static_cast<double>(val._int), b.val._double) );
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_INTEGER))
		throw(ConversionException(std::string("Could not convert string to integer for binary operator ")+OpName+"."));

	    Operator<int> op;

	    int bval = bc.getInteger();
	    if (OpName == '/' && bval == 0) throw(ArithmeticException("Integer division by zero"));

	    return AnyScalar( op(val._int, bval) );
	}
	}
	break;
    }

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpName+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpName+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<int> op;
	    if (OpName == '/' && b.val._int == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(static_cast<signed int>(val._uint), b.val._int) );
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<unsigned int> op;
	    if (OpName == '/' && b.val._uint == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(val._uint, b.val._uint) );
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<long long> op;
	    if (OpName == '/' && b.val._long == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(static_cast<signed long long>(val._uint), b.val._long) );
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<unsigned long long> op;
	    if (OpName == '/' && b.val._ulong == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(static_cast<unsigned long long>(val._uint), b.val._ulong) );
	}
	    
	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return AnyScalar( op(static_cast<float>(val._uint), b.val._float));
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return AnyScalar( op(static_cast<double>(val._uint), b.val._double) );
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_DWORD))
		throw(ConversionException(std::string("Could not convert string to unsigned integer for binary operator ")+OpName+"."));

	    Operator<unsigned int> op;

	    int bval = bc.getInteger();
	    if (OpName == '/' && bval == 0) throw(ArithmeticException("Integer division by zero"));

	    return AnyScalar( op(val._int, bval) );
	}
	}
	break;
    }

    case ATTRTYPE_LONG:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpName+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpName+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<long long> op;
	    if (OpName == '/' && b.val._int == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(val._long, static_cast<long long>(b.val._int)) );
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<long long> op;
	    if (OpName == '/' && b.val._uint == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(val._long, static_cast<signed long long>(b.val._uint)) );
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<long long> op;
	    if (OpName == '/' && b.val._long == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(val._long, b.val._long) );
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<long long> op;
	    if (OpName == '/' && b.val._ulong == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(val._long, static_cast<signed long long>(b.val._ulong)) );
	}
	    
	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return AnyScalar( op(static_cast<float>(val._long), b.val._float));
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return AnyScalar( op(static_cast<double>(val._long), b.val._double) );
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_LONG))
		throw(ConversionException(std::string("Could not convert string to long for binary operator ")+OpName+"."));

	    Operator<long long> op;

	    long long bval = bc.getLong();
	    if (OpName == '/' && bval == 0) throw(ArithmeticException("Integer division by zero"));

	    return AnyScalar( op(val._long, bval) );
	}
	}
	break;
    }

    case ATTRTYPE_QWORD:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpName+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpName+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<long long> op;
	    if (OpName == '/' && b.val._int == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(static_cast<signed long long>(val._ulong), b.val._int) );
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<unsigned long long> op;
	    if (OpName == '/' && b.val._uint == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(val._ulong, static_cast<unsigned long long>(b.val._uint)) );
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<long long> op;
	    if (OpName == '/' && b.val._long == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(static_cast<signed long long>(val._ulong), b.val._long) );
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<unsigned long long> op;
	    if (OpName == '/' && b.val._ulong == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(val._ulong, b.val._ulong) );
	}
	    
	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return AnyScalar( op(static_cast<float>(val._ulong), b.val._float));
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return AnyScalar( op(static_cast<double>(val._ulong), b.val._double) );
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_QWORD))
		throw(ConversionException(std::string("Could not convert string to unsigned long long for binary operator ")+OpName+"."));

	    Operator<unsigned long long> op;

	    long long bval = bc.getLong();
	    if (OpName == '/' && bval == 0) throw(ArithmeticException("Integer division by zero"));

	    return AnyScalar( op(val._ulong, bval) );
	}
	}
	break;
    }

    case ATTRTYPE_FLOAT:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpName+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpName+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<float> op;
	    return AnyScalar( op(val._float, static_cast<float>(b.val._int)) );
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<float> op;
	    return AnyScalar( op(val._float, static_cast<float>(b.val._uint)) );
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<float> op;
	    return AnyScalar( op(val._float, static_cast<float>(b.val._long)) );
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<float> op;
	    return AnyScalar( op(val._float, static_cast<float>(b.val._ulong)) );
	}

	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return AnyScalar( op(val._float, b.val._float));
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return AnyScalar( op(static_cast<double>(val._float), b.val._double) );
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_FLOAT))
		throw(ConversionException(std::string("Could not convert string to float for binary operator ")+OpName+"."));

	    Operator<float> op;

	    return AnyScalar( op(val._float, bc.val._float) );
	}
	}

	break;
    }

    case ATTRTYPE_DOUBLE:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpName+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpName+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<double> op;
	    return AnyScalar( op(val._double, static_cast<double>(b.val._int)) );
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<double> op;
	    return AnyScalar( op(val._double, static_cast<double>(b.val._uint)) );
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<double> op;
	    return AnyScalar( op(val._double, static_cast<double>(b.val._long)) );
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<double> op;
	    return AnyScalar( op(val._double, static_cast<double>(b.val._ulong)) );
	}

	case ATTRTYPE_FLOAT:
	{
	    Operator<double> op;
	    return AnyScalar( op(val._double, static_cast<double>(b.val._float)));
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return AnyScalar( op(val._double, b.val._double) );
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_DOUBLE))
		throw(ConversionException(std::string("Could not convert string to double for binary operator ")+OpName+"."));

	    Operator<double> op;

	    return AnyScalar( op(val._double, bc.getDouble()) );
	}
	}

	break;
    }

    // if this is a string, then the other type defines the conversion
    case ATTRTYPE_STRING:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpName+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpName+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<int> op;
	    if (OpName == '/' && b.val._int == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(this->getInteger(), b.val._int) );
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<unsigned int> op;
	    if (OpName == '/' && b.val._uint == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(this->getUnsignedInteger(), b.val._uint) );
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<long long> op;
	    if (OpName == '/' && b.val._long == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(this->getLong(), b.val._long) );
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<unsigned long long> op;
	    if (OpName == '/' && b.val._ulong == 0) throw(ArithmeticException("Integer division by zero"));
	    return AnyScalar( op(this->getUnsignedLong(), b.val._ulong) );
	}

	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return AnyScalar( op(static_cast<float>(this->getDouble()), b.val._float) );
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return AnyScalar( op(this->getDouble(), b.val._double) );
	}

	case ATTRTYPE_STRING:
	    if (OpName == '+')
	    {
		return AnyScalar( *val._string + *b.val._string );
	    }

	    throw(ConversionException(std::string("Binary operator ")+OpName+" is not allowed between two string values."));
	}
	break;
    }
    }
    assert(0);
    throw(ConversionException("Invalid binary operator call. (PROGRAM ERROR)"));
}

// *** Force instantiation of binary_op for the four arithmetic operators

/// Forced instantiation of binary_arith_op for AnyScalar::operator+()
template AnyScalar AnyScalar::binary_arith_op<std::plus, '+'>(const AnyScalar &b) const;

/// Forced instantiation of binary_arith_op for AnyScalar::operator-()
template AnyScalar AnyScalar::binary_arith_op<std::minus, '-'>(const AnyScalar &b) const;

/// Forced instantiation of binary_arith_op for AnyScalar::operator*()
template AnyScalar AnyScalar::binary_arith_op<std::multiplies, '*'>(const AnyScalar &b) const;

/// Forced instantiation of binary_arith_op for AnyScalar::operator/()
template AnyScalar AnyScalar::binary_arith_op<std::divides, '/'>(const AnyScalar &b) const;

template <template <typename Type> class Operator, int OpNum>
bool AnyScalar::binary_comp_op(const AnyScalar &b) const
{
    static const char *OpNameArray[] = { "==", "!=", "<", ">", "<=", ">=" };

    switch(atype)
    {
    case ATTRTYPE_INVALID:
	assert(0);
	throw(ConversionException(std::string("Invalid type for first operand of binary operator ")+OpNameArray[OpNum]+"."));

    case ATTRTYPE_BOOL:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpNameArray[OpNum]+"."));

	case ATTRTYPE_BOOL:
	{
	    Operator<bool> op;
	    return op(val._int, b.val._int);
	}

	default:
	    throw(ConversionException(std::string("Binary operator ")+OpNameArray[OpNum]+" is not permitted on bool values."));
	}
	break;
    }

    case ATTRTYPE_CHAR:
    case ATTRTYPE_SHORT:
    case ATTRTYPE_INTEGER:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpNameArray[OpNum]+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpNameArray[OpNum]+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<int> op;
	    return op(val._int, b.val._int);
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<int> op;
	    return op(val._int, static_cast<signed int>(b.val._uint));
	}

	case ATTRTYPE_LONG:
	{
	    Operator<long long> op;
	    return op(val._int, b.val._long);
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<long long> op;
	    return op(val._int, static_cast<signed long long>(b.val._ulong));
	}

	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return op(static_cast<float>(val._int), b.val._float);
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return op(static_cast<double>(val._int), b.val._double);
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_INTEGER))
		throw(ConversionException(std::string("Could not convert string to integer for binary operator ")+OpNameArray[OpNum]+"."));

	    Operator<int> op;

	    return op(val._int, bc.getInteger());
	}
	}
	break;
    }

    case ATTRTYPE_BYTE:
    case ATTRTYPE_WORD:
    case ATTRTYPE_DWORD:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpNameArray[OpNum]+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpNameArray[OpNum]+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<int> op;
	    return op(static_cast<signed int>(val._uint), b.val._int);
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<unsigned int> op;
	    return op(val._uint, b.val._uint);
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<long long> op;
	    return op(static_cast<signed long long>(val._uint), b.val._long);
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<unsigned long long> op;
	    return op(static_cast<unsigned long long>(val._uint), b.val._ulong);
	}

	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return op(static_cast<float>(val._uint), b.val._float);
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return op(static_cast<double>(val._uint), b.val._double);
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_DWORD))
		throw(ConversionException(std::string("Could not convert string to unsigned integer for binary operator ")+OpNameArray[OpNum]+"."));

	    Operator<unsigned int> op;

	    return op(val._int, bc.getInteger());
	}
	}
	break;
    }

    case ATTRTYPE_LONG:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpNameArray[OpNum]+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpNameArray[OpNum]+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<long long> op;
	    return op(val._long, static_cast<long long>(b.val._int));
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<long long> op;
	    return op(val._long, static_cast<signed long long>(b.val._uint));
	}

	case ATTRTYPE_LONG:
	{
	    Operator<long long> op;
	    return op(val._long, b.val._long);
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<long long> op;
	    return op(val._long, static_cast<signed long long>(b.val._ulong));
	}

	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return op(static_cast<float>(val._long), b.val._float);
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return op(static_cast<double>(val._long), b.val._double);
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_LONG))
		throw(ConversionException(std::string("Could not convert string to long long for binary operator ")+OpNameArray[OpNum]+"."));

	    Operator<long long> op;

	    return op(val._long, bc.getLong());
	}
	}
	break;
    }

    case ATTRTYPE_QWORD:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpNameArray[OpNum]+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpNameArray[OpNum]+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<long long> op;
	    return op(val._ulong, static_cast<signed long long>(b.val._int));
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<unsigned long long> op;
	    return op(val._ulong, b.val._uint);
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<long long> op;
	    return op(static_cast<signed long long>(val._ulong), b.val._long);
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<unsigned long long> op;
	    return op(val._ulong, b.val._ulong);
	}

	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return op(static_cast<float>(val._ulong), b.val._float);
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return op(static_cast<double>(val._ulong), b.val._double);
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_QWORD))
		throw(ConversionException(std::string("Could not convert string to unsigned long long for binary operator ")+OpNameArray[OpNum]+"."));

	    Operator<unsigned long long> op;

	    return op(val._int, bc.getUnsignedLong());
	}
	}
	break;
    }

    case ATTRTYPE_FLOAT:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpNameArray[OpNum]+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpNameArray[OpNum]+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<float> op;
	    return op(val._float, static_cast<float>(b.val._int));
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<float> op;
	    return op(val._float, static_cast<float>(b.val._uint));
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<float> op;
	    return op(val._float, static_cast<float>(b.val._long));
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<float> op;
	    return op(val._float, static_cast<float>(b.val._ulong));
	}
	    
	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return op(val._float, b.val._float);
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return op(static_cast<double>(val._float), b.val._double);
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_FLOAT))
		throw(ConversionException(std::string("Could not convert string to float for binary operator ")+OpNameArray[OpNum]+"."));

	    Operator<float> op;

	    return op(val._float, bc.val._float);
	}
	}

	break;
    }

    case ATTRTYPE_DOUBLE:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpNameArray[OpNum]+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpNameArray[OpNum]+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<double> op;
	    return op(val._double, static_cast<double>(b.val._int));
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<double> op;
	    return op(val._double, static_cast<double>(b.val._uint));
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<double> op;
	    return op(val._double, static_cast<double>(b.val._long));
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<double> op;
	    return op(val._double, static_cast<double>(b.val._ulong));
	}
	    
	case ATTRTYPE_FLOAT:
	{
	    Operator<double> op;
	    return op(val._double, static_cast<double>(b.val._float));
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return op(val._double, b.val._double);
	}

	case ATTRTYPE_STRING:
	{
	    AnyScalar bc = b;
	    
	    if (!bc.convertType(ATTRTYPE_DOUBLE))
		throw(ConversionException(std::string("Could not convert string to double for binary operator ")+OpNameArray[OpNum]+"."));

	    Operator<double> op;

	    return op(val._double, bc.getDouble());
	}
	}

	break;
    }

    // if this is a string, then the other type defines the conversion
    case ATTRTYPE_STRING:
    {
	switch(b.atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    throw(ConversionException(std::string("Invalid type for second operand of binary operator ")+OpNameArray[OpNum]+"."));

	case ATTRTYPE_BOOL:
	    throw(ConversionException(std::string("Binary operator ")+OpNameArray[OpNum]+" is not permitted on bool values."));

	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	{
	    Operator<int> op;
	    return op(this->getInteger(), b.val._int);
	}

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	{
	    Operator<unsigned int> op;
	    return op(this->getUnsignedInteger(), b.val._int);
	}
	    
	case ATTRTYPE_LONG:
	{
	    Operator<long long> op;
	    return op(this->getLong(), b.val._long);
	}

	case ATTRTYPE_QWORD:
	{
	    Operator<unsigned long long> op;
	    return op(this->getUnsignedLong(), b.val._ulong);
	}

	case ATTRTYPE_FLOAT:
	{
	    Operator<float> op;
	    return op(static_cast<float>(this->getDouble()), b.val._float);
	}

	case ATTRTYPE_DOUBLE:
	{
	    Operator<double> op;
	    return op(this->getDouble(), b.val._double);
	}

	case ATTRTYPE_STRING:
	{
	    Operator<std::string> op;
	    return op(*val._string, *b.val._string);
	}
	}
	break;
    }
    }
    assert(0);
    throw(ConversionException("Invalid binary operator call. (PROGRAM ERROR)"));
}

/// Forced instantiation of binary_comp_op for AnyScalar::equal_to()
template bool AnyScalar::binary_comp_op<std::equal_to, 0>(const AnyScalar &b) const;

/// Forced instantiation of binary_comp_op for AnyScalar::not_equal_to()
template bool AnyScalar::binary_comp_op<std::not_equal_to, 1>(const AnyScalar &b) const;

/// Forced instantiation of binary_comp_op for AnyScalar::less()
template bool AnyScalar::binary_comp_op<std::less, 2>(const AnyScalar &b) const;

/// Forced instantiation of binary_comp_op for AnyScalar::greater()
template bool AnyScalar::binary_comp_op<std::greater, 3>(const AnyScalar &b) const;

/// Forced instantiation of binary_comp_op for AnyScalar::less_equal()
template bool AnyScalar::binary_comp_op<std::less_equal, 4>(const AnyScalar &b) const;

/// Forced instantiation of binary_comp_op for AnyScalar::greater_equal()
template bool AnyScalar::binary_comp_op<std::greater_equal, 5>(const AnyScalar &b) const;

} // namespace stx
