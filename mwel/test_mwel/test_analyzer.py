from __future__ import division, print_function, unicode_literals
import unittest

from mwel.analyzer import ExpressionAnalyzer, Component, Analyzer
from mwel.parser import ExpressionParser, Parser

from . import ErrorLoggerMixin, TempFilesMixin


class AnalyzerTestMixin(ErrorLoggerMixin):

    parser_class = Parser
    analyzer_class = Analyzer

    def setUp(self):
        super(AnalyzerTestMixin, self).setUp()

        parser = self.parser_class(self.error_logger)
        self.analyzer = self.analyzer_class(self.error_logger)

        def analyze(*args):
            tree = parser.parse(*args)
            if tree:
                return self.analyzer.analyze(tree)

        self.analyze = self.with_error_check(analyze)

    def assertComponent(self, c, lineno, colno, filename='',
                        name=None, type=None, tag=None, **params):
        self.assertIsInstance(c, Component)
        self.assertEqual(lineno, c.lineno)
        self.assertEqual(colno, c.colno)
        self.assertEqual(filename, c.filename)
        self.assertEqual(name, c.name)
        self.assertEqual(type, c.type)
        self.assertEqual(tag, c.tag)
        self.assertEqual(params, dict(c.params))
        return c.children


class TestExpressionAnalyzer(AnalyzerTestMixin, unittest.TestCase):

    parser_class = ExpressionParser
    analyzer_class = ExpressionAnalyzer

    def test_identifier_expr(self):
        with self.analyze('foo') as s:
            self.assertEqual('foo', s)

    def test_number_literal_expr(self):
        with self.analyze('3') as s:
            self.assertEqual('3', s)

    def test_string_literal_expr(self):
        with self.analyze('""') as s:
            self.assertEqual('""', s)
        with self.analyze("'abc 123'") as s:
            self.assertEqual("'abc 123'", s)

    def test_list_literal_and_range_expr(self):
        with self.analyze('[   ]') as s:
            self.assertEqual('[]', s)
        with self.analyze('[ 1,  2 :4,"foo" , a :  b    : c]') as s:
            self.assertEqual('[1, 2:4, "foo", a:b:c]', s)

    def test_dict_literal_expr(self):
        with self.analyze('{  }') as s:
            self.assertEqual('{}', s)
        with self.analyze('{  "a"  :1,2.5: foo  }') as s:
            self.assertEqual('{"a": 1, 2.5: foo}', s)

    def test_function_call_expr(self):
        with self.analyze('foo (  )') as s:
            self.assertEqual('foo()', s)
        with self.analyze('blah(  1,  "two",3.3  )') as s:
            self.assertEqual('blah(1, "two", 3.3)', s)

    def test_parenthetic_expr(self):
        with self.analyze('( "foo"   )') as s:
            self.assertEqual('("foo")', s)

    def test_units_expr(self):
        with self.analyze('foo   ms') as s:
            self.assertEqual('foo ms', s)

    def test_subscript_expr(self):
        with self.analyze('foo [  3   ]') as s:
            self.assertEqual('foo[3]', s)

    def test_unary_op_expr(self):
        with self.analyze('-   1.5') as s:
            self.assertEqual('-1.5', s)
        with self.analyze('not  foo') as s:
            self.assertEqual('not foo', s)

    def test_cast_expr(self):
        with self.analyze('( float  )  3') as s:
            self.assertEqual('(float)3', s)

    def test_binary_op_expr(self):
        with self.analyze('foo  and    3') as s:
            self.assertEqual('foo and 3', s)


