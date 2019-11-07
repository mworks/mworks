from __future__ import division, print_function, unicode_literals
import io
import unittest
import xml.etree.ElementTree as ET

from mwel.analyzer import Analyzer
from mwel.generator import XMLGenerator
from mwel.parser import Parser
from mwel.validator import Validator

from . import ErrorLoggerMixin, TempFilesMixin


class TestXMLGenerator(ErrorLoggerMixin, TempFilesMixin, unittest.TestCase):

    def setUp(self):
        super(TestXMLGenerator, self).setUp()
        self.parser = Parser(self.error_logger)
        self.analyzer = Analyzer(self.error_logger)
        self.validator = Validator(self.error_logger)
        self.generator = XMLGenerator()

    def generate(self, src, omit_metadata=False):
        tree = self.parser.parse(src, self.tmpdir)
        self.assertNoErrors()
        cmpts = self.analyzer.analyze(tree)
        cmpts = self.validator.validate(cmpts)
        self.assertNoErrors()
        return self.generator.generate(cmpts, omit_metadata)

    def test_all(self):
        base_src = '''\
var x = 2

folder 'Other Vars' {
    var foo = 12
    var bar = 'This is a string'
    var blah = [1.5, 2.5, 3.5]
}

%include protocol

experiment 'My Experiment' {}
'''

        protocol_src = '''\
protocol 'Test Protocol' {
    if (x > 1) {
        report ('x is greater than 1!')
    }
    x = 'foo'
    report ('x = $x')
}
'''
        protocol_path = self.write_file('protocol.mwel', protocol_src)

        expected_xml_output = '''\
<?xml version='1.0' encoding='UTF-8'?>
<monkeyml version="1.0">
  <variable _location="line 1, column 1" default_value="2" tag="x" />
  <folder _location="line 3, column 1" tag="Other Vars">
    <variable _location="line 4, column 5" default_value="12" tag="foo" />
    <variable _location="line 5, column 5" default_value="'This is a string'" tag="bar" />
    <variable _location="line 6, column 5" default_value="[1.5, 2.5, 3.5]" tag="blah" />
  </folder>
  <experiment _location="line 11, column 1" tag="My Experiment">
    <protocol _location="protocol.mwel: line 1, column 1" tag="Test Protocol">
      <action _location="protocol.mwel: line 2, column 5" condition="x &gt; 1" type="if">
        <action _location="protocol.mwel: line 3, column 9" message="x is greater than 1!" type="report" />
      </action>
      <action _location="protocol.mwel: line 5, column 7" type="assignment" value="'foo'" variable="x" />
      <action _location="protocol.mwel: line 6, column 5" message="x = $x" type="report" />
    </protocol>
  </experiment>
  <extra>Some extra text</extra>
  <extra>More extra text<child />
    <child />Some stray text</extra>
</monkeyml>
'''

        root = self.generate(base_src)
        self.assertIsInstance(root, ET.Element)

        # Add node with text but no children
        node = ET.SubElement(root, 'extra')
        node.text = 'Some extra text'

        # Add node with text and children
        node = ET.SubElement(root, 'extra')
        node.text = 'More extra text'
        ET.SubElement(node, 'child')

        # Add node with tail
        node = ET.SubElement(node, 'child')
        node.tail = 'Some stray text'

        self.generator.format(root)

        output_stream = io.StringIO()
        self.generator.write(root, output_stream)
        self.assertEqual(expected_xml_output, output_stream.getvalue())

    def test_omit_metadata(self):
        base_src = '''\
var x = 2

folder 'Other Vars' {
    var foo = 12
    var bar = 'This is a string'
    var blah = [1.5, 2.5, 3.5]
}

%include protocol

experiment 'My Experiment' {}
'''

        protocol_src = '''\
protocol 'Test Protocol' {
    if (x > 1) {
        report ('x is greater than 1!')
    }
    x = 'foo'
    report ('x = $x')
}
'''
        protocol_path = self.write_file('protocol.mwel', protocol_src)

        expected_xml_output = '''\
<?xml version='1.0' encoding='UTF-8'?>
<monkeyml version="1.0">
  <variable default_value="2" tag="x" />
  <folder tag="Other Vars">
    <variable default_value="12" tag="foo" />
    <variable default_value="'This is a string'" tag="bar" />
    <variable default_value="[1.5, 2.5, 3.5]" tag="blah" />
  </folder>
  <experiment tag="My Experiment">
    <protocol tag="Test Protocol">
      <action condition="x &gt; 1" type="if">
        <action message="x is greater than 1!" type="report" />
      </action>
      <action type="assignment" value="'foo'" variable="x" />
      <action message="x = $x" type="report" />
    </protocol>
  </experiment>
</monkeyml>
'''

        root = self.generate(base_src, omit_metadata=True)
        self.assertIsInstance(root, ET.Element)

        self.generator.format(root)

        output_stream = io.StringIO()
        self.generator.write(root, output_stream)
        self.assertEqual(expected_xml_output, output_stream.getvalue())
