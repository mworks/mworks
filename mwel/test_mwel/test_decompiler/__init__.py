from __future__ import division, print_function, unicode_literals
from collections import OrderedDict

from mwel.decompiler.converter import Comment, Assignment, Declaration
from mwel.decompiler.xmlparser import XMLElement


class ElementCheckMixin(object):

    def assertElement(self, e, tag, attrib=[], text=None, num_children=0,
                      lineno=None, colno=None):
        self.assertIsInstance(e, XMLElement)
        self.assertEqual(tag, e.tag)
        self.assertIsInstance(e.attrib, OrderedDict)
        self.assertEqual(OrderedDict(attrib), e.attrib)
        self.assertEqual(text, e.text)
        self.assertIsNone(e.tail)
        self.assertEqual(num_children, len(e.children))
        if lineno is not None:
            self.assertEqual(lineno, e.lineno)
        if colno is not None:
            self.assertEqual(colno, e.colno)

    def assertComment(self, e, text, lineno=None, colno=None):
        self.assertElement(e,
                           XMLElement.comment_tag,
                           text = text,
                           lineno = lineno,
                           colno = colno)


class DeclarationCheckMixin(object):

    def assertDeclaration(self, d, signature, tag=None, value=None, params=[],
                          num_children=0):
        self.assertIsInstance(d, Declaration)
        self.assertEqual(signature, d.signature)
        self.assertEqual(tag, d.tag)
        self.assertEqual(value, d.value)
        if isinstance(params, list):
            self.assertIsInstance(d.params, OrderedDict)
            self.assertEqual(OrderedDict(params), d.params)
        else:
            self.assertIsInstance(d.params, type(''))
            self.assertEqual(params, d.params)
        self.assertEqual(num_children, len(d.children))

    def assertAssignment(self, a, variable, value):
        self.assertIsInstance(a, Assignment)
        self.assertEqual(variable, a.variable)
        self.assertEqual(value, a.value)

    def assertComment(self, c, text):
        self.assertIsInstance(c, Comment)
        self.assertEqual(text, c.text)
