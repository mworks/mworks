.. _Expressions:

Expressions
===========

MWorks' expression language should feel familiar to anyone who has used programming or scripting languages before.  The following sections describe it in detail.


Literals
--------

The expression parser recognizes literal values for a variety of data types.

.. list-table::
   :header-rows: 1

   * - Data Type
     - Examples
     - Notes
   * - Boolean
     - ::

         true
         false
         YES
         NO

     - Not case-sensitive
   * - Integer
     - ::

         1
         +123
         -4567

     -
   * - Floating point
     - ::

         1.0
         1.
         .1
         -0.1
         +0.1
         1.2e10

     -
   * - String
     - ::

         ''
         "foo"
         'bar'
         "Hello, world!"

     - Supports some C/C++ `escape sequences <http://en.cppreference.com/w/cpp/language/escape>`_
   * - List
     - ::

         []
         [1.5]
         [1, 2.5, 'foo']
         [1:10]
         [1,2,3:6,7,8:10]

     -
   * - Dictionary
     - ::

         {}
         {'foo': 1.5}
         {'a': 1, true: 2.5, 3.5: 'foo'}

     -


Operators
---------

The expression parser supports a number of mathematical, logical, comparison, and other operators.

=====================  ============  ===================  ==========
Description            Operator      Alternative Form(s)  Precedence
=====================  ============  ===================  ==========
**Unit operators (postfix)**
--------------------------------------------------------------------
Microseconds           ``us``                             1
Milliseconds           ``ms``                             1
Seconds                ``s``                              1
**Element access operators**
--------------------------------------------------------------------
Subscript              ``[]``                             2
**Unary operators (prefix)**
--------------------------------------------------------------------
Positive               ``+``                              3
Negative               ``-``                              3
Logical "not"          ``!``         ``not``              3
**Type conversion operators (prefix)**
--------------------------------------------------------------------
Boolean                ``(bool)``                         4
Integer                ``(int)``     ``(integer)``        4
Floating point         ``(float)``   ``(double)``         4
String                 ``(string)``                       4
**Binary operators**
--------------------------------------------------------------------
Multiplication         ``*``                              5
Division               ``/``                              5
Modulus (remainder)    ``%``                              5
Addition               ``+``                              6
Subtraction            ``-``                              6
Equality               ``==``        ``=``                7
Inequality             ``!=``                             7
Less than              ``<``         ``#LT``              7
Less than or equal     ``<=``        ``=<`` ``#LE``       7
Greater than           ``>``         ``#GT``              7
Greater than or equal  ``>=``        ``=>`` ``#GE``       7
Logical "and"          ``&&``        ``and`` ``#AND``     8
Logical "or"           ``||``        ``or`` ``#OR``       9
=====================  ============  ===================  ==========

Note that the unit operators and the plain-English logical operators (``not``, ``and``, and ``or``) are not case-sensitive.  However, the type-conversion operators and all operators that begin with ``#`` (``#LT``, ``#AND``, etc.) *are* case-sensitive.


Units
^^^^^

Since MWorks does not provide true support for quantities with associated units, the unit operators are simply shorthand for multiplication operations.  In the case of time units (``us``, ``ms``, and ``s``), the operators multiply by a factor suitable for converting to microseconds (1, 1000, and 1000000, respectively).


Subscripting
^^^^^^^^^^^^

If the expression ``exp`` evaluates to a list, then ``exp[n]`` returns the element at index *n*.  If *n* is out of bounds, the expression evaluates to zero, and the parser issues an error message.  (Note that the first element of a list is at index 0, *not* 1.)

If the expression ``exp`` evaluates to a dictionary, then ``exp[k]`` returns the value associated with key *k* in the dictionary.  If the dictionary has no matching key, the expression evaluates to zero, and the parser issues an error message.


Precedence
^^^^^^^^^^

