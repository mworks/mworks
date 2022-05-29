import collections
import os
import unittest

from mwel import ast
from mwel.parser import ExpressionParser, Parser

from . import ErrorLoggerMixin, TempFilesMixin


class ParserTestMixin(ErrorLoggerMixin):

    parser_class = Parser

    foo = ast.IdentifierExpr(value='foo')
    bar = ast.IdentifierExpr(value='bar')

    true = ast.IdentifierExpr(value='true')

    one = ast.NumberLiteralExpr(value='1')
    two = ast.NumberLiteralExpr(value='2')
    three = ast.NumberLiteralExpr(value='3')

    one_plus_foo = ast.BinaryOpExpr(op='+', operands=(one, foo))

    def setUp(self):
        super(ParserTestMixin, self).setUp()
        self.parser = self.parser_class(self.error_logger)
        self.parse = self.with_error_check(self.parser.parse)

    def assertExpected(self, expected, got=None, lineno=None, colno=None):
        if len(expected) == 1:
            expected = repr(str(expected))
        msg = 'Expected ' + expected
        if got:
            msg += ', found %r' % str(got)
        self.assertError(msg, got, lineno, colno)

    def assertLocation(self, p, lineno, colno):
        self.assertEqual(lineno, p.lineno)
        self.assertEqual(colno, p.colno)


