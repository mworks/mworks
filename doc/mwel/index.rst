.. _MWEL:

Experiment Language (MWEL)
==========================

MWorks Experiment Language (MWEL) provides a user-friendly, programming-language-like alternative to the `XML <https://en.wikipedia.org/wiki/XML>`_\-based experiments created by MWEditor.  It is designed to be read and written via a text editor and is optimized for conciseness and clarity, with minimal syntactic "noise".

At runtime, MWEL experiments are translated into MWorks' XML, after which they are parsed and executed in exactly the same way as traditional, XML-based experiments.  This ensures that equivalent experiments written in MWEL and XML perform identically.  (However, it also means that some of the limitations of XML-based experiments apply to MWEL-based ones, too.)

To be loadable by MWorks, MWEL experiment files must be named with the extension ``.mwel``.

The following sections describe MWEL in detail.


Comments
--------

Comments are used to explain, clarify, or otherwise document parts of an experiment.  MWEL supports both single-line and multi-line comments.

Single-line comments begin with ``//`` and continue to the end of the line::

    // This is a single-line comment

    var x = 1  // Comments can appear on the same line as non-comment code

Multi-line comments begin with ``/*`` and end with ``*/``::

    /*
       Here is a comment
       that spans
       multiple lines
    */

    var y = /* This type of comment can appear within non-comment code */ 2

    /*
       Unlike some programming languages,
       /* MWEL allows nested pairs of
          /* multi-line comment delimiters within
             a multi-line comment */
       */
    */

    var z = 3


.. _identifiers:

Identifiers
-----------

Identifiers are used to name `variables <Variables>`, other `components <Components>`, and `macros`_.  They consist of a letter followed by any number of letters, digits, and underscores.  Here are some examples of valid identifiers::

    x
    abc123
    A_Long_Name_With_Many_Words


Expressions
-----------

Expressions in MWEL are identical to those used in XML-based experiments.  For details, see the `expression reference <Expressions>`.


.. _assignments:

Assignments
-----------

An assignment changes the value of a `variable <Variable>`.  Assignments come in three different varieties.

A simple assignment just replaces the old value with a new one::

    a = 'foo'
    b = [1,2,3]
    c = b + [4]  // c == [1, 2, 3, 4]

If a variable's current value is a list or dictionary, an index assignment assigns a new value to one of its elements::

    b[2] = {'a': 1.5}       // b == [1, 2, {"a": 1.5}]
    b[2]['b'] = [4,5,6]     // b == [1, 2, {"a": 1.5, "b": [4, 5, 6]}]
    b[2]['b'][3] = 'seven'  // b == [1, 2, {"a": 1.5, "b": [4, 5, 6, "seven"]}]

Finally, an augmented assignment performs a binary operation with the current value and another value and assigns the result to the variable::

    d = 7
    d += 8  // d == 15
    d /= 2  // d == 7.5

Index assignments may also be augmented::

    c[3] *= -2  // c == [1, 2, 3, -8]

All binary arithmetic operators (``+``, ``-``, ``*``, ``/``, ``%``) have corresponding augmented assignment operators (``+=``, ``-=``, ``*=``, ``/=``, ``%=``).


.. _component declarations:

Component Declarations
----------------------

`Components` are the building blocks of MWorks experiments.  Consequently, an experiment written in MWEL consists primarily of component declarations.

The general form of a component declaration is as follows::

    signature tag (
        parameter1 = ...
        parameter2 = ...
    ) {
        // Child components
        ...
    }

*signature* is the type signature of the component.  For example, the signature of a `Video Stimulus` component is ``stimulus/video``.

*tag* is a unique name to associate with the component.  It can be either an `identifier <identifiers>` (e.g. ``my_video``) or a string literal (e.g. ``'My Video'``).  The tag is used to refer to the component elsewhere in the experiment (for example, as a parameter of a `Queue Stimulus` action).  If the component will not be referenced elsewhere, the tag may be omitted.


.. _component declaration parameters:

Parameters
^^^^^^^^^^

The signature and tag are followed by a list of parameter names and values, enclosed in parentheses.  For example::

    stimulus/video 'My Video' (
        path = 'my_video.mp4'
        x_size = 15.0
        y_size = 15.0
        autoplay = true
    )

To specify multiple parameters on the same line, separate each name/value pair with a semicolon::

    action/start_timer (timer = MyTimer; duration = 100ms)

If a component does not require parameters, the parameter list can be empty::

    stimulus/white_noise_background bg ()


Children
^^^^^^^^

Most components are arranged in a parent/child hierarchy.  For example, a `paradigm component <Paradigm Components>` can have `actions <Actions>` and other paradigm components as children, and those children can have their own child components.