class TestAnalyzer(AnalyzerTestMixin, unittest.TestCase):

    def test_empty(self):
        with self.analyze('') as cmpts:
            self.assertEqual([], cmpts)

    def test_assignment_stmt(self):
        with self.analyze('''
                          foo = 'bar'
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 31,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'foo',
                                            value = "'bar'")
            self.assertEqual([], children)

    def test_augmented_assignment_stmt(self):
        with self.analyze('''
                          foo += 'bar'
                          bar -= 1 + 2
                          ''') as cmpts:
            self.assertEqual(2, len(cmpts))

            children = self.assertComponent(cmpts[0], 2, 31,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'foo',
                                            value = "foo + ('bar')")
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1], 3, 31,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'bar',
                                            value = 'bar - (1 + 2)')
            self.assertEqual([], children)

    def test_index_assignment_stmt(self):
        with self.analyze('''
                          foo [ 3 ] = 'bar'
                          blah [x ] [ y][  z ] -= 1.5
                          ''') as cmpts:
            self.assertEqual(2, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 37,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'foo[3]',
                                            value = "'bar'")
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[1], 3, 48,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'blah[x][y][z]',
                                            value = 'blah[x][y][z] - (1.5)')
            self.assertEqual([], children)

    def test_decl_stmt(self):
        # No parameters or children
        with self.analyze('''
                          foo ()
                          blah bleep {}
                          something/else ()
                          ''') as cmpts:
            self.assertEqual(3, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27, name='foo')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[1], 3, 27,
                                            name = 'blah',
                                            tag = 'bleep')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[2], 4, 27,
                                            name = 'something',
                                            type = 'else')
            self.assertEqual([], children)

        # With params, no children
        with self.analyze('''
                          foo 'The Thing' (
                              bar = 1
                              blob = a,  b, 1+2  ,     d
                              blah = 'floop boop'
                              ham = ""
                              blam = spam
                          )
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'foo',
                                            tag = 'The Thing',
                                            bar = '1',
                                            blob = 'a, b, 1 + 2, d',
                                            blah = 'floop boop',
                                            ham = '',
                                            blam = 'spam')
            self.assertEqual([], children)

        # With params and children
        with self.analyze('''
                          foo/bar blah ( a=1 :  10; b='ham' ) {
                              x = 1
                              y = "two"
                          }
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            cmpts = self.assertComponent(cmpts[0], 2, 27,
                                         name = 'foo',
                                         type = 'bar',
                                         tag = 'blah',
                                         a = '1:10',
                                         b = 'ham')
            self.assertEqual(2, len(cmpts))

            children = self.assertComponent(cmpts[0], 3, 33,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '1')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1], 4, 33,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'y',
                                            value = '"two"')
            self.assertEqual([], children)

    def test_decl_stmt_type_name_inference(self):
        # Single match
        with self.analyze('''
                          report (message = 'An important message')
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'action',
                                            type = 'report',
                                            message = 'An important message')
            self.assertEqual([], children)

        # Multiple matches
        # Need to cheat, because there aren't any overlaps in the wild
        self.analyzer._short_typenames['fizzbizz'] = ['stimulus', 'action']
        with self.analyze('''
                          fizzbizz (a = b)
                          ''') as cmpts:
            self.assertError("Multiple matches for component type 'fizzbizz': "
                             "'stimulus/fizzbizz', 'action/fizzbizz'",
                             lineno = 2,
                             colno = 27)
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'fizzbizz',
                                            a = 'b')
            self.assertEqual([], children)

        # Complete type
        with self.analyze('''
                          protocol {}
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'protocol')
            self.assertEqual([], children)

        # No matches
        with self.analyze('''
                          bizzfizz (b = a)
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'bizzfizz',
                                            b = 'a')
            self.assertEqual([], children)

    def test_decl_stmt_with_value(self):
        # No parameters or children
        with self.analyze('''
                          var foo = 'bar'
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'variable',
                                            tag = 'foo',
                                            default_value = "'bar'")
            self.assertEqual([], children)

        # With params, no children
        with self.analyze('''
                          variable foo = 'bar' (a = b)
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'variable',
                                            tag = 'foo',
                                            default_value = "'bar'",
                                            a = 'b')
            self.assertEqual([], children)

        # With params and children
        with self.analyze('''
                          var foo = 'bar' (a = b) {
                              x = 1
                              y = "two"
                          }
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            cmpts = self.assertComponent(cmpts[0], 2, 27,
                                         name = 'variable',
                                         tag = 'foo',
                                         default_value = "'bar'",
                                         a = 'b')
            self.assertEqual(2, len(cmpts))

            children = self.assertComponent(cmpts[0], 3, 33,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '1')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1], 4, 33,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'y',
                                            value = '"two"')
            self.assertEqual([], children)

        # Not a variable declaration
        with self.analyze('''
                          foo bar = 3
                          variable/selection foo = 4
                          ''') as cmpts:
            self.assertEqual(2, len(cmpts))

            self.assertError('Only variable declarations can specify a '
                             "default value with '='",
                             lineno = 2,
                             colno = 27)
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'foo',
                                            tag = 'bar')
            self.assertEqual([], children)

            self.assertError('Only variable declarations can specify a '
                             "default value with '='",
                             lineno = 3,
                             colno = 27)
            children = self.assertComponent(cmpts[1], 3, 27,
                                            name = 'variable',
                                            type = 'selection',
                                            tag = 'foo')
            self.assertEqual([], children)

    def test_decl_stmt_parameter_name_inference(self):
        # Single param
        with self.analyze('''
                          action/update_stimulus_display ('foo')
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'action',
                                            type = 'update_stimulus_display',
                                            predicted_output_time = 'foo')
            self.assertEqual([], children)

        # Single required param
        with self.analyze('''
                          variable/selection sel (1 ,  2  ,3)
                          report ('This is a test')
                          ''') as cmpts:
            self.assertEqual(2, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'variable',
                                            type = 'selection',
                                            tag = 'sel',
                                            values = '1, 2, 3')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[1], 3, 27,
                                            name = 'action',
                                            type = 'report',
                                            message = 'This is a test')
            self.assertEqual([], children)

        # No params
        with self.analyze('''
                          action/pause_experiment (4)
                          ''') as cmpts:
            self.assertError('Cannot infer parameter name for component '
                             "'action/pause_experiment'",
                             lineno = 2,
                             colno = 27)
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'action',
                                            type = 'pause_experiment',
                                            __UNKNOWN__ = '4')
            self.assertEqual([], children)

        # Multiple required params
        with self.analyze('''
                          action/assignment (x)
                          ''') as cmpts:
            self.assertError('Cannot infer parameter name for component '
                             "'action/assignment'",
                             lineno = 2,
                             colno = 27)
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'action',
                                            type = 'assignment',
                                            __UNKNOWN__ = 'x')
            self.assertEqual([], children)

        # Unknown component
        with self.analyze('''
                          flipflopbop ('foo')
                          ''') as cmpts:
            self.assertError('Cannot infer parameter name for component '
                             "'flipflopbop'",
                             lineno = 2,
                             colno = 27)
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 27,
                                            name = 'flipflopbop',
                                            __UNKNOWN__ = 'foo')
            self.assertEqual([], children)

    def test_alias(self):
        with self.analyze('''
                          // Without inference
                          action/show_stimulus (stimulus = stim)
                          // With inference
                          hide_stimulus (stim)
                          ''') as cmpts:
            self.assertEqual(2, len(cmpts))

            children = self.assertComponent(cmpts[0], 3, 27,
                                            name = 'action',
                                            type = 'queue_stimulus',
                                            stimulus = 'stim')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1], 5, 27,
                                            name = 'action',
                                            type = 'dequeue_stimulus',
                                            stimulus = 'stim')
            self.assertEqual([], children)

    def test_mwel_only_alias(self):
        with self.analyze('''
                          // Without inference
                          var foo (default_value = 3)
                          // With inference
                          var bar (4)
                          ''') as cmpts:
            self.assertEqual(2, len(cmpts))

            children = self.assertComponent(cmpts[0], 3, 27,
                                            name = 'variable',
                                            tag = 'foo',
                                            default_value = '3')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1], 5, 27,
                                            name = 'variable',
                                            tag = 'bar',
                                            default_value = '4')
            self.assertEqual([], children)

    def test_require(self):
        with self.analyze('''
                          %define foo = 'Hello, world!'
                          %define bar (x, y)
                              report ('x = $x, y = $y')
                          %end
                          %require foo
                          %require blah
                          %require foo, bar
                          %require foo, baz, bar
                          %define blah
                          %define baz = 3
                          %require blah, baz
                          ''') as cmpts:
            self.assertError("Macro 'blah' is not defined",
                             lineno = 7,
                             colno = 28)
            self.assertError("Macro 'baz' is not defined",
                             lineno = 9,
                             colno = 28)

            self.assertEqual(0, len(cmpts))


