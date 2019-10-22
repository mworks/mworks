from __future__ import division, print_function, unicode_literals
import unittest

from mwel.decompiler.xmlparser import XMLParser

from . import ElementCheckMixin
from .. import ErrorLoggerMixin


class TestXMLParser(ElementCheckMixin, ErrorLoggerMixin, unittest.TestCase):

    def setUp(self):
        super(TestXMLParser, self).setUp()

        xmlparser = XMLParser(self.error_logger)
        self.parse = self.with_error_check(xmlparser.parse)

    def test_bad_xml(self):
        src = b'''\
<something>
   <
</something>'''

        with self.parse(src) as root:
            self.assertError('Failed to parse XML: not well-formed '
                             '(invalid token)',
                             lineno = 2,
                             colno = 4)
            self.assertIsNone(root)

    def test_not_monkeyml(self):
        src = b'''\
<donkeyml>
</donkeyml>'''

        with self.parse(src) as root:
            self.assertError('XML does not contain an MWorks experiment '
                             '(root element is donkeyml, not monkeyml)')
            self.assertIsNone(root)

    def test_success(self):
        src = b'''\
<?xml version="1.0"?>
   <?mworks mwpp="/bin/bash"?>
<monkeyml version="1.0">
    <experiment tag="New Experiment" full_name="" description="">
        <protocol tag="New Protocol" nsamples="1" sampling_method="cycles" selection="sequential" interruptible="YES">
            <action type="assignment" tag="Assign Variable" variable="x" value="1"></action>
            <action tag="Update Stimulus Display" type="update_stimulus_display" predicted_output_time=""></action>
            <!-- This is a comment -->  This is unhandled text
        </protocol>
    </experiment>
</monkeyml>'''

        with self.parse(src) as root:
            self.assertError('XML contains unexpected data',
                             lineno = 2,
                             colno = 3)

            self.assertElement(root,
                               'monkeyml',
                               [('version', '1.0')],
                               num_children = 1,
                               lineno = 3,
                               colno = 0)
            root = root.children[0]

            self.assertElement(root,
                               'experiment',
                               [('tag', 'New Experiment'),
                                ('full_name', ''),
                                ('description', '')],
                               num_children = 1,
                               lineno = 4,
                               colno = 4)
            root = root.children[0]

            self.assertElement(root,
                               'protocol',
                               [('tag', 'New Protocol'),
                                ('nsamples', '1'),
                                ('sampling_method', 'cycles'),
                                ('selection', 'sequential'),
                                ('interruptible', 'YES')],
                               num_children = 3,
                               lineno = 5,
                               colno = 8)
            elems = root.children

            self.assertElement(elems[0],
                               'action',
                               [('type', 'assignment'),
                                ('tag', 'Assign Variable'),
                                ('variable', 'x'),
                                ('value', '1')],
                               lineno = 6,
                               colno = 12)

            self.assertElement(elems[1],
                               'action',
                               [('tag', 'Update Stimulus Display'),
                                ('type', 'update_stimulus_display'),
                                ('predicted_output_time', '')],
                               lineno = 7,
                               colno = 12)

            self.assertComment(elems[2],
                               ' This is a comment ',
                               lineno = 8,
                               colno = 12)

            self.assertError('XML contains unexpected data',
                             lineno = 8,
                             colno = 38)
