// $Id: AnyScalar.h 59 2007-07-17 14:43:23Z tb $

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

/** \file AnyScalar.h
 * Definition of a typed scalar value class AnyScalar used by the parser to
 * represent values.
 */

#ifndef _STX_AnyScalar_H_
#define _STX_AnyScalar_H_

#include <string>
#include <stdexcept>
#include <functional>
#include <ostream>
#include <assert.h>

namespace stx {

/** AnyScalar constructs objects holding a typed scalar value. It supports
 * boolean values, integer values both signed and unsigned, floating point
 * values and strings. The class provides operators which will compare scalars
 * between other scalars by converting them into a common domain. Furthermore
 * arithmetic operator will compose one or two scalars where the calculation is
 * done in the "higher" domain. */

class AnyScalar
{
public:
    /// Enumeration establishing identifiers for all supported types. All
    /// "small" integer types are included, because the class use originally
    /// designed for serializing large amounts of data.
    enum attrtype_t
    {
	/// Flag identifier for an uninitialized object.
	ATTRTYPE_INVALID = 0x00,

	/// A boolean type holding only true and false.
	ATTRTYPE_BOOL = 0x01,

	/// Character type, not viewed as a letter or digit, but as 8 bit
	/// signed integer.
	ATTRTYPE_CHAR = 0x10,

	/// Short signed integer type, 2 bytes long.
	ATTRTYPE_SHORT = 0x11,

	/// Medium signed integer type, 4 bytes long.
	ATTRTYPE_INTEGER = 0x12,

	/// Long (long) signed integer type, 8 bytes long.
	ATTRTYPE_LONG = 0x13,

        /// Byte type, not viewed as a letter or digit, but as 8 bit unsigned
	/// integer.
	ATTRTYPE_BYTE = 0x20,

	/// Short unsigned integer type, 2 bytes long.
	ATTRTYPE_WORD = 0x21,

	/// Medium unsigned integer type, 4 bytes long.
	ATTRTYPE_DWORD = 0x22,

	/// Long (long) unsigned integer type, 8 bytes long.
	ATTRTYPE_QWORD = 0x23,

	/// Single precision floating point type, 4 bytes long.
	ATTRTYPE_FLOAT = 0x30,

	/// Double precision floating point type, 8 bytes long.
	ATTRTYPE_DOUBLE = 0x31,

	/// String type, variable length. Needs much extra care to handle
	/// memory allocation.
	ATTRTYPE_STRING = 0x40
    };

private:
    /// The currently set type in the union.
    attrtype_t		atype;

    /// Union type to holding the current value of an AnyScalar.
    union value_t
    {
	/// Used for ATTRTYPE_BOOL, ATTRTYPE_CHAR, ATTRTYPE_SHORT, ATTRTYPE_INTEGER
	int			_int;

	/// Used for ATTRTYPE_BYTE, ATTRTYPE_WORD, ATTRTYPE_DWORD
	unsigned int		_uint;

	/// Used for ATTRTYPE_LONG
	long long		_long;

	/// Used for ATTRTYPE_QWORD
	unsigned long long	_ulong;

	/// Used for ATTRTYPE_FLOAT
	float			_float;

	/// Used for ATTRTYPE_DOUBLE
	double			_double;

	/// Used for ATTRTYPE_STRING, make sure it get delete'ed correctly.
	std::string*		_string;
    };

    /// Union holding the current value of set type.
    union value_t	val;
    
public:
    /// Create a new empty AnyScalar object of given type.
    explicit inline AnyScalar(attrtype_t t = ATTRTYPE_INVALID)
	: atype(t)
    { 
	if (atype == ATTRTYPE_STRING) {
	    val._string = new std::string;
	}
	else {
	    val._ulong = 0;
	}
    }

    // *** Constructors for the various types