class TestIncludes(AnalyzerTestMixin, TempFilesMixin, unittest.TestCase):

    def test_basic(self):
        base_src = '''\
var foo = 1
%include include1
foo = 2
'''

        include1_src = '''\
bar[1] = 2
%include include2
foo/bar baz {}
'''
        include1_path = self.write_file('include1.mwel', include1_src)

        include2_src = '''\
range_replicator (variable = blah; from = 1; to = 3; step = 1) {
}
list_replicator (variable = blah; values = a, b, c) {
}
'''
        include2_path = self.write_file('include2.mwel', include2_src)

        with self.analyze(base_src, self.tmpdir) as cmpts:
            self.assertEqual(6, len(cmpts))

            children = self.assertComponent(cmpts[0], 1, 1,
                                            name = 'variable',
                                            tag = 'foo',
                                            default_value = '1')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1], 1, 8,
                                            filename = include1_path,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'bar[1]',
                                            value = '2')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[2], 1, 1,
                                            filename = include2_path,
                                            name = 'range_replicator',
                                            **{
                                                'variable': 'blah',
                                                'from': '1',
                                                'to': '3',
                                                'step': '1',
                                                })
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[3], 3, 1,
                                            filename = include2_path,
                                            name = 'list_replicator',
                                            variable = 'blah',
                                            values = 'a, b, c')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[4], 3, 1,
                                            filename = include1_path,
                                            name = 'foo',
                                            type = 'bar',
                                            tag = 'baz')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[5], 3, 5,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'foo',
                                            value = '2')
            self.assertEqual([], children)

    def test_nested_errors(self):
        base_src = '''\
%include include1
foo (1)
'''

        self.analyzer._short_typenames['bar'] = ['stimulus', 'action']
        include1_src = '''\
%include include2
bar ()
'''
        include1_path = self.write_file('include1.mwel', include1_src)

        include2_src = '''\
blah (3)
'''
        include2_path = self.write_file('include2.mwel', include2_src)

        with self.analyze(base_src, self.tmpdir) as cmpts:
            self.assertEqual(3, len(cmpts))

            self.assertError("Cannot infer parameter name for component 'blah'",
                             lineno = 1,
                             colno = 1,
                             filename = include2_path)
            children = self.assertComponent(cmpts[0], 1, 1,
                                            filename = include2_path,
                                            name = 'blah',
                                            __UNKNOWN__ = '3')
            self.assertEqual([], children)

            self.assertError("Multiple matches for component type 'bar': "
                             "'stimulus/bar', 'action/bar'",
                             lineno = 2,
                             colno = 1,
                             filename = include1_path)
            children = self.assertComponent(cmpts[1], 2, 1,
                                            filename = include1_path,
                                            name = 'bar')
            self.assertEqual([], children)

            self.assertError("Cannot infer parameter name for component 'foo'",
                             lineno = 2,
                             colno = 1)
            children = self.assertComponent(cmpts[2], 2, 1,
                                            name = 'foo',
                                            __UNKNOWN__ = '1')
            self.assertEqual([], children)

    def test_backtrace(self):
        base_src = '''\
%include include1
set_x_2(3)
'''

        include1_src = '''\
%include include2
set_x(2)
%define set_x_2(value)
    set_x(value)
%end
'''
        include1_path = self.write_file('include1.mwel', include1_src)

        include2_src = '''\
var x = 0
%define set_x(value)
    x = value
%end
set_x(1)
'''
        include2_path = self.write_file('include2.mwel', include2_src)

        with self.analyze(base_src, self.tmpdir) as cmpts:
            self.assertEqual(4, len(cmpts))

            children = self.assertComponent(cmpts[0], 1, 1,
                                            filename = include2_path,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = '0')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1],
                                            (3, 5),
                                            (7, 1),
                                            filename = (include2_path,
                                                        include2_path),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '1')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[2],
                                            (3, 2),
                                            (7, 1),
                                            filename = (include2_path,
                                                        include1_path),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '2')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[3],
                                            (3, 4, 2),
                                            (7, 5, 1),
                                            filename = (include2_path,
                                                        include1_path,
                                                        ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '3')
            self.assertEqual([], children)


