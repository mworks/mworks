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

     - Can include :ref:`range expressions <Range Expressions>`
   * - Dictionary
     - ::

         {}
         {'foo': 1.5}
         {'a': 1, true: 2.5, 3.5: 'foo'}

     -


Operators
---------

The expression parser supports a number of mathematical, logical, comparison, and other operators.

=====================  ============  ===================  ==============  ==========
Description            Operator      Alternative Form(s)  Example         Precedence
=====================  ============  ===================  ==============  ==========
**Unit operators (postfix)**
------------------------------------------------------------------------------------
Microseconds           ``us``                             ``2000us``      1
Milliseconds           ``ms``                             ``100ms``       1
Seconds                ``s``                              ``1.5s``        1
**Element access operators**
------------------------------------------------------------------------------------
Subscript              ``[]``                             ``{1:'a'}[1]``  2
**Unary operators (prefix)**
------------------------------------------------------------------------------------
Positive               ``+``                               ``+3``         3
Negative               ``-``                               ``-1.7``       3
Logical "not"          ``!``         ``not``               ``!true``      3
**Type conversion operators (prefix)**
------------------------------------------------------------------------------------
Boolean                ``(bool)``                          ``(bool)3``    4
Integer                ``(int)``     ``(integer)``         ``(int)2.7``   4
Floating point         ``(float)``   ``(double)``          ``(float)5``   4
String                 ``(string)``                        ``(string)4``  4
**Binary operators**
------------------------------------------------------------------------------------
Multiplication         ``*``                               ``1 * 2``      5
Division               ``/``                               ``1 / 2``      5
Modulus (remainder)    ``%``                               ``1 % 2``      5
Addition               ``+``                               ``1 + 2``      6
Subtraction            ``-``                               ``1 - 2``      6
Equality               ``==``        ``=``                 ``1 == 2``     7
Inequality             ``!=``                              ``1 != 2``     7
Less than              ``<``         ``#LT``               ``1 < 2``      7
Less than or equal     ``<=``        ``=<`` ``#LE``        ``1 <= 2``     7
Greater than           ``>``         ``#GT``               ``1 > 2``      7
Greater than or equal  ``>=``        ``=>`` ``#GE``        ``1 >= 2``     7
Logical "and"          ``&&``        ``and`` ``#AND``      ``1 && 2``     8
Logical "or"           ``||``        ``or`` ``#OR``        ``1 || 2``     9
=====================  ============  ===================  ==============  ==========

Note that the unit operators and the plain-English logical operators (``not``, ``and``, and ``or``) are not case-sensitive.  However, the type-conversion operators and all operators that begin with ``#`` (``#LT``, ``#AND``, etc.) *are* case-sensitive.

Also, be aware that the division operator (``/``) always produces a floating-point result, even if both of its operands are integers.  For example, ``1/2`` evaluates to 0.5, not 0 (as it does in some programming languages).


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


.. _Range Expressions:

Ranges
------

A range expression is a compact representation of an evenly-spaced, ordered sequence of integers.

The general form of a range expression is *start:stop:step*, where *start*, *stop*, and *step* are expressions that evaluate to integers.  *step* is optional.  If provided, it must evaluate to a positive integer; if omitted, it defaults to 1.  If *start* is less than *stop*, the sequence will be increasing; otherwise, it will be decreasing.  *start* is always included in the sequence.  If *start* and *stop* are separated by a multiple of *step*, *stop* will be included as well.

List literals, function calls, and `selection variable <Selection Variable>` value lists can all incorporate range expressions.  In all cases, range expressions can be mixed with other expressions, so long as all items are separated by commas.

The following table shows some examples of range expression usage, with equivalent, range-expression-free versions shown alongside.

.. list-table::
   :header-rows: 1

   * - Expression
     - Equivalent

   * - ``[1:5]``
     - ``[1,2,3,4,5]``

   * - ``f(2:-1)``
     - ``f(2,1,0,-1)``

   * - ``3:10:2``
     - ``3,5,7,9``

   * - ``[9:0:3]``
     - ``[9,6,3,0]``

   * - ``f(3:3)``
     - ``f(3)``

   * - ``7,2:4,-5,9:5:2,1``
     - ``7,2,3,4,-5,9,7,5,1``


Variables
---------

MWorks variables can be incorporated into expressions by referencing each variable by its name.  For example, if your experiment defines two variables, ``x`` and ``y``, which hold the values 1 and 2, respectively, then the expression ``3*x+y`` will evaluate to 5.

Variable names are case-sensitive, so ``myvar``, ``MYVAR``, and ``MyVar`` all refer to different variables.


Selection
^^^^^^^^^

When used in an expression, a `selection variable <Selection Variable>` evaluates to the currently-selected value.  If no selections have been made on the variable, the first one is made automatically, and its value is returned.


Timer
^^^^^

MWorks `timers <Start Timer>` are implemented as variables.  When used in an expression, the name of a timer evaluates to true if the timer has expired, false otherwise.

*Note*: ``timer_expired`` (see `Time-Related`_ functions below) actually just passes its input value unchanged.  However, using it may help to clarify the intent of an expression.


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

``display_bounds()``
  Returns a dictionary containing the bounds (in degrees) of the main display.  The keys in the dictionary are the strings "left", "right", "bottom", and "top".

``display_bounds(edge)``
  Given one of the strings "left", "right", "bottom", or "top", returns the corresponding boundary (in degrees) of the main display

``filenames(glob_expr)``
   Given *glob_expr*, a string containing a `shell-style file name pattern <http://tomecat.com/jeffy/tttt/glob.html>`_, returns a list of strings containing the names of all matching files (or any empty list, if no files match)

``format(fmt,...)``
   `printf-style <http://www.boost.org/doc/libs/1_66_0/libs/format/doc/format.html#printf_directives>`_ string formatting.  *fmt* is the format string, and any subsequent arguments are items to be formatted.

``num_accepted('sel')``
   Takes the name of a selectable object (as a string) and returns the number of accepted selections that have been made on it

``osname()``
   Name of the current operating system as a string (e.g. "macos", "ios")

``py_call(expr,...)``
   Evaluates *expr* as a Python expression, calls the resulting Python object with the remaining arguments, and returns the result.  If a Python error occurs, logs the error message and returns 0.

``py_eval(expr)``
   Evaluates *expr* as a Python expression and returns the result.  If a Python error occurs, logs the error message and returns 0.

``selection('sel',n)``
   Takes the name of a selection variable (as a string) and an integer *n* and returns the *n*-th tentative (i.e. non-accepted) selection on the variable

``size(x)``
   Size of *x*: number of characters in a string, number of elements in a list or dictionary, 0 otherwise

``type(x)``
   Data type of *x* as a string ("boolean", "integer", "float", "string", "list", or "dictionary")