class TestExpressions(ParserTestMixin, unittest.TestCase):

    parser_class = ExpressionParser

    def test_incomplete_input(self):
        with self.parse('1+') as p:
            self.assertError('Input ended unexpectedly')
            self.assertIsNone(p)

    def test_identifier_expr(self):
        with self.parse('foo') as p:
            self.assertIsInstance(p, ast.IdentifierExpr)
            self.assertLocation(p, 1, 1)
            self.assertEqual('foo', p.value)

    def test_number_literal_expr(self):
        with self.parse('123') as p:
            self.assertIsInstance(p, ast.NumberLiteralExpr)
            self.assertLocation(p, 1, 1)
            self.assertEqual('123', p.value)

    def test_string_literal_expr(self):
        with self.parse('"foo bar blah"') as p:
            self.assertIsInstance(p, ast.StringLiteralExpr)
            self.assertLocation(p, 1, 1)
            self.assertEqual('"foo bar blah"', p.value)

    def test_list_literal_expr(self):
        def test_list(expr, *items):
            with self.parse(expr) as p:
                self.assertIsInstance(p, ast.ListLiteralExpr)
                self.assertLocation(p, 1, 1)
                self.assertIsInstance(p.items, tuple)
                self.assertEqual(items, p.items)

        # Basics
        test_list('[]')
        test_list('[1]', self.one)
        test_list('[1,2]', self.one, self.two)
        test_list('[1,2,foo]', self.one, self.two, self.foo)

        # Extra comma
        with self.parse('[,]'):
            self.assertError(token=',')
        with self.parse('[1,]'):
            self.assertError(token=']')

        # Missing comma
        with self.parse('[1 2]'):
            self.assertExpected(']', got='2')

        # Missing range stop
        with self.parse('[1:]'):
            self.assertError(token=']')

        # Missing range step
        with self.parse('[1:2:]'):
            self.assertError(token=']')

        # Embedded newlines and range expressions
        test_list('[ \n  \n\n  \n ]')
        with self.parse('''\
                        [
                            1,
                            1 :
                              2
                              ,
                            foo,
                            foo:bar:2
                        ]\
                        ''') as p:
            self.assertIsInstance(p, ast.ListLiteralExpr)
            items = p.items
            self.assertEqual(4, len(items))
            self.assertEqual(self.one, items[0])
            self.assertEqual(self.foo, items[2])

            p = items[1]
            self.assertIsInstance(p, ast.RangeExpr)
            self.assertLocation(p, 3, 31)
            self.assertEqual(self.one, p.start)
            self.assertEqual(self.two, p.stop)
            self.assertIsNone(p.step)

            p = items[3]
            self.assertIsInstance(p, ast.RangeExpr)
            self.assertLocation(p, 7, 32)
            self.assertEqual(self.foo, p.start)
            self.assertEqual(self.bar, p.stop)
            self.assertEqual(self.two, p.step)

    def test_dict_literal_expr(self):
        def test_dict(expr, *items):
            with self.parse(expr) as p:
                self.assertIsInstance(p, ast.DictLiteralExpr)
                self.assertLocation(p, 1, 1)
                self.assertIsInstance(p.items, tuple)
                items = tuple(zip(items[::2], items[1::2]))
                self.assertEqual(items, p.items)

        # Basics
        test_dict('{}')
        test_dict('{foo:1}', self.foo, self.one)
        test_dict('{foo:1, 2:bar}', self.foo, self.one, self.two, self.bar)

        # Extra comma
        with self.parse('{,}'):
            self.assertError(token=',')
        with self.parse('{a: 1,}'):
            self.assertError(token='}')

        # Missing colon
        with self.parse('{foo 3}'):
            self.assertExpected(':', got='3')

        # Missing key
        with self.parse('{:3}'):
            self.assertError(token=':')

        # Missing value
        with self.parse('{foo:}'):
            self.assertError(token='}')

        # Missing right brace
        with self.parse('{foo: 3 2'):
            self.assertExpected('}', got='2')

        # Embedded newlines
        test_dict('{ \n  \n\n  \n }')
        test_dict('''{
                      2
                      :
                      1
                      +
                      foo
                      ,

                      foo :
                      1
                  }''',
                  self.two, self.one_plus_foo,
                  self.foo, self.one,
                  )

    def test_function_call_expr(self):
        def test_call(expr, colno, name, *args):
            with self.parse(expr) as p:
                self.assertIsInstance(p, ast.FunctionCallExpr)
                self.assertLocation(p, 1, colno)
                self.assertIsInstance(p.name, str)
                self.assertEqual(name, p.name)
                self.assertIsInstance(p.args, tuple)
                self.assertEqual(args, p.args)

        # Basics
        test_call('foo()', 4, 'foo')
        test_call('foo(1)', 4, 'foo', self.one)
        test_call('abc123(1,2)', 7, 'abc123', self.one, self.two)
        test_call('abc123(1,2,foo)', 7, 'abc123', self.one, self.two, self.foo)

        # Extra comma
        with self.parse('foo(,)'):
            self.assertError(token=',')
        with self.parse('foo(1,)'):
            self.assertError(token=')')

        # Missing comma
        with self.parse('foo(1 2)'):
            self.assertExpected(')', got='2')

        # Missing range stop
        with self.parse('foo(1:)'):
            self.assertError(token=')')

        # Missing range step
        with self.parse('foo(1:2:)'):
            self.assertError(token=')')

        # Embedded newlines and range expressions
        test_call('foo( \n  \n\n  \n )', 4, 'foo')
        with self.parse('''\
                        foo (
                            1,

                            1
                              :
                              2
                              ,

                            foo,


                            foo
                            : bar :
                              2

                        )''') as p:
            self.assertIsInstance(p, ast.FunctionCallExpr)
            self.assertEqual('foo', p.name)
            args = p.args
            self.assertEqual(4, len(args))
            self.assertEqual(self.one, args[0])
            self.assertEqual(self.foo, args[2])

            p = args[1]
            self.assertIsInstance(p, ast.RangeExpr)
            self.assertLocation(p, 5, 31)
            self.assertEqual(self.one, p.start)
            self.assertEqual(self.two, p.stop)
            self.assertIsNone(p.step)

            p = args[3]
            self.assertIsInstance(p, ast.RangeExpr)
            self.assertLocation(p, 13, 29)
            self.assertEqual(self.foo, p.start)
            self.assertEqual(self.bar, p.stop)
            self.assertEqual(self.two, p.step)

    def test_parenthetic_expr(self):
        def test_parens(src, expr):
            with self.parse(src) as p:
                self.assertIsInstance(p, ast.ParentheticExpr)
                self.assertLocation(p, 1, 1)
                self.assertEqual(expr, p.expr)

        test_parens('(1)', self.one)
        test_parens('''(

                        1

                           +

                        foo


                    )''',
                    self.one_plus_foo)

        # Missing expression
        with self.parse('()'):
            self.assertError(token=')')

        # Missing right paren
        with self.parse('(1 + 2 3'):
            self.assertExpected(')', got='3')

    def test_units_expr(self):
        def test_units(expr, colno, target, unit):
            with self.parse(expr) as p:
                self.assertIsInstance(p, ast.UnitsExpr)
                self.assertLocation(p, 1, colno)
                self.assertEqual(target, p.target)
                self.assertIsInstance(p.unit, str)
                self.assertEqual(unit, p.unit)

        # Basics
        test_units('1us', 2, self.one, 'us')
        test_units('2 ms', 3, self.two, 'ms')
        test_units('3  s', 4, self.three, 's')

        # Uppercase and mixed-case units
        test_units('1US', 2, self.one, 'US')
        test_units('2 mS', 3, self.two, 'mS')
        test_units('2 Ms', 3, self.two, 'Ms')
        test_units('3  S', 4, self.three, 'S')

        # Invalid unit
        with self.parse('1cm'):
            self.assertError(token='cm')

    def test_subscript_expr(self):
        def test_sub(expr, target, value, colno):
            with self.parse(expr) as p:
                self.assertIsInstance(p, ast.SubscriptExpr)
                self.assertLocation(p, 1, colno)
                self.assertEqual(target, p.target)
                self.assertEqual(value, p.value)

        test_sub('foo[1]', self.foo, self.one, 4)
        test_sub('1[foo]', self.one, self.foo, 2)
        test_sub('foo[1][2]',
                 ast.SubscriptExpr(target=self.foo, value=self.one),
                 self.two,
                 7)
        test_sub('''bar[

                     1

                       +


                       foo


                 ]''',
                 self.bar,
                 self.one_plus_foo,
                 4)

        # Missing value
        with self.parse('foo[]'):
            self.assertError(token=']')

        # Extra value
        with self.parse('foo[1,2]'):
            self.assertExpected(']', got=',')

        # Missing right bracket
        with self.parse('[foo[1, bar]'):
            self.assertExpected(']', got=',')

    def test_unary_expr(self):
        def test_unop(expr, op, operand):
            with self.parse(expr) as p:
                self.assertIsInstance(p, ast.UnaryOpExpr)
                self.assertLocation(p, 1, 1)
                self.assertEqual(op, p.op)
                self.assertEqual(operand, p.operand)

        test_unop('+foo', '+', self.foo)
        test_unop('-bar', '-', self.bar)
        test_unop('!foo', '!', self.foo)
        test_unop('not bar', 'not', self.bar)

        # Uppercase and mixed-case "not"
        test_unop('NOT bar', 'NOT', self.bar)
        test_unop('nOt bar', 'nOt', self.bar)

        # Operator chaining
        with self.parse('+-foo'):
            self.assertError(token='-')

    def test_cast_expr(self):
        def test_cast(expr, totype, target):
            with self.parse(expr) as p:
                self.assertIsInstance(p, ast.CastExpr)
                self.assertLocation(p, 1, 1)
                self.assertEqual(target, p.target)
                self.assertIsInstance(p.totype, str)
                self.assertEqual(totype, p.totype)

        test_cast('(bool)1', 'bool', self.one)

        test_cast('(char)2', 'char', self.two)
        test_cast('(short)2', 'short', self.two)
        test_cast('(int)2', 'int', self.two)
        test_cast('(integer)2', 'integer', self.two)
        test_cast('(long)2', 'long', self.two)

        test_cast('(byte)3', 'byte', self.three)
        test_cast('(word)3', 'word', self.three)
        test_cast('(dword)3', 'dword', self.three)
        test_cast('(qword)3', 'qword', self.three)

        test_cast('(float)foo', 'float', self.foo)
        test_cast('(double)foo', 'double', self.foo)

        test_cast('(string)bar', 'string', self.bar)

        # Wrong case
        with self.parse('(BOOL)1'):
            self.assertError(token='1')

        # Invalid type name
        with self.parse('(unsigned)2'):
            self.assertError(token='2')

        # Missing right paren
        with self.parse('(bool 1'):
            self.assertExpected(')', got='1')

        # Embedded newlines
        with self.parse('(\nbool)1'):
            self.assertError(token='1')

    def _test_binary_op(self, op, *sibling_ops):
        def test_binop(expr, operands, colno):
            with self.parse(expr) as p:
                self.assertIsInstance(p, ast.BinaryOpExpr)
                self.assertLocation(p, 1, colno)
                self.assertEqual(op, p.op)
                assert len(operands) == 2
                self.assertEqual(operands, p.operands)

        test_binop('1 %s 2' % op, (self.one, self.two), 3)

        for other_op in (op,) + sibling_ops:
            test_binop(
                '1 %s 2 %s 3' % (other_op, op),
                (ast.BinaryOpExpr(op = other_op,
                                  operands = (self.one, self.two)),
                 self.three),
                6 + len(other_op),
                )

    def test_multiplicative_expr(self):
        self._test_binary_op('*', '/', '%')
        self._test_binary_op('/', '*', '%')
        self._test_binary_op('%', '*', '/')

    def test_additive_expr(self):
        self._test_binary_op('+', '-')
        self._test_binary_op('-', '+')

    def test_comparison_expr(self):
        ops = ['==',
               '!=',
               '<=', '=<', '#LE',
               '<', '#LT',
               '>=', '=>', '#GE',
               '>', '#GT']

        for _ in range(len(ops)):
            self._test_binary_op(ops[0], *(ops[1:]))
            ops.append(ops.pop(0))

        # Single '=' is *not* supported
        with self.parse('foo = bar'):
            self.assertError(token='=')

    def test_and_expr(self):
        self._test_binary_op('and', '&&', '#AND')
        self._test_binary_op('&&', 'and', '#AND')
        self._test_binary_op('#AND', 'and', '&&')

        # Uppercase and mixed-case "and"
        self._test_binary_op('and', 'AND', 'And', 'aNd')

    def test_or_expr(self):
        self._test_binary_op('or', '||', '#OR')
        self._test_binary_op('||', 'or', '#OR')
        self._test_binary_op('#OR', 'or', '||')

        # Uppercase and mixed-case "or"
        self._test_binary_op('or', 'OR', 'Or', 'oR')

    def test_operator_precedence(self):
        x = ast.IdentifierExpr(value='x')
        with self.parse('''\
                        [x:x or x and x < x + x * (bool) -x ms [x]]\
                        ''') as p:
            # Outer []
            self.assertIsInstance(p, ast.ListLiteralExpr)
            self.assertEqual(1, len(p.items))
            p = p.items[0]

            # :
            self.assertIsInstance(p, ast.RangeExpr)
            self.assertEqual(x, p.start)
            p = p.stop

            # or
            self.assertIsInstance(p, ast.BinaryOpExpr)
            self.assertEqual('or', p.op)
            self.assertEqual(x, p.operands[0])
            p = p.operands[1]

            # and
            self.assertIsInstance(p, ast.BinaryOpExpr)
            self.assertEqual('and', p.op)
            self.assertEqual(x, p.operands[0])
            p = p.operands[1]

            # <
            self.assertIsInstance(p, ast.BinaryOpExpr)
            self.assertEqual('<', p.op)
            self.assertEqual(x, p.operands[0])
            p = p.operands[1]

            # +
            self.assertIsInstance(p, ast.BinaryOpExpr)
            self.assertEqual('+', p.op)
            self.assertEqual(x, p.operands[0])
            p = p.operands[1]

            # *
            self.assertIsInstance(p, ast.BinaryOpExpr)
            self.assertEqual('*', p.op)
            self.assertEqual(x, p.operands[0])
            p = p.operands[1]

            # (bool)
            self.assertIsInstance(p, ast.CastExpr)
            self.assertEqual('bool', p.totype)
            p = p.target

            # -
            self.assertIsInstance(p, ast.UnaryOpExpr)
            self.assertEqual('-', p.op)
            p = p.operand

            # Inner []
            self.assertIsInstance(p, ast.SubscriptExpr)
            self.assertEqual(x, p.value)
            p = p.target

            # ms
            self.assertIsInstance(p, ast.UnitsExpr)
            self.assertEqual('ms', p.unit)
            self.assertEqual(x, p.target)