In MWEL, a component's children follow its parameters, enclosed in braces (aka curly brackets)::

    protocol 'Protocol 1' {
        trial (nsamples = 1000) {
            task_system {
                task_system_state 'Acquire Fixation' {
                    action/queue_stimulus (stimulus = fixation_point)
                    action/update_stimulus_display ()
                    action/start_timer (timer = MyTimer; duration = 500ms)
                    ...
                }

                // Other states
                ...
            }
        }
    }

As shown above, if a component requires no parameters but does have children, its parameter list may be omitted.  However, a component must always have *either* a parameter list or a child list (which can be empty).  You cannot omit both::

    protocol A ()  // OK
    protocol B {}  // OK
    protocol C     // Not OK! (syntax error)


Type and Parameter Name Inference
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The general form of a component type signature is ``kind/type``, e.g. ``stimulus/movie`` or ``action/load_stimulus``.  However, if ``type`` is unique among all components, you may omit ``kind/`` from declarations of the given component type, as the MWEL processor can infer it at runtime.

Furthermore, if a component has only one parameter (or only one *required* parameter), you may omit ``name =`` from the parameter list and provide just the value of that parameter.

MWEL's inference of type and parameter names can make your experiments both shorter (with fewer characters to type) *and* easier to read and understand.  This is especially true when declaring `actions <Actions>`. For example, compare the following experiment fragment::

    action/if (condition = images_queued) {
        image_index = 0
        action/while (condition = image_index < num_images) {
            action/dequeue_stimulus (stimulus = images[image_index])
            action/report (message = 'Dequeued image $image_index')
            image_index += 1
        }
        action/update_stimulus_display ()
    }

with an equivalent version that makes use of type and parameter name inference::

    if (images_queued) {
        image_index = 0
        while (image_index < num_images) {
            dequeue_stimulus (images[image_index])
            report ('Dequeued image $image_index')
            image_index += 1
        }
        update_stimulus_display ()
    }


.. _variable declarations:

Variable Declarations
---------------------

Like other components, `variables <Variable>` are declared using :ref:`component declaration <component declarations>` syntax::

    var a (default_value = 1.5)

Because most experiments declare many variables, MWEL provides a simplified syntax for specifying a variable's default value::

    var a = 1.5  // Equivalent to above

The default value can be any expression::

    var b = 'Hello, world!'
    var c = 2*a + 3

Variables declared in this way can still include parameters and/or child components::

    var x = 3 (persistant = YES) {
        report ('x = $x')
    }

Variables can be declared only at the top level of an experiment or inside a top-level `folder <Folder>`.  They cannot be declared inside a `protocol <Protocol>` or other component.


.. _includes:

Includes
--------

To promote code reuse, or to simplify the management of a complex experiment, you may want to divide your experimental code in to multiple files.  MWEL supports this via the ``%include`` directive::

    %include my_vars
    %include 'stims/setA.mwel'
    %include '/my_lab/shared/setup1_io.inc'

When the MWEL parser encounters an include statement, it loads and parses the specified file and inserts the result in to the including file's parse tree at the location of the statement.  The provided file path may be absolute or relative to the including file.  If the path does not include a file extension, ``.mwel`` is assumed.

Included files can themselves include other files.  A file may even be included multiple times in multiple files; the parser will process only the first instance of the include that it encounters and ignore all others.

Files can be included only at the top level of a source file.  They cannot be included within a `protocol <Protocol>` or other component.


.. _macros:

Macros
------

Macros are another mechanism for code reuse in MWEL.  By defining a macro, you can write an expression or list of statements once and then use it any number of times throughout your experiment.

In many ways, macros serve the same purpose that functions or subroutines would in a programming language.  By moving commonly-used macros to separate, :ref:`includable <includes>` files, you can establish a library of useful code that can be shared among multiple experiments, projects, and researchers.

Macros are defined via the ``%define`` directive.  All macro definitions must be at the top level of a source file.  They cannot be defined inside a `protocol <Protocol>` or other component.


Expression
^^^^^^^^^^

An expression macro associates a name with an expression.

Expression macros can be defined in two ways.  For macros that take no parameters, you can use the following syntax::

    %define name = expression

*name* must be an `identifier <identifiers>`.  *expression* can be any `expression <Expressions>`.

Expression macros that do take parameters are defined as follows::

    %define name(param1, param2, ...) expression

Each parameter name must be an `identifier <identifiers>`.  Within *expression*, the parameters can be referred to by name, as if they were variables.

