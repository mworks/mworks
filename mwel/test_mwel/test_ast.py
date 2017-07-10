from __future__ import division, print_function, unicode_literals
import unittest

from mwel import ast


class Node(ast.AST):

    _fields = ('foo', 'bar')


class DerivedNode(Node):
    pass


class OtherNode(ast.AST):
    pass


class NodeWithDefaults(ast.AST):

    _fields = ('a', 'b', 'c')
    _defaults = {'b': 2, 'c': 3}


class TestAST(unittest.TestCase):

    def test_lineno_and_colno(self):
        n = OtherNode()
        self.assertEqual(-1, n.lineno)
        self.assertEqual(-1, n.colno)

        n = OtherNode(lineno=12, colno=34)
        self.assertEqual(12, n.lineno)
        self.assertEqual(34, n.colno)

    def test_missing_field(self):
        with self.assertRaises(KeyError) as cm:
            Node(foo=1)
        self.assertEqual('bar', cm.exception.args[0])

    def test_invalid_field(self):
        with self.assertRaises(TypeError) as cm:
            DerivedNode(foo=1, bar=2, blah=3)
        self.assertEqual("DerivedNode has no field 'blah'",
                         cm.exception.args[0])

    def test_defaults(self):
        # All fields specified
        n = NodeWithDefaults(a=1, b=4, c=5)
        self.assertEqual(1, n.a)
        self.assertEqual(4, n.b)
        self.assertEqual(5, n.c)

        # One field omitted
        n = NodeWithDefaults(a=1, c=5)
        self.assertEqual(1, n.a)
        self.assertEqual(2, n.b)
        self.assertEqual(5, n.c)

        # Two fields omitted
        n = NodeWithDefaults(a=1)
        self.assertEqual(1, n.a)
        self.assertEqual(2, n.b)
        self.assertEqual(3, n.c)

        # All fields omitted
        with self.assertRaises(KeyError) as cm:
            NodeWithDefaults()
        self.assertEqual('a', cm.exception.args[0])

        # Invalid field
        with self.assertRaises(TypeError) as cm:
            NodeWithDefaults(a=1, d=4)
        self.assertEqual("NodeWithDefaults has no field 'd'",
                         cm.exception.args[0])

    def test_repr(self):
        self.assertEqual('Node(foo=1, bar=2)', repr(Node(foo=1, bar=2)))
        self.assertEqual('DerivedNode(foo=3, bar=4)',
                         repr(DerivedNode(foo=3, bar=4)))
        self.assertEqual('OtherNode()', repr(OtherNode()))

    def test_equality(self):
        n1 = Node(foo=1, bar=2)
        n2 = Node(foo=1, bar=2)
        n3 = Node(foo=1, bar=3)

        self.assertTrue(n1 == n1)
        self.assertTrue(n1 == n2)
        self.assertFalse(n1 == n3)

        self.assertFalse(n1 != n1)
        self.assertFalse(n1 != n2)
        self.assertTrue(n1 != n3)

        d = DerivedNode(foo=n1.foo, bar=n1.bar)
        self.assertEqual(n1.foo, d.foo)
        self.assertEqual(n1.bar, d.bar)

        self.assertFalse(n1 == d)
        self.assertFalse(d == n1)

        self.assertTrue(n1 != d)
        self.assertTrue(d != n1)

        o1 = OtherNode()
        o2 = OtherNode()

        self.assertTrue(o1 == o1)
        self.assertTrue(o1 == o2)
        self.assertFalse(o1 == n1)

        self.assertFalse(o1 != o1)
        self.assertFalse(o1 != o2)
        self.assertTrue(o1 != n1)