class TestStatements(ParserTestMixin, unittest.TestCase):

    def test_module(self):
        def test_mod(src, *statements):
            with self.parse(src) as p:
                self.assertIsInstance(p, ast.Module)
                self.assertLocation(p, 0, 0)
                self.assertIsInstance(p.filename, str)
                self.assertEqual('', p.filename)
                self.assertIsInstance(p.statements, tuple)
                self.assertEqual(statements, p.statements)

        # Empty
        test_mod('')
        test_mod('// This module is empty\n   \n')

        # Single statement
        test_mod('''
                 var foo = bar
                 ''',
                 ast.DeclarationStmt(type='var', tag='foo', value=self.bar),
                 )

        # Multiple statements
        test_mod('''
                 // This is a nice module
                 var foo = bar
                 x = 2
                 // One more statement
                 foo bar ()
                 ''',
                 ast.DeclarationStmt(type='var', tag='foo', value=self.bar),
                 ast.AssignmentStmt(varname='x', value=self.two),
                 ast.DeclarationStmt(type='foo', tag='bar'),
                 )

        # Missing newline between statements
        with self.parse('''
                        x = 1 y = 2
                        '''):
            self.assertError('Missing newline at end of statement',
                             token='y')

    def test_assignment_stmt(self):
        def test_assign(src, colno, varname, value):
            with self.parse(src) as p:
                self.assertIsInstance(p, ast.Module)
                self.assertEqual(1, len(p.statements))
                p = p.statements[0]

                self.assertIsInstance(p, ast.AssignmentStmt)
                self.assertLocation(p, 1, colno)
                self.assertEqual(varname, p.varname)
                self.assertEqual((), p.indices)
                self.assertIsNone(p.op)
                self.assertEqual(value, p.value)

        test_assign('foo = 1', 5, 'foo', self.one)
        test_assign('x = bar', 3, 'x', self.bar)

        # Missing value
        with self.parse('''
                        foo =
                        bar = 2
                        '''):
            self.assertError('Line ended unexpectedly', token='\n', lineno=2)

    def test_augmented_assignment_stmt(self):
        def test_assign(op):
            with self.parse('foo %s= 2' % op) as p:
                self.assertIsInstance(p, ast.Module)
                self.assertEqual(1, len(p.statements))
                p = p.statements[0]

                self.assertIsInstance(p, ast.AssignmentStmt)
                self.assertLocation(p, 1, 5)
                self.assertEqual('foo', p.varname)
                self.assertEqual((), p.indices)
                self.assertEqual(op, p.op)
                self.assertEqual(self.two, p.value)

        test_assign('+')
        test_assign('-')
        test_assign('*')
        test_assign('/')
        test_assign('%')

        # Missing value
        with self.parse('''
                        foo +=
                        bar -= 2
                        '''):
            self.assertError('Line ended unexpectedly', token='\n', lineno=2)

    def test_index_assignment_stmt(self):
        def test_assign(src, colno, varname, value, *indices):
            with self.parse(src) as p:
                self.assertIsInstance(p, ast.Module)
                self.assertEqual(1, len(p.statements))
                p = p.statements[0]

                self.assertIsInstance(p, ast.AssignmentStmt)
                self.assertLocation(p, 2, colno)
                self.assertEqual(varname, p.varname)
                self.assertIsInstance(p.indices, tuple)
                self.assertEqual(indices, p.indices)
                self.assertEqual(value, p.value)

                return p.op

        # Single index, simple assignment
        op = test_assign('''
                         foo[1] = 2
                         ''',
                         33, 'foo', self.two, self.one)
        self.assertIsNone(op)

        # Multiple indices, augmented assignment
        op = test_assign('''
                         x[bar][1+foo] *= 3
                         ''',
                         40, 'x', self.three, self.bar, self.one_plus_foo)
        self.assertEqual('*', op)

        # Missing index expression
        with self.parse('a[] = 1'):
            self.assertError(token=']')

        # Missing right bracket
        with self.parse('a[1 = 2'):
            self.assertExpected(']', got='=')

        # Missing =
        with self.parse('a[1]  2'):
            self.assertExpected('assignment operator', got='2')

        # Missing value
        with self.parse('''
                        a[1] =
                        b[2] = 3
                        '''):
            self.assertError('Line ended unexpectedly', token='\n', lineno=2)

    def test_declaration_stmt(self):
        def test_decl(src,
                      typename,
                      tagname = None,
                      value = None,
                      params = (),
                      children = ()):
            with self.parse(src) as p:
                self.assertIsInstance(p, ast.Module)
                self.assertEqual(1, len(p.statements))
                p = p.statements[0]

                self.assertIsInstance(p, ast.DeclarationStmt)
                self.assertLocation(p, 1, 1)
                self.assertEqual(typename, p.type)
                self.assertEqual(tagname, p.tag)
                self.assertEqual(value, p.value)
                self.assertIsInstance(p.params, (tuple, dict))
                self.assertEqual(params, p.params)
                self.assertIsInstance(p.children, tuple)
                self.assertEqual(children, p.children)

        #
        # Basics
        #

        test_decl('foo ()', 'foo')
        test_decl('foo ( \n  \n\n  \n )', 'foo')
        test_decl('foo/bar blah ()', 'foo/bar', 'blah')
        test_decl('foo (bar)', 'foo', params=(self.bar,))
        test_decl('foo ( \n \n 1 \n + \n \n foo \n )',
                  'foo',
                  params = (self.one_plus_foo,),
                  )
        test_decl('foo/bar (p1 = 1)', 'foo/bar', params = {'p1': self.one})
        test_decl('foo/bar (p1 = 1; p2 = 2; p3 = 3)',
                  'foo/bar',
                  params = {'p1': self.one, 'p2': self.two, 'p3': self.three},
                  )
        test_decl('''foo 'blah blah blah' (

                         p1 = 1

                         p2 = 2


                     )
                  ''',
                  'foo',
                  'blah blah blah',
                  params = {'p1': self.one, 'p2': self.two})

        #
        # Expression lists
        #

        foo_bar = ast.ExprList(items=(self.foo, self.bar))
        one_two_three = ast.ExprList(items=(self.one, self.two, self.three))
        foo_to_bar = ast.RangeExpr(start=self.foo, stop=self.bar)

        test_decl('foo "bar" (foo, bar)', 'foo', 'bar', params=(foo_bar,))
        test_decl('''foo/bar (
                         abc = 1,2,3
                         xyz = foo:bar
                     )
                  ''',
                  'foo/bar',
                  params = {'abc': one_two_three, 'xyz': foo_to_bar})

        #
        # Children
        #

        # Empty child list
        test_decl('foo () {}', 'foo')
        test_decl('foo {}', 'foo')

        # One child
        test_decl('''foo/bar (p1 = 1) {
                         x = 2
                     }
                  ''',
                  'foo/bar',
                  params = {'p1': self.one},
                  children = (ast.AssignmentStmt(varname='x', value=self.two),),
                  )

        # Multiple children
        test_decl('''foo bar (p1 = 1; p2 = 2) { var foo = bar
                         x = 2
                         foo bar () {}
                     }
                  ''',
                  'foo',
                  'bar',
                  params = {'p1': self.one, 'p2': self.two},
                  children = (
                      ast.DeclarationStmt(type = 'var',
                                          tag = 'foo',
                                          value = self.bar),
                      ast.AssignmentStmt(varname='x', value=self.two),
                      ast.DeclarationStmt(type='foo', tag='bar'),
                      ),
                  )

        #
        # Value
        #

        # No params or children
        test_decl('foo bar = 1', 'foo', 'bar', value=self.one)

        # Params
        test_decl('foo bar = 1 (p1 = 1)',
                  'foo',
                  'bar',
                  value = self.one,
                  params = {'p1': self.one})

        # Children
        test_decl('''foo/bar blah = 2 {
                         x = 2
                     }
                  ''',
                  'foo/bar',
                  'blah',
                  value = self.two,
                  children = (ast.AssignmentStmt(varname='x', value=self.two),),
                  )

        # Params and children
        test_decl('''foo/bar blah = 2 (p1 = 1) {
                         x = 2
                     }
                  ''',
                  'foo/bar',
                  'blah',
                  value = self.two,
                  params = {'p1': self.one},
                  children = (ast.AssignmentStmt(varname='x', value=self.two),),
                  )

        #
        # Errors
        #

        # Bad sub-type
        with self.parse('foo/1 ()'):
            self.assertExpected('identifier', got='1')

        # Extra /
        with self.parse('''
                        foo/bar/blah ()
                        '''):
            self.assertExpected("'(' or '{'", got='/', colno=32)

        # No parentheses
        with self.parse('''
                        foo blah
                        bar abc ()
                        '''):
            self.assertExpected("'(' or '{'", lineno=2, colno=33)

        # Missing right paren
        with self.parse('''
                        foo blah (
                        bar abc ()
                        '''):
            self.assertExpected(')', got='abc')

        # Bad param name
        with self.parse('''
                        foo (
                            a = 1
                            'b' = 2
                        )
                        '''):
            self.assertExpected('identifier', got="'b'")

        # Missing =
        with self.parse('foo (a 1)'):
            self.assertExpected(')', got='1')

        # Missing value
        with self.parse('''
                        foo (
                            a =
                            b = 2
                        )
                        '''):
            self.assertError('Line ended unexpectedly', token='\n', lineno=3)

        # Missing newline or semicolon
        with self.parse('foo (a = 1 b = 2)'):
            self.assertExpected("line ending or ';'", got='b')

        # Extra semicolon
        with self.parse('foo (a = 1; b = 2;)'):
            self.assertExpected('identifier', got=')')

        # Missing right brace
        with self.parse('''
                        foo () {
                        x = 1
                        y = 2
                        bar ()
                        '''):
            self.assertError('Input ended unexpectedly')

        # Value without tag
        with self.parse('foo = 1 (b = 2)'):
            self.assertError('Missing newline at end of statement',
                             token='(')