    /// Construct a new AnyScalar object of type ATTRTYPE_BOOL and set the given
    /// boolean value.
    inline AnyScalar(bool b)
	: atype(ATTRTYPE_BOOL)
    {
	val._int = b;
    }

#ifndef SWIG	// This constructor confuses SWIG into calling it with
		// one-character strings
    /// Construct a new AnyScalar object of type ATTRTYPE_CHAR and set the given
    /// char value.
    inline AnyScalar(char c)
	: atype(ATTRTYPE_CHAR)
    {
	val._int = c;
    }
#endif
    /// Construct a new AnyScalar object of type ATTRTYPE_SHORT and set the given
    /// short value.
    inline AnyScalar(short s)
	: atype(ATTRTYPE_SHORT)
    {
	val._int = s;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_INTEGER and set the
    /// given integer value.
    inline AnyScalar(int i)
	: atype(ATTRTYPE_INTEGER)
    {
	val._int = i;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_INTEGER and set the
    /// given integer value.
    inline AnyScalar(long i)
	: atype(ATTRTYPE_INTEGER)
    {
	val._int = i;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_LONG and set the
    /// given long value.
    inline AnyScalar(long long l)
	: atype(ATTRTYPE_LONG)
    {
	val._long = l;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_BYTE and set the given
    /// unsigned value.
    inline AnyScalar(unsigned char c)
	: atype(ATTRTYPE_BYTE)
    {
	val._uint = c;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_WORD and set the given
    /// unsigned value.
    inline AnyScalar(unsigned short s)
	: atype(ATTRTYPE_WORD)
    {
	val._uint = s;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_DWORD and set the given
    /// unsigned value.
    inline AnyScalar(unsigned int i)
	: atype(ATTRTYPE_DWORD)
    {
	val._uint = i;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_DWORD and set the given
    /// unsigned value.
    inline AnyScalar(unsigned long i)
	: atype(ATTRTYPE_DWORD)
    {
	val._uint = i;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_QWORD and set the given
    /// unsigned value.
    inline AnyScalar(unsigned long long l)
	: atype(ATTRTYPE_QWORD)
    {
	val._ulong = l;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_FLOAT and set the given
    /// floating point value.
    inline AnyScalar(float f)
	: atype(ATTRTYPE_FLOAT)
    {
	val._float = f;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_DOUBLE and set the
    /// given floating point value.
    inline AnyScalar(double d)
	: atype(ATTRTYPE_DOUBLE)
    {
	val._double = d;
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_STRING and set the
    /// given string value.
    inline AnyScalar(const char *s)
	: atype(ATTRTYPE_STRING)
    {
	if (s == NULL) 
	    val._string = new std::string;
	else
	    val._string = new std::string(s);
    }
    /// Construct a new AnyScalar object of type ATTRTYPE_STRING and set the
    /// given string value.
    inline AnyScalar(const std::string &s)
	: atype(ATTRTYPE_STRING)
    {
	val._string = new std::string(s);
    }

    /// Destroy the object: free associated string memory if necessary.
    inline ~AnyScalar()
    {
	if (atype == ATTRTYPE_STRING) {
	    delete val._string;
	    val._string = NULL;
	}
    }
    
    /// Copy-constructor to deal with enclosed strings. Transfers type and
    /// value.
    inline AnyScalar(const AnyScalar &a)
	: atype(a.atype)
    {
	switch(atype)
	{
	case ATTRTYPE_INVALID:
	    break;

	case ATTRTYPE_BOOL:
	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	    val._int = a.val._int;
	    break;

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	    val._uint = a.val._uint;
	    break;

	case ATTRTYPE_LONG:
	    val._long = a.val._long;
	    break;

	case ATTRTYPE_QWORD:
	    val._ulong = a.val._ulong;
	    break;

	case ATTRTYPE_FLOAT:
	    val._float = a.val._float;
	    break;

	case ATTRTYPE_DOUBLE:
	    val._double = a.val._double;
	    break;

	case ATTRTYPE_STRING:
	    val._string = new std::string(*a.val._string);
	    break;
	}
    }

    /// Assignment operator to deal with enclosed strings. Transfers type and
    /// value.
    inline AnyScalar& operator=(const AnyScalar &a)
    {
	// check if we are to assign ourself
	if (this == &a) return *this;

	if (atype == ATTRTYPE_STRING) {
	    delete val._string;
	    val._string = NULL;
	}

	atype = a.atype;
	
	switch(atype)
	{
	case ATTRTYPE_INVALID:
	    assert(0);
	    break;

	case ATTRTYPE_BOOL:
	case ATTRTYPE_CHAR:
	case ATTRTYPE_SHORT:
	case ATTRTYPE_INTEGER:
	    val._int = a.val._int;
	    break;

	case ATTRTYPE_BYTE:
	case ATTRTYPE_WORD:
	case ATTRTYPE_DWORD:
	    val._uint = a.val._uint;
	    break;

	case ATTRTYPE_LONG:
	    val._long = a.val._long;
	    break;

	case ATTRTYPE_QWORD:
	    val._ulong = a.val._ulong;
	    break;

	case ATTRTYPE_FLOAT:
	    val._float = a.val._float;
	    break;

	case ATTRTYPE_DOUBLE:
	    val._double = a.val._double;
	    break;

	case ATTRTYPE_STRING:
	    val._string = new std::string(*a.val._string);
	    break;
	}

	return *this;
    }

    /// Comparison operator. Directly compares type _and_ value. Does NOT
    /// attempt to convert the values into a common domain.
    bool operator==(const AnyScalar &a) const;

    /// Comparison operator: Directly compares type _and_ value. Does NOT
    /// attempt to convert the values into a common domain.
    inline bool operator!=(const AnyScalar &a) const
    { return !(*this == a); }

    /// Return the type identifier of the object.
    inline attrtype_t	getType() const
    {
	return atype;
    }

    /// Returns true if this object contains a boolean value.
    inline bool		isBooleanType() const
    {
	return (atype == ATTRTYPE_BOOL);
    }

    /// Returns true if this object contains one of the integer types.
    inline bool		isIntegerType() const
    {
	return (atype == ATTRTYPE_BOOL ||
		atype == ATTRTYPE_CHAR || atype == ATTRTYPE_SHORT ||
		atype == ATTRTYPE_INTEGER || atype == ATTRTYPE_LONG ||
		atype == ATTRTYPE_BYTE || atype == ATTRTYPE_WORD ||
		atype == ATTRTYPE_DWORD || atype == ATTRTYPE_QWORD);
    }

    /// Returns true if this object contains one of the floating point types.
    inline bool		isFloatingType() const
    {
	return (atype == ATTRTYPE_FLOAT || atype == ATTRTYPE_DOUBLE);
    }

    /// Attempts to convert the current type/value into the given type. Returns
    /// false if the value could not be represented in the new type, the new
    /// type is set nonetheless. See the getXXX below on how the new value is
    /// computed from the old type.
    bool	convertType(attrtype_t t);

    /// Changes the current type and resets the contents without attempting to
    /// convert the enclosed value.
    void	resetType(attrtype_t t);

    // *** attrtype_t to string and string to attrtype_t functions

    /// Returns true if the attrtype_t is a valid type identifier.
    static bool isValidAttrtype(attrtype_t at);
    
    /// Returns the attrtype identifier of a string, throws ConversionException
    /// if s does not specify a valid type name.
    static attrtype_t stringToType(const std::string &s)
    {
	return stringToType(s.c_str());
    }

    /// Returns the attrtype identifier a string, throws ConversionException if
    /// it does not specify a valid type name.
    static attrtype_t stringToType(const char *s);

    /// Returns a const char* for each attrtype_t.
    static std::string getTypeString(attrtype_t at);

    /// Returns a string for this AnyScalar's type.
    inline std::string getTypeString() const
    {
	return getTypeString(atype);
    }

    // *** Type and Value Length Functions
    
    /// Return the storage length of the type in bytes. Beware that
    /// getTypeLength(bool) == 0.
    inline int	getTypeLength() const
    {
	return getTypeLength(atype);
    }

    /// Return the storage length of the type in bytes. Beware that
    /// getTypeLength(bool) == 0.
    static int	getTypeLength(attrtype_t t);

    /// Boolean check if this type is of fixed length.
    static bool isFixedLength(attrtype_t t)
    {
	return getTypeLength(t) >= 0;
    }

    /// Boolean check if this type is of fixed length.
    inline bool	isFixedLength() const
    {
	return isFixedLength(atype);
    }

    /// Return the storage length of this value (mark the different to
    /// getTypeLength())
    unsigned int getValueLength() const;

    // *** Setters

    // Sets the value, converting the input to the currently set type if
    // necessary. Returns false if the input could not be converted into the
    // right type.

    /** Change the value of the current object to i. Converting the integer
     * to this object's type if necessary.
     *
     * - boolean: the integer i is cut to 0 or 1.
     * - char: cut to -128..127 returns false if out of range.
     * - short: cut to -32768..32767 return false if out of range.
     * - byte: cut to 0..255 returns false if out of range.
     * - word: cut to 0..65536 returns false if out of range.
     * - int: no conversion.
     * - dword: cut to 0..4294967295 return false if it was negative.
     * - long: no cutting necessary.
     * - qword: cut to positive number.
     * - float: integer stored as floating point
     * - double: integer stored as floating point
     * - string: integer is stringified.
     */ 
    bool		setInteger(int i);

    /** Change the value of the current object to l. Converting the long
     * long integer to this object's type if necessary. If the long long does
     * not fully fit into the current type, then this function returns false.
     *
     * - boolean: the long i is cut to 0 or 1.
     * - char: cut to -128..127 returns false if out of range.
     * - short: cut to -32768..32767 return false if out of range.
     * - int: cut to -2147483646..2147483647 returns false if out of range.
     * - byte: cut to 0..255 returns false if out of range.
     * - word: cut to 0..65536 returns false if out of range.
     * - dword: cut to 0..4294967295 return false if out of range.
     * - long: no conversion
     * - qword: cut to positive number.
     * - float: long stored as floating point
     * - double: long stored as floating point
     * - string: long is stringified.
     */
    bool		setLong(long long l);

    /** Change the value of the current object to d. Converting the floating
     * point to this object's type if necessary.
     *
     * - boolean: [0:0.5] is false, (0.5:1] is true. else true and return false.
     * - char: truncated to integer ranged -128..127 returns false if out of range.
     * - short: truncated to integer ranged -32768..32767 return false if out of range.
     * - int: truncated to integer ranged -2147483646..2147483647 returns false if out of range.
     * - byte: truncated to integer ranged 0..255 returns false if out of range.
     * - word: truncated to integer ranged 0..65536 returns false if out of range.
     * - dword: truncated to integer ranged 0..4294967295 return false if out of range.
     * - long: truncated to integer ranged -9223372036854775806..9223372036854775807
     * - qword: truncated to integer ranged 0..18446744073709551615
     * - float: truncated to single-precision
     * - double: no conversion
     * - string: double is stringified.
     */
    bool		setDouble(double d);

    /** Change the value of the current object to s. Converting the string
     * to this object's type if necessary. If this string cannot be converted
     * to say an integer, then this function returns false.
     *
     * - boolean: "0", "f", "false", "n" and "no" are false,
     *            "1", "t", "true", "y" and "yes" are true. else returns false.
     * - char: string read using strtol and truncated to -128..127
               returns false if out of range or unparseable.
     * - short: string read using strtol and truncated to -32768..32767
                returns false if out of range or unparseable.
     * - int: string read using strtol returns false if out of range or unparseable.
     * - long: string read using strtoll returns false if out of range or unparseable.
     * - byte: string read using strtoul and truncated to 0..255
               returns false if out of range or unparseable.
     * - word: string read using strtoul and truncated to 0..65535
               returns false if out of range or unparseable.
     * - dword: string read using strtoul returns false if out of range or unparseable.
     * - qword: string read using strtoull returns false if out of range or unparseable.
     * - float: string read using strtod and converted to single-precision
                returns false if unparseable.
     * - double: string read using strtod returns false if unparseable.
     * - string: no conversion
     */
    bool		setString(const std::string &s);
    
    /// Change the value of the current object to s. This string s must be
    /// quoted: "str". This function resolves escape sequences like \n.
    bool		setStringQuoted(const std::string &s);


    /** Change the type _and_ value of the current object to s. The current
     * type is set if the given string can be converted to an integer or a
     * floating point number. This was required because e.g. the usual input
     * from files or perl are all untyped strings. The following input
     * conversions are tested and yield the given types:
     *
     * - boolean: no boolean strings are matched!
     * - int: input was readable by strtoll and is small enough.
     * - long: input was readable by strtoll
     * - double: input was readble by strtod
     * - string: all above failed.
     *
     * @return reference to this for chaining.
     */
    AnyScalar&		setAutoString(const std::string &input);

    // *** Getters

    // Return the enclosed value in different types, converting if
    // necessary. If the enclosed value cannot be converted these functions
    // throw a ConversionError exception.

    /** Return the value converted to a boolean. If the enclosed value could
     * not be converted this function throws a ConversionException.
     *
     * - boolean: no conversion
     * - char/short/int/long: test integer != 0
     * - byte/word/dword/qword: test integer != 0
     * - float/double: test float != 0.0
     * - string: "0", "f", "false", "n", "no" are false,
     *           "1", "t", "true", "y", "yes" are true
     *           other strings throw ConversionException.
     */
    bool		getBoolean() const;

    /** Return the value converted to an integer. If the enclosed value could
     * not be converted this function throws a ConversionException.
     *
     * - boolean: returns 0 or 1.
     * - char/short/int: return the integer
     * - byte/word/dword: return the integer (possibly as a negative number)
     * - long: cut to -2147483646..2147483647.
     * - qword: cut to 0..4294967295
     * - float/double: machine-conversion floating point to integer
     * - string: Attempt to strtol the string, throws ConversionException if this fails.
     */
    int			getInteger() const;

    /** Return the value converted to an unsigned integer. If the enclosed
     * value could not be converted this function throws a ConversionException.
     *
     * - boolean: returns 0 or 1.
     * - char/short/int: return the integer (possibly representing a negative number as positive)
     * - byte/word/dword: return the unsigned integer
     * - long: cut to 0..4294967295 (also misrepresenting because of machine-conversion)
     * - qword: cut to 0..4294967295
     * - float/double: machine-conversion floating point to unsigned integer.
     * - string: Attempt to strtoul the string, throws ConversionException if this fails.
     */
    unsigned int 	getUnsignedInteger() const;

    /// Alias for getInteger()
    inline int		getInt() const
    {
	return getInteger();
    }

    /// Alias for getUnsignedInteger()
    inline unsigned int	getUInt() const
    {
	return getUnsignedInteger();
    }

    /** Return the value converted to a long integer. If the enclosed value
     * could not be converted this function throws a ConversionException.
     *
     * - boolean: returns 0 or 1.
     * - char/short/int: return the integer (possibly representing a negative number as positive)
     * - byte/word/dword: return the unsigned integer
     * - long: no conversion
     * - qword: return the integer (possibly converting a large long to a negative)
     * - float/double: machine-conversion floating point to long long.
     * - string: Attempt to strtoll the string, throws ConversionException if this fails.
     */
    long long 		getLong() const;

    /** Return the value converted to an unsigned long integer. If the enclosed
     * value could not be converted this function throws a ConversionException.
     *
     * - boolean: returns 0 or 1.
     * - char/short/int: return the integer (possibly representing a negative number as positive)
     * - byte/word/dword: return the unsigned integer
     * - long: return the long (possibly interpreting a negative long as a large positive)
     * - qword: no conversion
     * - float/double: machine-conversion floating point to unsigned long long.
     * - string: Attempt to strtoull the string, throws ConversionException if this fails.
     */
    unsigned long long 	getUnsignedLong() const;

    /// Alias for getUnsignedLong()
    unsigned long long	getULong() const
    {
	return getUnsignedLong();
    }

    /** Return the value converted to a double. If the enclosed value could not
     * be converted this function throws a ConversionException.
     *
     * - boolean: returns 0 or 1.0.
     * - char/short/int/long: return the integer as a float
     * - byte/word/dword/qword: return the unsigned integer as a float
     * - float: cast to double.
     * - double: no conversion
     * - string: Attempt to strtod the string, throws ConversionException if this fails.
     */
    double		getDouble() const;

    /** Return the value converted to a string. This function never throws
     * ConversionException.
     *
     * - boolean: returns "false" or "true"
     * - char/short/int/long: return the integer as a string using boost::lexical_cast
     * - byte/word/dword/qword: return the unsigned integer as a string using boost::lexical_cast
     * - float/double: return float as a string using boost::lexical_cast
     * - string: return the string.
     */
    std::string		getString() const;

    /** Return the value converted to a quoted string. This function never
     * throws ConversionException. It calls getString() and adds " and escape
     * sequences. */
    std::string		getStringQuoted() const;
    
    // *** Unary Operators
    
    /** Unary prefix - operator. Attempts to convert the AnyScalar to a numeric
     * value.
     *
     * - bool: invert the boolean value
     * - char/short/int/long: switch sign
     * - byte/word/dword/qword: negated the number even tho it doesn't make sense.
     * - float/double: switch sign
     * - string: attempt to convert the string to a double and negate it!
     *           May throw a ConversionException
     */
    AnyScalar 		operator-() const;

    // *** Binary Operators

    // These will convert the two operands to the largest common type of the
    // same field.

#ifndef SWIG	// obviously too strange for SWIG
private:
    /** Binary arithmetic template operator. Converts the two AnyScalars into the
     * largest type of their common field. If a string cannot be converted to a
     * numeric of the same field as the other operand a ConversionException is
     * thrown.
     
     * Conversion chart: * signifies the operator
     
     - bool * (any): ConversionException. Don't consider boolean as the field F_2.
     - char|short|int * char|short|int: return AnyScalar is an integer.
     - byte|word|dword * char|short|int: returned AnyScalar is a signed integer.
     - byte|word|dword * byte|word|dword: returned AnyScalar is an unsigned integer.
     - long * char|short|int|long|byte|word|dword|qword: returned AnyScalar is a signed long (long).
     - qword * char|short|int|long: : returned AnyScalar is a signed long (long).
     - qword * byte|word|dword|qword: : returned AnyScalar is an unsigned long (long).
     - float|double * char|short|int|long|byte|word|dword|qword: calculate as floating point number
     - string * char|short|int: convert string to integer, calculate as signed integer.
     - string * byte|word|dword: convert string to unsigned integer, calculate as unsigned integer.
     - string * long: convert string to long integer, calculate as long.
     - string * qword: convert string to unsigned long integer, calculate as unsigned long.
     - string * float|double: convert string to floating point, calculate as floating point.
     - string * string: only valid for "+" operator as string concatenation.
    */
    template <template <typename Type> class Operator, char OpName>
    AnyScalar		binary_arith_op(const AnyScalar &b) const;
#endif
    
public:
    /// Instantiation of binary_arith_op for "+" plus.
    inline AnyScalar	operator+(const AnyScalar &b) const
    {
	return binary_arith_op<std::plus, '+'>(b);
    }

    /// Instantiation of binary_arith_op for "-" minus.
    inline AnyScalar	operator-(const AnyScalar &b) const
    {
	return binary_arith_op<std::minus, '-'>(b);
    }

    /// Instantiation of binary_arith_op for "*" multiplication.
    inline AnyScalar	operator*(const AnyScalar &b) const
    {
	return binary_arith_op<std::multiplies, '*'>(b);
    }

    /// Instantiation of binary_arith_op for "/" division.
    inline AnyScalar	operator/(const AnyScalar &b) const
    {
	return binary_arith_op<std::divides, '/'>(b);
    }

    /// Alias for operator+ in script languages
    inline AnyScalar	add(const AnyScalar &b) const
    {
	return (*this + b);
    }

    /// Alias for operator- in script languages
    inline AnyScalar	subtract(const AnyScalar &b) const
    {
	return (*this - b);
    }

    /// Alias for operator* in script languages
    inline AnyScalar	multiply(const AnyScalar &b) const
    {
	return (*this * b);
    }

    /// Alias for operator/ in script languages
    inline AnyScalar	divide(const AnyScalar &b) const
    {
	return (*this / b);
    }

#ifndef SWIG	// obviously too strange for SWIG
private:
    /** Binary comparison template operator. Converts the two AnyScalars into the
     * largest type of their common field. If a string cannot be converted to a
     * numeric of the same field as the other operand a ConversionException is
     * thrown. Operator is the STL operator class. OpNum is an identifier for
     * the operator name string within the template.

     * Conversion chart: * signifies the operator
     
     - bool * bool: usual comparison
     - bool * (any): ConversionException. Don't consider boolean as the field F_2.

     - char|short|int * char|short|int: compare as signed integers.
     - byte|word|dword * char|short|int: compare as signed integers.
     - byte|word|dword * byte|word|dword: compare as unsigned integers.
     - long * char|short|int|long|byte|word|dword|qword: compare as signed long integer
     - qword * char|short|int|long: : compare as signed long integer
     - qword * byte|word|dword|qword: : compare an unsigned long (long).
     - float|double * char|short|int|long|byte|word|dword|qword: compare as floating point numbers
     - string * char|short|int: convert string to integer, compare as signed integer.
     - string * byte|word|dword: convert string to unsigned integer, compare as unsigned integer.
     - string * long: convert string to long integer, compare as long.
     - string * qword: convert string to unsigned long integer, compare as unsigned long.
     - string * float|double: convert string to floating point, compare as floating point.
     - string * string: usual string comparison operators.
    */
    template <template <typename Type> class Operator, int OpNum>
    bool		binary_comp_op(const AnyScalar &b) const;
#endif

    // *** Don't use the operators themselves, because operator== is defined
    // *** differently above.
public:
    /// Instantiation of binary_comp_op for "==" equality.
    inline bool		equal_to(const AnyScalar &b) const
    {
	return binary_comp_op<std::equal_to, 0>(b);
    }

    /// Instantiation of binary_comp_op for "!=" inequality.
    inline bool		not_equal_to(const AnyScalar &b) const
    {
	return binary_comp_op<std::not_equal_to, 1>(b);
    }

    /// Instantiation of binary_comp_op for "<" less-than.
    inline bool		less(const AnyScalar &b) const
    {
	return binary_comp_op<std::less, 2>(b);
    }

    /// Instantiation of binary_comp_op for ">" greater-than.
    inline bool		greater(const AnyScalar &b) const
    {
	return binary_comp_op<std::greater, 3>(b);
    }

    /// Instantiation of binary_comp_op for "<=" less-or-equal-than.
    inline bool		less_equal(const AnyScalar &b) const
    {
	return binary_comp_op<std::less_equal, 4>(b);
    }

    /// Instantiation of binary_comp_op for ">=" greater-or-equal-than.
    inline bool		greater_equal(const AnyScalar &b) const
    {
	return binary_comp_op<std::greater_equal, 5>(b);
    }
};

/// Make AnyScalar outputtable to ostream
static inline std::ostream& operator<< (std::ostream &stream, const AnyScalar &as)
{
    return stream << as.getString();
}

} // namespace stx

#endif // VGS_AnyScalar_H