The above table lists the operators from highest precedence (1) to lowest precedence (9).  Operator precedence determines the order of operations in expressions, as higher-precedence operations are evaluated before lower-precedence ones.

The order of operations can be altered by wrapping sub-expressions in parentheses, which have higher precedence than all other operators.  For example, ``*`` has higher precedence than ``+``, so ``1+2*3`` evaluates to 7, but ``(1+2)*3`` evaluates to 9.


Variables
---------

MWorks variables can be incorporated into expressions by referencing each variable by its name.  For example, if your experiment defines two variables, ``x`` and ``y``, which hold the values 1 and 2, respectively, then the expression ``3*x+y`` will evaluate to 5.

Variable names are case-sensitive, so ``myvar``, ``MYVAR``, and ``MyVar`` all refer to different variables.


Functions
---------

The expression parser recognizes a number of pre-defined functions, many of which are found in other programming or scripting languages.  Note that function names are *not* case sensitive, so ``now()``, ``NOW()``, and ``Now()`` are all equivalent.


Mathematical
^^^^^^^^^^^^

``abs(x)``
   Absolute value of *x*

``ceil(x)``
   Nearest integer not less than *x*

``cos(x)``
   Cosine of *x* (measured in radians)

``exp(x)``
   Euler's number (*e*) raised to the power *x*

``floor(x)``
   Nearest integer not greater than *x*

``logn(x)``
   Natural logarithm of *x*

``max(x,y)``
   Larger of *x* and *y*

``min(x,y)``
   Smaller of *x* and *y*

``pi()``
   The constant 𝜋

``pow(x,y)``
   *x* raised to the power *y*

``round(x)``
   Nearest integer to *x*.  Number is rounded away from zero in halfway cases.

``sin(x)``
   Sine of *x* (measured in radians)

``sqrt(x)``
   Square root of *x*

``tan(x)``
   Tangent of *x* (measured in radians)


Random Number Generators
^^^^^^^^^^^^^^^^^^^^^^^^

``rand()``
   Random floating-point value uniformly distributed in the range *[0..1)*

``rand(min,max)``
   Random floating-point value uniformly distributed in the range *[min..max)*

``disc_rand(min,max)``
   Random integer value uniformly distributed in the set of integer numbers *{min, min+1, min+2, ..., max}*

``geom_rand(prob,max)``
   Discrete random number sampled in the interval *[0, max]* from a geometric distribution with constant Bernoulli probability *prob*

``exp_rand(beta,min,max)``
   Random number from exponential distribution with mean *beta+min*, optionally truncated at *max*. *min* and *max* are both optional and default to zero and positive infinity, respectively.


Time-Related
^^^^^^^^^^^^

``next_frame_time()``
   Anticipated output time (in microseconds) of the frame that the stimulus display is currently rendering

``now()``
   Current experiment time in microseconds

``refresh_rate()``
   Main display’s refresh rate in hertz

``timer_expired(timer)``
   1 if the specified timer has expired, 0 otherwise


Other
^^^^^

``filenames(glob_expr)``
   Given *glob_expr*, a string containing a `shell-style file name pattern <http://tomecat.com/jeffy/tttt/glob.html>`_, returns a list of strings containing the names of all matching files (or any empty list, if no files match)

``format(fmt,...)``
   `printf-style <http://www.boost.org/doc/libs/1_64_0/libs/format/doc/format.html#printf_directives>`_ string formatting.  *fmt* is the format string, and any subsequent arguments are items to be formatted.

``num_accepted('sel')``
   Takes the name of a selectable object (as a string) and returns the number of accepted selections that have been made on it

``osname()``
   Name of the current operating system as a string (e.g. "macos", "ios")

``selection('sel',n)``
   Takes the name of a selection variable (as a string) and an integer *n* and returns the *n*-th tentative (i.e. non-accepted) selection on the variable

``size(x)``
   Size of *x*: number of characters in a string, number of elements in a list or dictionary, -1 otherwise