class TestIncludes(ParserTestMixin, TempFilesMixin, unittest.TestCase):

    def test_syntax_errors(self):
        # Wrong case
        with self.parse('%INCLUDE foo_bar'):
            self.assertError(token='%')

        # Missing target
        with self.parse('''
                        %include
                        %include foo
                        '''):
            self.assertExpected('identifier or string', lineno=2)

        # Bad target
        with self.parse('%include 123'):
            self.assertExpected('identifier or string', got='123')

        # Not at top level
        with self.parse('''
                        foo {
                            %include bar
                        }
                        '''):
            self.assertError('Include statements are permitted at the top '
                             'level only',
                             lineno = 3,
                             colno = 30)

    def test_basic(self):
        basedir = os.path.join(self.tmpdir, 'base')
        os.mkdir(basedir)
        included_files = collections.OrderedDict()

        blah_src = '''\
foo = 1
blah ()
'''
        blah_path = self.write_file('base/blah.mwel', blah_src)

        something_src = '''\
baz {
    foo = 3
}
'''
        something_path = self.write_file('something.mwel', something_src)

        syntax_error_src = '''\
bar = = 1
'''
        syntax_error_path = self.write_file('syntax_error.foo',
                                            syntax_error_src)

        with self.parse('''\
var foo = bar
%%include blah
bar = 2
%%include "../something.mwel"
foo bar () {}
%%include not_an_include
bar = foo
%%include '%s/syntax_error.foo'
bar = 1
%%include 'blah.mwel'  // Repeated include is OK
''' % self.tmpdir, basedir, included_files) as p:
            self.assertIsInstance(p, ast.Module)
            self.assertEqual(7, len(p.statements))

            self.assertEqual(ast.DeclarationStmt(type = 'var',
                                                 tag = 'foo',
                                                 value = self.bar),
                             p.statements[0])

            blah = p.statements[1]
            self.assertIsInstance(blah, ast.Module)
            self.assertLocation(blah, 2, 2)
            self.assertEqual(blah_path, blah.filename)
            self.assertEqual(2, len(blah.statements))
            self.assertEqual(ast.AssignmentStmt(varname='foo', value=self.one),
                             blah.statements[0])
            self.assertEqual(ast.DeclarationStmt(type='blah'),
                             blah.statements[1])

            self.assertEqual(ast.AssignmentStmt(varname='bar', value=self.two),
                             p.statements[2])

            something = p.statements[3]
            self.assertIsInstance(something, ast.Module)
            self.assertLocation(something, 4, 2)
            self.assertEqual(something_path, something.filename)
            self.assertEqual(1, len(something.statements))
            self.assertEqual(ast.DeclarationStmt(
                type = 'baz',
                children = (ast.AssignmentStmt(varname='foo',
                                               value=self.three),)
                ),
                             something.statements[0])

            self.assertEqual(ast.DeclarationStmt(type='foo', tag='bar'),
                             p.statements[4])

            self.assertError(("Failed to open file '%s': "
                              "No such file or directory" %
                              os.path.join(basedir, 'not_an_include.mwel')),
                             lineno = 6,
                             colno = 2)

            self.assertEqual(ast.AssignmentStmt(varname='bar', value=self.foo),
                             p.statements[5])

            self.assertError(token = '=',
                             lineno = 1,
                             colno = 7,
                             filename = os.path.join(self.tmpdir,
                                                     'syntax_error.foo'))

            self.assertEqual(ast.AssignmentStmt(varname='bar', value=self.one),
                             p.statements[6])

            self.assertEqual(4, len(included_files))
            included_files = list(included_files.items())
            self.assertEqual(blah_path, included_files[0][0])
            self.assertEqual(blah_src, included_files[0][1])
            self.assertEqual(something_path, included_files[1][0])
            self.assertEqual(something_src, included_files[1][1])
            self.assertEqual(os.path.join(basedir, 'not_an_include.mwel'),
                             included_files[2][0])
            self.assertIsNone(included_files[2][1])
            self.assertEqual(syntax_error_path, included_files[3][0])
            self.assertEqual(syntax_error_src, included_files[3][1])

    def test_nested(self):
        os.makedirs(os.path.join(self.tmpdir, 'dir1/dir2'))
        os.makedirs(os.path.join(self.tmpdir, 'dir3/dir4'))
        included_files = collections.OrderedDict()

        include1_src = '''\
foo = 2
%include 'dir2/include2'
foo = 2
'''
        include1_path = self.write_file('dir1/include1.mwel', include1_src)

        include2_src = '''\
foo = 3
%include '../../dir3/dir4/include3'
foo = 3
'''
        include2_path = self.write_file('dir1/dir2/include2.mwel', include2_src)

        include3_src = '''\
bar = 1
%include include3  // Recursive include is OK
'''
        include3_path = self.write_file('dir3/dir4/include3.mwel', include3_src)

        with self.parse('''\
foo = 1
%include 'dir1/include1'
foo = 1
''', self.tmpdir, included_files) as p:
            self.assertIsInstance(p, ast.Module)
            self.assertEqual(3, len(p.statements))

            self.assertEqual(ast.AssignmentStmt(varname='foo', value=self.one),
                             p.statements[0])
            self.assertEqual(ast.AssignmentStmt(varname='foo', value=self.one),
                             p.statements[2])

            p = p.statements[1]
            self.assertIsInstance(p, ast.Module)
            self.assertEqual(3, len(p.statements))
            self.assertLocation(p, 2, 2)
            self.assertEqual(include1_path, p.filename)

            self.assertEqual(ast.AssignmentStmt(varname='foo', value=self.two),
                             p.statements[0])
            self.assertEqual(ast.AssignmentStmt(varname='foo', value=self.two),
                             p.statements[2])

            p = p.statements[1]
            self.assertIsInstance(p, ast.Module)
            self.assertEqual(3, len(p.statements))
            self.assertLocation(p, 2, 2)
            self.assertEqual(include2_path, p.filename)

            self.assertEqual(ast.AssignmentStmt(varname = 'foo',
                                                value = self.three),
                             p.statements[0])
            self.assertEqual(ast.AssignmentStmt(varname = 'foo',
                                                value = self.three),
                             p.statements[2])

            p = p.statements[1]
            self.assertIsInstance(p, ast.Module)
            self.assertEqual(1, len(p.statements))
            self.assertLocation(p, 2, 2)
            self.assertEqual(include3_path, p.filename)

            self.assertEqual(ast.AssignmentStmt(varname='bar', value=self.one),
                             p.statements[0])

            self.assertEqual(3, len(included_files))
            included_files = list(included_files.items())
            self.assertEqual(include1_path, included_files[0][0])
            self.assertEqual(include1_src, included_files[0][1])
            self.assertEqual(include2_path, included_files[1][0])
            self.assertEqual(include2_src, included_files[1][1])
            self.assertEqual(include3_path, included_files[2][0])
            self.assertEqual(include3_src, included_files[2][1])

    def test_nested_error(self):
        os.makedirs(os.path.join(self.tmpdir, 'dir1/dir2'))
        os.makedirs(os.path.join(self.tmpdir, 'dir3/dir4'))

        include1_src = '''\
%include 'dir2/include2'
foo = 2
'''
        self.write_file('dir1/include1.mwel', include1_src)

        include2_src = '''\
%include '../../dir3/dir4/include3'
foo = 3
'''
        self.write_file('dir1/dir2/include2.mwel', include2_src)

        include3_src = '''\
bar = = 1
'''
        self.write_file('dir3/dir4/include3.mwel', include3_src)

        with self.parse('''\
%include 'dir1/include1'
foo = 1
''', self.tmpdir):
            self.assertError(token = '=',
                             lineno = 1,
                             colno = 7,
                             filename = os.path.join(self.tmpdir,
                                                     'dir3/dir4/include3.mwel'))