class TestExpressionMacros(AnalyzerTestMixin, unittest.TestCase):

    def test_simple(self):
        # Basics
        with self.analyze('''
                          %define foo = 'Hello, world!'
                          %define bar = 1+2
                          %define foo = 7  // Redefinition not allowed
                          var x = foo   // Atomic expansion
                          var y = bar   // Non-atomic expansion
                          report (foo)  // Unquoted expansion
                          ''') as cmpts:
            self.assertError("Macro 'foo' is already defined",
                             lineno = 4,
                             colno = 28)

            self.assertEqual(3, len(cmpts))

            children = self.assertComponent(cmpts[0], 5, 27,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = "'Hello, world!'")
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1], 6, 27,
                                            name = 'variable',
                                            tag = 'y',
                                            default_value = '(1 + 2)')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[2], 7, 27,
                                            name = 'action',
                                            type = 'report',
                                            message = 'Hello, world!')
            self.assertEqual([], children)

        # Recursive expansion
        with self.analyze('''
                          %define a = 1+b
                          %define b = 2+c
                          %define c = 3+d
                          %define d = 4
                          var x = a
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))

            children = self.assertComponent(cmpts[0], 6, 27,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = '(1 + (2 + (3 + 4)))')
            self.assertEqual([], children)

        # Circular expansion
        with self.analyze('''
                          %define aa = 1+bb
                          %define bb = 2+aa
                          var x = aa
                          var y = aa
                          %define aa = 3  // Still can't redefine
                          ''') as cmpts:
            self.assertError("Definition of macro 'aa' is self-referential",
                             lineno = 2,
                             colno = 28)
            self.assertError("Macro 'aa' is already defined",
                             lineno = 6,
                             colno = 28)

            self.assertEqual(2, len(cmpts))

            children = self.assertComponent(cmpts[0], 4, 27,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = '(1 + (2 + aa))')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1], 5, 27,
                                            name = 'variable',
                                            tag = 'y',
                                            default_value = 'aa')
            self.assertEqual([], children)

    def test_parametrized(self):
        # Basics
        with self.analyze('''
                          %define f = 1
                          %define f0() 1
                          %define f1(x) 1+x
                          %define f2(x,y) 1+x+y
                          var a = f()
                          var b = f0()
                          var c = f1(2)
                          var d = f2(2,3)

                          %define echo(x) x
                          // Quoted parameter expansion
                          var e = echo('foo')
                          // Unquoted parameter expansion
                          queue_stimulus (echo('foo'))

                          // Wrong number of parameters
                          var f = f(1)
                          var g = f0(1)
                          var h = f1()
                          var i = f1(1,2)
                          var j = f2(1)
                          var k = f2(1,2,3)

                          // Can't be used as statement
                          f0()
                          ''') as cmpts:
            self.assertEqual(13, len(cmpts))

            children = self.assertComponent(cmpts[0], 6, 27,
                                            name = 'variable',
                                            tag = 'a',
                                            default_value = '1')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1], 7, 27,
                                            name = 'variable',
                                            tag = 'b',
                                            default_value = '1')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[2], 8, 27,
                                            name = 'variable',
                                            tag = 'c',
                                            default_value = '(1 + 2)')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[3], 9, 27,
                                            name = 'variable',
                                            tag = 'd',
                                            default_value = '(1 + 2 + 3)')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[4], 13, 27,
                                            name = 'variable',
                                            tag = 'e',
                                            default_value = "'foo'")
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[5], 15, 27,
                                            name = 'action',
                                            type = 'queue_stimulus',
                                            stimulus = 'foo')
            self.assertEqual([], children)

            self.assertError("Macro 'f' takes no parameters",
                             lineno = 18,
                             colno = 36)
            children = self.assertComponent(cmpts[6], 18, 27,
                                            name = 'variable',
                                            tag = 'f',
                                            default_value = 'f(1)')
            self.assertEqual([], children)

            self.assertError("Macro 'f0' takes no parameters",
                             lineno = 19,
                             colno = 37)
            children = self.assertComponent(cmpts[7], 19, 27,
                                            name = 'variable',
                                            tag = 'g',
                                            default_value = 'f0(1)')
            self.assertEqual([], children)

            self.assertError("Macro 'f1' takes 1 parameter",
                             lineno = 20,
                             colno = 37)
            children = self.assertComponent(cmpts[8], 20, 27,
                                            name = 'variable',
                                            tag = 'h',
                                            default_value = 'f1()')
            self.assertEqual([], children)

            self.assertError("Macro 'f1' takes 1 parameter",
                             lineno = 21,
                             colno = 37)
            children = self.assertComponent(cmpts[9], 21, 27,
                                            name = 'variable',
                                            tag = 'i',
                                            default_value = 'f1(1, 2)')
            self.assertEqual([], children)

            self.assertError("Macro 'f2' takes 2 parameters",
                             lineno = 22,
                             colno = 37)
            children = self.assertComponent(cmpts[10], 22, 27,
                                            name = 'variable',
                                            tag = 'j',
                                            default_value = 'f2(1)')
            self.assertEqual([], children)

            self.assertError("Macro 'f2' takes 2 parameters",
                             lineno = 23,
                             colno = 37)
            children = self.assertComponent(cmpts[11], 23, 27,
                                            name = 'variable',
                                            tag = 'k',
                                            default_value = 'f2(1, 2, 3)')
            self.assertEqual([], children)

            self.assertError("Macro 'f0' cannot be used as a statement",
                             lineno = 26,
                             colno = 27)
            children = self.assertComponent(cmpts[12], 26, 27,
                                            name = 'f0')
            self.assertEqual([], children)

        # Recursive expansion
        with self.analyze('''
                          %define a(x) y + b(x)
                          %define b(y) x + c(y)
                          %define c(x) x * y
                          var x = a(1+2)

                          // Quoted parameter expansion
                          var y = a('foo')

                          // Unquoted parameter expansion
                          queue_stimulus (a('foo'))
                          %define echo2(x) echo(x)
                          queue_stimulus (echo2(foo))
                          ''') as cmpts:
            self.assertEqual(4, len(cmpts))

            expected = '(y + (x + ((1 + 2) * y)))'
            children = self.assertComponent(cmpts[0], 5, 27,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = expected)
            self.assertEqual([], children)

            expected = "(y + (x + ('foo' * y)))"
            children = self.assertComponent(cmpts[1], 8, 27,
                                            name = 'variable',
                                            tag = 'y',
                                            default_value = expected)
            self.assertEqual([], children)

            expected = "y + (x + ('foo' * y))"
            children = self.assertComponent(cmpts[2], 11, 27,
                                            name = 'action',
                                            type = 'queue_stimulus',
                                            stimulus = expected)
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[3], 13, 27,
                                            name = 'action',
                                            type = 'queue_stimulus',
                                            stimulus = 'foo')
            self.assertEqual([], children)


class TestStatementMacros(AnalyzerTestMixin, unittest.TestCase):

    def test_no_args(self):
        with self.analyze('''
                          %define inc_and_announce()
                              x = x+1
                              report ('Incremented x')
                          %end

                          var x = 0
                          inc_and_announce()
                          var y = 1
                          inc_and_announce()

                          // Can't be used as expression
                          var z = inc_and_announce()

                          // Redefinition not allowed
                          %define inc_and_announce()
                              x = x+2
                          %end
                          ''') as cmpts:
            self.assertEqual(7, len(cmpts))

            children = self.assertComponent(cmpts[0], 7, 27,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = '0')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1],
                                            (3, 8),
                                            (33, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = 'x + 1')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[2],
                                            (4, 8),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'report',
                                            message = 'Incremented x')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[3], 9, 27,
                                            name = 'variable',
                                            tag = 'y',
                                            default_value = '1')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[4],
                                            (3, 10),
                                            (33, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = 'x + 1')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[5],
                                            (4, 10),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'report',
                                            message = 'Incremented x')
            self.assertEqual([], children)

            self.assertError("Macro 'inc_and_announce' cannot be used as an "
                             "expression",
                             lineno = 13,
                             colno = 51)
            children = self.assertComponent(cmpts[6], 13, 27,
                                            name = 'variable',
                                            tag = 'z',
                                            default_value = 'inc_and_announce()')
            self.assertEqual([], children)

            self.assertError("Macro 'inc_and_announce' is already defined",
                             lineno = 16,
                             colno = 28)

    def test_single_arg(self):
        with self.analyze('''
                          %define add_and_announce(y)
                              x = x+y
                              report ('Added y to x')
                          %end

                          var x = 1
                          add_and_announce(2)
                          var z = 3
                          add_and_announce(z+4)

                          // Wrong number of parameters
                          add_and_announce()
                          add_and_announce(5,6)

                          // Named arg
                          add_and_announce(y=7)
                          ''') as cmpts:
            self.assertEqual(11, len(cmpts))

            children = self.assertComponent(cmpts[0], 7, 27,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = '1')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1],
                                            (3, 8),
                                            (33, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = 'x + 2')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[2],
                                            (4, 8),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'report',
                                            message = 'Added y to x')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[3], 9, 27,
                                            name = 'variable',
                                            tag = 'z',
                                            default_value = '3')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[4],
                                            (3, 10),
                                            (33, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = 'x + (z + 4)')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[5],
                                            (4, 10),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'report',
                                            message = 'Added y to x')
            self.assertEqual([], children)

            self.assertError("Macro 'add_and_announce' takes 1 parameter",
                             lineno = 13,
                             colno = 27)
            children = self.assertComponent(cmpts[6], 13, 27,
                                            name = 'add_and_announce')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[7],
                                            (3, 14),
                                            (33, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = 'x + (5, 6)')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[8],
                                            (4, 14),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'report',
                                            message = 'Added y to x')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[9],
                                            (3, 17),
                                            (33, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = 'x + 7')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[10],
                                            (4, 17),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'report',
                                            message = 'Added y to x')
            self.assertEqual([], children)

    def test_multiple_args(self):
        with self.analyze('''
                          %define add_and_announce(y, z)
                              x = x+y*z
                              report ('Added y*z to x')
                          %end

                          var x = 1
                          add_and_announce(y=2; z=3)
                          var z = 4
                          add_and_announce(z = z+6
                                           y = z+5)

                          // Missing parameter
                          add_and_announce(y=2)

                          // Extra parameter
                          add_and_announce(y=2; z=3; w=4)

                          // Invalid parameter
                          add_and_announce(y=2; w=3)
                          ''') as cmpts:
            self.assertEqual(9, len(cmpts))

            children = self.assertComponent(cmpts[0], 7, 27,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = '1')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1],
                                            (3, 8),
                                            (33, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = 'x + 2 * 3')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[2],
                                            (4, 8),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'report',
                                            message = 'Added y*z to x')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[3], 9, 27,
                                            name = 'variable',
                                            tag = 'z',
                                            default_value = '4')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[4],
                                            (3, 10),
                                            (33, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = 'x + (z + 5) * (z + 6)')
            self.assertEqual([], children)
            children = self.assertComponent(cmpts[5],
                                            (4, 10),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'report',
                                            message = 'Added y*z to x')
            self.assertEqual([], children)

            self.assertError("Macro 'add_and_announce' takes 2 parameters",
                             lineno = 14,
                             colno = 27)
            children = self.assertComponent(cmpts[6], 14, 27,
                                            name = 'add_and_announce',
                                            y = '2')
            self.assertEqual([], children)

            self.assertError("Macro 'add_and_announce' has no parameter 'w'",
                             lineno = 17,
                             colno = 27)
            children = self.assertComponent(cmpts[7], 17, 27,
                                            name = 'add_and_announce',
                                            y = '2',
                                            z = '3',
                                            w = '4')
            self.assertEqual([], children)

            self.assertError("Macro 'add_and_announce' has no parameter 'w'",
                             lineno = 20,
                             colno = 27)
            children = self.assertComponent(cmpts[8], 20, 27,
                                            name = 'add_and_announce',
                                            y = '2',
                                            w = '3')
            self.assertEqual([], children)

    def test_circular_expansion(self):
        with self.analyze('''
                          %define f()
                              g()
                          %end
                          %define g()
                              f()
                          %end
                          f()
                          f()
                          ''') as cmpts:
            self.assertEqual(2, len(cmpts))

            self.assertError("Definition of macro 'f' is self-referential",
                             lineno = 2,
                             colno = 28)

            children = self.assertComponent(cmpts[0],
                                            (6, 3, 8),
                                            (31, 31, 27),
                                            filename = ('', '', ''),
                                            name = 'f')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1], 9, 27,
                                            name = 'f')
            self.assertEqual([], children)

    def test_recursive_expansion(self):
        with self.analyze('''
                          %define a(x)
                              var foo = y
                              b(x)
                          %end

                          %define b(y)
                              var bar = x
                              c(y)
                          %end

                          %define c(x)
                              var blah = x  // Quoted expansion
                              queue_stimulus (x)  // Unquoted expansion
                              bar = y
                          %end

                          a(1+2)
                          a('floop')
                          ''') as cmpts:
            self.assertEqual(10, len(cmpts))

            children = self.assertComponent(cmpts[0],
                                            (3, 18),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'variable',
                                            tag = 'foo',
                                            default_value = 'y')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[1],
                                            (8, 4, 18),
                                            (31, 31, 27),
                                            filename = ('', '', ''),
                                            name = 'variable',
                                            tag = 'bar',
                                            default_value = 'x')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[2],
                                            (13, 9, 4, 18),
                                            (31, 31, 31, 27),
                                            filename = ('', '', '', ''),
                                            name = 'variable',
                                            tag = 'blah',
                                            default_value = '(1 + 2)')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[3],
                                            (14, 9, 4, 18),
                                            (31, 31, 31, 27),
                                            filename = ('', '', '', ''),
                                            name = 'action',
                                            type = 'queue_stimulus',
                                            stimulus = '1 + 2')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[4],
                                            (15, 9, 4, 18),
                                            (35, 31, 31, 27),
                                            filename = ('', '', '', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'bar',
                                            value = 'y')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[5],
                                            (3, 19),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'variable',
                                            tag = 'foo',
                                            default_value = 'y')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[6],
                                            (8, 4, 19),
                                            (31, 31, 27),
                                            filename = ('', '', ''),
                                            name = 'variable',
                                            tag = 'bar',
                                            default_value = 'x')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[7],
                                            (13, 9, 4, 19),
                                            (31, 31, 31, 27),
                                            filename = ('', '', '', ''),
                                            name = 'variable',
                                            tag = 'blah',
                                            default_value = "'floop'")
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[8],
                                            (14, 9, 4, 19),
                                            (31, 31, 31, 27),
                                            filename = ('', '', '', ''),
                                            name = 'action',
                                            type = 'queue_stimulus',
                                            stimulus = 'floop')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[9],
                                            (15, 9, 4, 19),
                                            (35, 31, 31, 27),
                                            filename = ('', '', '', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'bar',
                                            value = 'y')
            self.assertEqual([], children)

    def test_children_in_expansion(self):
        with self.analyze('''
                          %define a(x)
                              block {
                                  trial {
                                      var foo = x
                                  }
                              }
                          %end

                          a(3)
                          ''') as cmpts:
            self.assertEqual(1, len(cmpts))

            children = self.assertComponent(cmpts[0],
                                            (3, 10),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'block')
            self.assertEqual(1, len(children))

            children = self.assertComponent(children[0],
                                            (4, 10),
                                            (35, 27),
                                            filename = ('', ''),
                                            name = 'trial')
            self.assertEqual(1, len(children))

            children = self.assertComponent(children[0],
                                            (5, 10),
                                            (39, 27),
                                            filename = ('', ''),
                                            name = 'variable',
                                            tag = 'foo',
                                            default_value = '3')
            self.assertEqual([], children)

    def test_tag_and_children(self):
        with self.analyze('''
                          %define multiple_components()
                              block ()
                              trial ()
                          %end

                          %define has_tag()
                              block my_block ()
                          %end

                          %define has_children()
                              block {
                                  x = 1
                              }
                          %end

                          %define no_tag_or_children()
                              block ()
                          %end

                          multiple_components foo ()
                          multiple_components {
                              x = 3
                          }

                          has_tag bar ()
                          has_tag {
                              y = 7
                          }

                          has_children blah ()
                          has_children {
                              y = 12
                          }

                          no_tag_or_children florp ()
                          no_tag_or_children {
                              x = 5
                          }
                          no_tag_or_children blorp {
                              x = 6
                          }

                          %define children_in_enclosing_macro (a, b)
                              no_tag_or_children {
                                  x = a + b
                              }
                          %end

                          children_in_enclosing_macro morp (a=7; b=8)
                          ''') as cmpts:
            self.assertEqual(10, len(cmpts))

            self.assertError("Macro body declares 2 components, so invocation "
                             "cannot include a tag, a default value with '=', "
                             "or child components",
                             lineno = 21,
                             colno = 27)
            children = self.assertComponent(cmpts[0], 21, 27,
                                            name = 'multiple_components',
                                            tag = 'foo')
            self.assertEqual([], children)

            self.assertError("Macro body declares 2 components, so invocation "
                             "cannot include a tag, a default value with '=', "
                             "or child components",
                             lineno = 22,
                             colno = 27)
            children = self.assertComponent(cmpts[1], 22, 27,
                                            name = 'multiple_components')
            self.assertEqual(1, len(children))
            children = self.assertComponent(children[0], 23, 33,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '3')
            self.assertEqual([], children)

            self.assertError("Macro body contains a tag, so invocation cannot "
                             "include one",
                             lineno = 26,
                             colno = 27)
            children = self.assertComponent(cmpts[2], 26, 27,
                                            name = 'has_tag',
                                            tag = 'bar')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[3],
                                            (8, 27),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'block',
                                            tag = 'my_block')
            self.assertEqual(1, len(children))
            children = self.assertComponent(children[0], 28, 33,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'y',
                                            value = '7')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[4],
                                            (12, 31),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'block',
                                            tag = 'blah')
            self.assertEqual(1, len(children))
            children = self.assertComponent(children[0],
                                            (13, 31),
                                            (37, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '1')
            self.assertEqual([], children)

            self.assertError("Macro body contains children, so invocation "
                             "cannot include them",
                             lineno = 32,
                             colno = 27)
            children = self.assertComponent(cmpts[5], 32, 27,
                                            name = 'has_children')
            self.assertEqual(1, len(children))
            children = self.assertComponent(children[0], 33, 33,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'y',
                                            value = '12')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[6],
                                            (18, 36),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'block',
                                            tag = 'florp')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[7],
                                            (18, 37),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'block')
            self.assertEqual(1, len(children))
            children = self.assertComponent(children[0], 38, 33,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '5')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[8],
                                            (18, 40),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'block',
                                            tag = 'blorp')
            self.assertEqual(1, len(children))
            children = self.assertComponent(children[0], 41, 33,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '6')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[9],
                                            (18, 45, 50),
                                            (31, 31, 27),
                                            filename = ('', '', ''),
                                            name = 'block',
                                            tag = 'morp')
            self.assertEqual(1, len(children))
            children = self.assertComponent(children[0],
                                            (46, 50),
                                            (37, 27),
                                            filename = ('', ''),
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '7 + 8')
            self.assertEqual([], children)

    def test_value(self):
        with self.analyze('''
                          %define multiple_components()
                              variable var1 ()
                              variable var2 ()
                          %end

                          %define no_value_not_variable()
                              block ()
                          %end

                          %define no_value_selection_variable()
                              variable/selection ()
                          %end

                          %define no_value_variable()
                              variable ()
                          %end

                          %define no_value_var()
                              var ()
                          %end

                          multiple_components foo = 1
                          no_value_not_variable bar = 2
                          no_value_selection_variable blah = 3
                          no_value_variable baz = 4
                          no_value_var flaz = 5

                          %define recursive_no_value_var(a, b)
                              no_value_var var1 = a+1
                              no_value_var var2 = b+2
                          %end

                          recursive_no_value_var (a=6; b=7)

                          %define with_params(logging)
                              variable (logging = logging)
                          %end

                          with_params fraz = 8 (logging = never)
                          ''') as cmpts:
            self.assertEqual(8, len(cmpts))

            self.assertError("Macro body declares 2 components, so invocation "
                             "cannot include a tag, a default value with '=', "
                             "or child components",
                             lineno = 23,
                             colno = 27)
            self.assertError('Only variable declarations can specify a '
                             "default value with '='",
                             lineno = 23,
                             colno = 27)
            children = self.assertComponent(cmpts[0], 23, 27,
                                            name = 'multiple_components',
                                            tag = 'foo')
            self.assertEqual([], children)

            self.assertError("Macro body does not contain a variable "
                             "declaration, so invocation cannot include a "
                             "default value with '='",
                             lineno = 24,
                             colno = 27)
            self.assertError('Only variable declarations can specify a '
                             "default value with '='",
                             lineno = 24,
                             colno = 27)
            children = self.assertComponent(cmpts[1], 24, 27,
                                            name = 'no_value_not_variable',
                                            tag = 'bar')
            self.assertEqual([], children)

            self.assertError("Macro body does not contain a variable "
                             "declaration, so invocation cannot include a "
                             "default value with '='",
                             lineno = 25,
                             colno = 27)
            self.assertError('Only variable declarations can specify a '
                             "default value with '='",
                             lineno = 25,
                             colno = 27)
            children = self.assertComponent(cmpts[2], 25, 27,
                                            name = 'no_value_selection_variable',
                                            tag = 'blah')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[3],
                                            (16, 26),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'variable',
                                            tag = 'baz',
                                            default_value = '4')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[4],
                                            (20, 27),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'variable',
                                            tag = 'flaz',
                                            default_value = '5')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[5],
                                            (20, 30, 34),
                                            (31, 31, 27),
                                            filename = ('', '', ''),
                                            name = 'variable',
                                            tag = 'var1',
                                            default_value = '6 + 1')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[6],
                                            (20, 31, 34),
                                            (31, 31, 27),
                                            filename = ('', '', ''),
                                            name = 'variable',
                                            tag = 'var2',
                                            default_value = '7 + 2')
            self.assertEqual([], children)

            children = self.assertComponent(cmpts[7],
                                            (37, 40),
                                            (31, 27),
                                            filename = ('', ''),
                                            name = 'variable',
                                            tag = 'fraz',
                                            default_value = '8',
                                            logging = 'never')
            self.assertEqual([], children)