Once defined, an expression macro can be used like a variable name or function call, with the associated expression inserted at the point of invocation::

    %define three = 1 + 2
    %define sum_squares(x, y) x*x + y*y
    %define hypot(a, b) sqrt(sum_squares(a, b))

    var h = hypot(three, 4)  // h == 5

    %define h_is_an_integer = (int)h == h

    var a = 0

    protocol {
        a = 1
        while (a <= 100) {
            h = hypot(a, a+1)
            if (h_is_an_integer) {
                report ('hypot($a, $a+1) = $h')
            }
            a += 1
        }
        // Output:
        //  hypot(3, 3+1) = 5
        //  hypot(20, 20+1) = 29
    }

As shown above, macros can invoke other macros.  However, it is an error for a macro to invoke itself (either directly or indirectly).


.. _statement macros:

Statement
^^^^^^^^^

A statement macro associates a name with a set of statements.  The set of statements can be of any size, from a single `action <Actions>` to an entire experiment.

Statement macro definitions take the following form::

    %define name (param1, param2, ...)
        // Statements
        ...
    %end

As with expression macros, *name* and all parameter names must be `identifiers <identifiers>`, and, inside the macro, parameters are referenced by name (like variables).

Once defined, a statement macro is invoked using :ref:`component declaration <component declarations>` syntax::

    %define present_image (index)
        queue_stimulus (images[index])
        update_stimulus_display ()
    %end

    %define replace_image (old_index, new_index)
        dequeue_stimulus (images[old_index])
        present_image (index = new_index)
    %end

    %define dequeue_all_images ()
        image_index = 0
        while (image_index < num_images) {
            dequeue_stimulus (images[image_index])
            image_index += 1
        }
        update_stimulus_display ()
    %end

    protocol {
        ...
        present_image (0)
        ...
        replace_image (
            old_index = 0
            new_index = 1
        )
        ...
        task {
            ...
            state 'No Fixation' {
                no_fixation = true
                goto ('End Trial')
            }
            state 'Fixation Broken' {
                fixation_broken = true
                dequeue_all_images ()
                goto ('End Trial')
            }
            state 'Success' {
                success = true
                dequeue_all_images ()
                goto ('End Trial')
            }
            ...
        }
        ...
    }

A statement macro invocation can include a tag and/or children only if

1. the macro body declares exactly one component, and
2. the component declaration in the macro body does not include a tag (if the invocation includes a tag) and/or children (if the invocation includes children).

Also, if the macro body is a single :ref:`variable declaration <variable declarations>`, then the invocation can include a default value with ``=`` (as
long as the body does not include one).

For example::

    %define reported_var (message)
        var {
            report (message)
        }
    %end

    // OK: invocation includes a tag, macro body does not
    reported_var x = 3 (message = 'x = $x')

    // Not OK: invocation and macro body both include children
    reported_var y = 3 (message = 'y = $y') {
        z = 2*y
    }


Whitespace
----------

For the most part, whitespace in MWEL code is ignored.  Specifically, the parser discards all space, tab, and carriage return characters (unless they appear within a string literal).

The exception to this rule is newline (aka line feed) characters, which, while often ignored, are syntactically significant in certain places.  Specifically:

* Newlines are *ignored* within

  * Parenthesized expressions
  * Subscript expressions
  * List and dictionary literals
  * Function call and :ref:`macro definition <macros>` parameter lists

* Newlines are *required* at the end of

  * `Assignments`_
  * :ref:`Component <component declarations>` and :ref:`variable <variable declarations>` declarations
  * :ref:`Include statements <includes>`
  * :ref:`Macro definitions <macros>`

* Name/value pairs in a component declaration's :ref:`parameter list <component declaration parameters>` must be separated by either newlines or semicolons.
* The statements in a :ref:`statement macro <statement macros>` definition must be separated from the parameter list by a newline.

Wherever a newline is required, a sequence of multiple newlines, optionally separated by non-newline whitespace, is treated as a single newline.

In general, if you follow the coding style used in the preceding examples, you should not need to be concerned about newline placement.


Converting To and From XML
--------------------------

As noted above, when loading an MWEL experiment, MWorks first converts it into XML.  The tool that performs this conversion, ``mwel2xml``, can also be run from the command line (e.g. in the *Terminal* application) as follows::

    /Library/Application\ Support/MWorks/MWEL/mwel2xml exp.mwel > exp.xml

The generated XML includes location information to improve error reporting, as well as embedded copies of both the main MWEL file and any files :ref:`included <includes>` by it.  To omit these extras, pass the option ``--omit-metadata`` to ``mwel2xml``.

Conversely, to convert an existing XML experiment file into MWEL, use the tool ``xml2mwel``::

    /Library/Application\ Support/MWorks/MWEL/xml2mwel exp.xml > exp.mwel