class TestMacros(ParserTestMixin, unittest.TestCase):

    def test_syntax_errors(self):
        # Wrong case
        with self.parse('%DEFINE x = 1'):
            self.assertError(token='%')

        # Missing name
        with self.parse('''
                        %define
                        x = y
                        '''):
            self.assertExpected('identifier', lineno=2)

        # Bad name
        with self.parse('%define 3 = 4'):
            self.assertExpected('identifier', got='3')

        # Not at top level
        with self.parse('''
                        foo {
                            %define x = 1
                        }
                        '''):
            self.assertError('Macro definitions are permitted at the top '
                             'level only',
                             lineno = 3,
                             colno = 30)

    def test_no_expression(self):
        with self.parse('''
                        %define abc
                        %define xyz = true
                        ''') as p:
            self.assertIsInstance(p, ast.Module)
            self.assertEqual(2, len(p.statements))
            p = p.statements

            self.assertIsInstance(p[0], ast.ExpressionMacroStmt)
            self.assertLocation(p[0], 2, 26)
            self.assertEqual('abc', p[0].name)
            self.assertEqual((), p[0].parameters)
            self.assertEqual(self.true, p[0].value)

            self.assertIsInstance(p[1], ast.ExpressionMacroStmt)
            self.assertLocation(p[1], 3, 26)
            self.assertEqual('xyz', p[1].name)
            self.assertEqual((), p[1].parameters)
            self.assertEqual(self.true, p[1].value)

    def test_simple_expression(self):
        # Missing value
        with self.parse('''
                        %define x =
                        x = y
                        '''):
            self.assertError('Line ended unexpectedly', token='\n', lineno=2)

        # Success
        with self.parse('''
                        %define abc = 3
                        %define xyz = 1 + foo
                        ''') as p:
            self.assertIsInstance(p, ast.Module)
            self.assertEqual(2, len(p.statements))
            p = p.statements

            self.assertIsInstance(p[0], ast.ExpressionMacroStmt)
            self.assertLocation(p[0], 2, 26)
            self.assertEqual('abc', p[0].name)
            self.assertEqual((), p[0].parameters)
            self.assertEqual(self.three, p[0].value)

            self.assertIsInstance(p[1], ast.ExpressionMacroStmt)
            self.assertLocation(p[1], 3, 26)
            self.assertEqual('xyz', p[1].name)
            self.assertEqual((), p[1].parameters)
            self.assertEqual(self.one_plus_foo, p[1].value)

    def test_parametrized_expression(self):
        # Bad parameter
        with self.parse('''
                        %define x(1)
                        x = y
                        '''):
            self.assertExpected('identifier', got='1', lineno=2)

        # Missing comma
        with self.parse('''
                        %define x(a b)
                        x = y
                        '''):
            self.assertExpected("')'", got='b', lineno=2)

        # Missing parameter
        with self.parse('''
                        %define x(a,)
                        x = y
                        '''):
            self.assertExpected('identifier', got=')', lineno=2)

        # Duplicate parameter
        with self.parse('''
                        %define x(a, b, a)
                        x = y
                        '''):
            self.assertError("Macro definition has multiple parameters named "
                             "'a'",
                             lineno = 2,
                             colno = 41)

        # Missing ')'
        with self.parse('''
                        %define x(a, b
                        x = y
                        '''):
            self.assertExpected(')', got='x', lineno=3)

        # Missing value
        with self.parse('''
                        %define x(a, b)
                        x = y
                        '''):
            self.assertError('Input ended unexpectedly')

        # Success
        with self.parse('''
                        %define f1() 1
                        %define f2(x) 2
                        %define f3(x,y) 3
                        %define f4(x,y,
                                   z
                                   ) 1 + foo
                        %define f5(
                                   ) 1
                        ''') as p:
            self.assertIsInstance(p, ast.Module)
            self.assertEqual(5, len(p.statements))
            p = p.statements

            def test_pe(p, lineno, name, params, value):
                self.assertIsInstance(p, ast.ExpressionMacroStmt)
                self.assertLocation(p, lineno, 26)
                self.assertEqual(name, p.name)
                self.assertEqual(params, p.parameters)
                self.assertEqual(value, p.value)

            test_pe(p[0], 2, 'f1', (), self.one)
            test_pe(p[1], 3, 'f2', ('x',), self.two)
            test_pe(p[2], 4, 'f3', ('x', 'y'), self.three)
            test_pe(p[3], 5, 'f4', ('x', 'y', 'z'), self.one_plus_foo)
            test_pe(p[4], 8, 'f5', (), self.one)

    def test_statement(self):
        # Missing '%end'
        with self.parse('''
                        %define x(a)
                            y = a
                        '''):
            self.assertError('Input ended unexpectedly')

        # Success
        with self.parse('''
                        %define f1()
                        %end

                        %define f2(foo)
                           x = foo
                        %end

                        %define f3(foo, bar)
                           x = foo
                           y = bar
                        %end
                        ''') as p:
            self.assertIsInstance(p, ast.Module)
            self.assertEqual(3, len(p.statements))
            p = p.statements

            self.assertIsInstance(p[0], ast.StatementMacroStmt)
            self.assertLocation(p[0], 2, 26)
            self.assertEqual('f1', p[0].name)
            self.assertEqual((), p[0].parameters)
            self.assertEqual((), p[0].statements)

            self.assertIsInstance(p[1], ast.StatementMacroStmt)
            self.assertLocation(p[1], 5, 26)
            self.assertEqual('f2', p[1].name)
            self.assertEqual(('foo',), p[1].parameters)
            self.assertEqual((ast.AssignmentStmt(varname = 'x',
                                                 value = self.foo),),
                             p[1].statements)

            self.assertIsInstance(p[2], ast.StatementMacroStmt)
            self.assertLocation(p[2], 9, 26)
            self.assertEqual('f3', p[2].name)
            self.assertEqual(('foo', 'bar'), p[2].parameters)
            self.assertEqual((ast.AssignmentStmt(varname = 'x',
                                                 value = self.foo),
                              ast.AssignmentStmt(varname = 'y',
                                                 value = self.bar)),
                             p[2].statements)

    def test_require(self):
        # Wrong case
        with self.parse('%REQUIRE x'):
            self.assertError(token='%')

        # Missing name
        with self.parse('''
                        %require
                        x = y
                        '''):
            self.assertExpected('identifier', lineno=2)

        # Bad name
        with self.parse('%require 3'):
            self.assertExpected('identifier', got='3')

        # Extra comma
        with self.parse('''
                        %require a, b,
                        '''):
            self.assertExpected('identifier', lineno=2)

        # Not at top level
        with self.parse('''
                        foo {
                            %require x
                        }
                        '''):
            self.assertError('Require statements are permitted at the top '
                             'level only',
                             lineno = 3,
                             colno = 30)

        # Success
        with self.parse('''
                        %require abc
                        %require x,y, z
                        ''') as p:
            self.assertIsInstance(p, ast.Module)
            self.assertEqual(2, len(p.statements))
            p = p.statements

            self.assertIsInstance(p[0], ast.RequireStmt)
            self.assertLocation(p[0], 2, 26)
            self.assertEqual(('abc',), p[0].names)

            self.assertIsInstance(p[1], ast.RequireStmt)
            self.assertLocation(p[1], 3, 26)
            self.assertEqual(('x', 'y', 'z'), p[1].names)
