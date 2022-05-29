import unittest

from mwel.decompiler.converter import Converter
from mwel.decompiler.simplifier import Simplifier
from mwel.decompiler.xmlparser import XMLParser

from . import DeclarationCheckMixin
from .. import ErrorLoggerMixin


class TestConverter(DeclarationCheckMixin, ErrorLoggerMixin, unittest.TestCase):

    def setUp(self):
        super(TestConverter, self).setUp()

        xmlparser = XMLParser(self.error_logger)
        simplifier = Simplifier(self.error_logger)
        converter = Converter(self.error_logger)

        def convert(src_data):
            root = xmlparser.parse(src_data)
            if root is not None:
                simplifier.simplify(root)
                return converter.convert(root)

        self.convert = self.with_error_check(convert)

    def test_basics(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <protocol tag="new_protocol" nsamples="1" sampling_method="cycles">
        <action tag="Update Stimulus Display" type="update_stimulus_display" predicted_output_time=""></action>
        <action type="report" message="protocol_ending"></action>
        <!-- This is a comment -->
    </protocol>
</monkeyml>'''

        with self.convert(src) as items:
            self.assertEqual(1, len(items))

            self.assertDeclaration(items[0],
                                   'protocol',
                                   'new_protocol',
                                   params = [('nsamples', '1'),
                                             ('sampling_method', 'cycles')],
                                   num_children = 3)
            items = items[0].children

            self.assertDeclaration(items[0], 'action/update_stimulus_display')

            self.assertDeclaration(items[1],
                                   'action/report',
                                   params = [('message', 'protocol_ending')])

            self.assertComment(items[2], ' This is a comment ')

    def test_variable_signature_handling(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <variable tag="a" default_value="1"/>
    <variable tag="b" default_value="2 3" type="float"/>
    <variable type="selection" tag="sel" values="1:10"/>
</monkeyml>'''

        with self.convert(src) as items:
            self.assertEqual(3, len(items))

            # No subtype
            self.assertDeclaration(items[0],
                                   'variable',
                                   'a',
                                   value = '1')

            # Subtype
            self.assertDeclaration(items[1],
                                   'variable',
                                   'b',
                                   params = [('default_value', "'2 3'"),
                                             ('type', 'float')])

            # Selection variable
            self.assertDeclaration(items[2],
                                   'variable/selection',
                                   'sel',
                                   params = [('values', '1:10')])

    def test_variable_type_any_handling(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <variable tag="a" default_value="1"/>
    <variable tag="b" default_value="2" type="any"/>
    <variable tag="c"/>
    <variable tag="d" default_value="4 5"/>
</monkeyml>'''

        with self.convert(src) as items:
            self.assertEqual(4, len(items))

            # No subtype
            self.assertDeclaration(items[0],
                                   'variable',
                                   'a',
                                   value = '1')

            # Subtype "any"
            self.assertDeclaration(items[1],
                                   'variable',
                                   'b',
                                   value = '2')

            # No default value
            self.assertDeclaration(items[2], 'variable', 'c')

            # Default value not a valid expression
            self.assertDeclaration(items[3],
                                   'variable',
                                   'd',
                                   params = [('default_value', "'4 5'")])

    def test_variable_type_string_handling(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <variable tag="x" type="string" default_value="foo bar"/>
    <variable tag="x" type="sTrInG" default_value="'foo'bar'"/>
    <variable tag="x" type="STRING" default_value="'foo&quot;bar'"/>
</monkeyml>'''

        with self.convert(src) as items:
            self.assertEqual(3, len(items))

            # No embedded quotes
            self.assertDeclaration(items[0],
                                   'variable',
                                   'x',
                                   value = "'foo bar'")

            # Embedded single quotes
            self.assertDeclaration(items[1],
                                   'variable',
                                   'x',
                                   value = '"\'foo\'bar\'"')

            # Embedded single and double quotes
            self.assertDeclaration(items[2],
                                   'variable',
                                   'x',
                                   value = "'\\'foo\"bar\\''")

    def test_variable_type_list_handling(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <variable tag="x" type="list" default_value="1"/>
    <variable tag="x" type="LiSt" default_value="2, 3,4 "/>
    <variable tag="x" type="list" default_value="5,6 7"/>
</monkeyml>'''

        with self.convert(src) as items:
            self.assertEqual(3, len(items))

            # Single element
            self.assertDeclaration(items[0],
                                   'variable',
                                   'x',
                                   value = "[1]")

            # Multiple elements
            self.assertDeclaration(items[1],
                                   'variable',
                                   'x',
                                   value = "[2, 3,4 ]")

            # Not a valid expression list
            self.assertDeclaration(items[2],
                                   'variable',
                                   'x',
                                   params = [('type', 'list'),
                                             ('default_value', "'5,6 7'")])

    def test_variable_type_other_handling(self):
        def src(subtype, value):
            return ('''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <variable tag="x" type="%s" default_value="%s"/>
</monkeyml>''' % (subtype, value)).encode('utf-8')

        def test_type(subtypes,
                      casted_to_type,
                      invalid_expr_value,
                      casted_values,
                      uncasted_values):
            for subtype in subtypes:
                with self.convert(src(subtype, invalid_expr_value)) as items:
                    self.assertEqual(1, len(items))
                    self.assertDeclaration(items[0],
                                           'variable',
                                           'x',
                                           params = [('type', subtype),
                                                     ('default_value',
                                                      "'%s'" % invalid_expr_value)])

                for value in casted_values:
                    with self.convert(src(subtype, value)) as items:
                        self.assertEqual(1, len(items))
                        self.assertDeclaration(items[0],
                                               'variable',
                                               'x',
                                               value = ('(%s)(%s)' %
                                                        (casted_to_type, value)))

                for value in uncasted_values:
                    with self.convert(src(subtype, value)) as items:
                        self.assertEqual(1, len(items))
                        self.assertDeclaration(items[0],
                                               'variable',
                                               'x',
                                               value = value)

        # Integer
        test_type(['integer', 'InTeGeR', 'INTEGER'],
                  'int',
                  '1 2',
                  ['1+2', '1.2', '1e2', '1E2'],
                  ['12'])

        # Float
        test_type(['float', 'fLoAt', 'FLOAT', 'double', 'DoUbLe', 'DOUBLE'],
                  'float',
                  '1. 2',
                  ['1.0+2.0', '12'],
                  ['1.2', '1e2', '1E2'])

        # Boolean
        test_type(['boolean', 'bOoLeAn', 'BOOLEAN'],
                  'bool',
                  'tru e',
                  ['true+false', '1.0', '0'],
                  ['TrUE', 'fAlSe', 'YeS', 'nO'])

    def test_variable_param_cleanup(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <variable tag="x" default_value="1" logging="wHeN_cHaNgEd" scope="GlObAl"/>
    <variable tag="x" default_value="1" exclude_from_data_file="0" persistant="0"/>
    <variable tag="x" default_value="1" exclude_from_data_file="No" persistant="No"/>
    <variable tag="x" default_value="1" exclude_from_data_file="fAlSe" persistant="fAlSe"/>
    <variable tag="x" default_value="1" exclude_from_data_file="0.0" persistant="0.0"/>
    <variable tag="x" default_value="1" description="la la la" editable="never" viewable="NO"/>
</monkeyml>'''

        with self.convert(src) as items:
            self.assertEqual(6, len(items))

            # "logging" and "scope" lowercased
            self.assertDeclaration(items[0],
                                   'variable',
                                   'x',
                                   value = '1',
                                   params = [('logging', 'when_changed'),
                                             ('scope', 'global')])

            # Variations of not "exclude_from_data_file" and "persistant" (ugh!)
            self.assertDeclaration(items[1],
                                   'variable',
                                   'x',
                                   value = '1')
            self.assertDeclaration(items[2],
                                   'variable',
                                   'x',
                                   value = '1')
            self.assertDeclaration(items[3],
                                   'variable',
                                   'x',
                                   value = '1')
            self.assertDeclaration(items[4],
                                   'variable',
                                   'x',
                                   value = '1',
                                   params = [('exclude_from_data_file', '0.0'),
                                             ('persistant', '0.0')])

            # "description", "editable", and "viewable" removed
            self.assertDeclaration(items[5],
                                   'variable',
                                   'x',
                                   value = '1')

    def test_subtype_case(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <protocol>
        <action type="update_stimulus_display"/>
        <action type="Update_Stimulus_Display"/>
        <action type="UPDATE_STIMULUS_DISPLAY"/>
    </protocol>
</monkeyml>'''

        with self.convert(src) as items:
            self.assertEqual(1, len(items))

            self.assertDeclaration(items[0], 'protocol', num_children=3)
            items = items[0].children

            self.assertDeclaration(items[0], 'action/update_stimulus_display')
            self.assertDeclaration(items[1], 'action/update_stimulus_display')
            self.assertDeclaration(items[2], 'action/update_stimulus_display')

    def test_tag_quoting(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <protocol/>
    <protocol tag="first_protocol"/>
    <protocol tag="Second Protocol"/>
    <protocol tag="1-2+3"/>
    <protocol tag=" fourth"/>
    <protocol tag="fifth  "/>
    <protocol tag="   "/>
    <protocol tag="sixth'one"/>
    <protocol tag="the&quot;seventh'one'"/>
</monkeyml>'''

        with self.convert(src) as items:
            self.assertEqual(9, len(items))

            # No tag
            self.assertDeclaration(items[0], 'protocol')

            # Valid identifier
            self.assertDeclaration(items[1], 'protocol', 'first_protocol')

            # Embedded spaces
            self.assertDeclaration(items[2], 'protocol', "'Second Protocol'")

            # Non-identifier expression
            self.assertDeclaration(items[3], 'protocol', "'1-2+3'")

            # Leading whitespace
            self.assertDeclaration(items[4], 'protocol', ' fourth')

            # Trailing whitespace
            self.assertDeclaration(items[5], 'protocol', 'fifth  ')

            # Only whitespace
            self.assertDeclaration(items[6], 'protocol', "'   '")

            # Embedded single quote
            self.assertDeclaration(items[7], 'protocol', '"sixth\'one"')

            # Embedded single and double quotes
            self.assertDeclaration(items[8],
                                   'protocol',
                                   "'the\"seventh\\'one\\''")

    def test_parameter_quoting(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <protocol>
        <action type="report" message="the_message"></action>
        <action type="report" message="The message"></action>
        <action type="report" message="(1+2)*3"></action>
        <action type="report" message="1, 2, 3"></action>
        <action type="report" message="0:10:2"></action>
        <action type="report" message=" 1, 2, 3"></action>
        <action type="report" message="1, 2, 3  "></action>
        <action type="report" message="   "></action>
        <action type="report" message="the'message"></action>
        <action type="report" message="one&quot;more'message'"></action>
    </protocol>
</monkeyml>'''

        with self.convert(src) as items:
            self.assertEqual(1, len(items))
            self.assertDeclaration(items[0], 'protocol', num_children=10)
            items = items[0].children

            # Valid identifier
            self.assertDeclaration(items[0],
                                   'action/report',
                                   params = [('message', 'the_message')])

            # Embedded spaces
            self.assertDeclaration(items[1],
                                   'action/report',
                                   params = [('message', "'The message'")])

            # Non-identifier expression
            self.assertDeclaration(items[2],
                                   'action/report',
                                   params = [('message', '(1+2)*3')])

            # Expression list
            self.assertDeclaration(items[3],
                                   'action/report',
                                   params = [('message', '1, 2, 3')])

            # Range expression
            self.assertDeclaration(items[4],
                                   'action/report',
                                   params = [('message', '0:10:2')])

            # Leading whitespace
            self.assertDeclaration(items[5],
                                   'action/report',
                                   params = [('message', ' 1, 2, 3')])

            # Trailing whitespace
            self.assertDeclaration(items[6],
                                   'action/report',
                                   params = [('message', '1, 2, 3  ')])

            # Only whitespace
            self.assertDeclaration(items[7],
                                   'action/report',
                                   params = [('message', "'   '")])

            # Embedded single quote
            self.assertDeclaration(items[8],
                                   'action/report',
                                   params = [('message', '"the\'message"')])

            # Embedded single and double quotes
            self.assertDeclaration(items[9],
                                   'action/report',
                                   params = [('message',
                                              "'one\"more\\'message\\''")])

    def test_assignments(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <protocol>
        <action type="assignment" variable="x" value="1"/>
        <action type="assignment" variable="foo[1][2]" value="3"/>
        <action type="assignment" variable="(1+2)[3][4][5]" value="6"/>
        <action type="assignment" variable="foo bar" value="1"/>
        <action type="assignment" variable=" var" value="1"/>
        <action type="assignment" variable="var  " value="2"/>
        <action type="assignment" variable="   " value="3"/>
        <action type="assignment" value="1"/>
        <action type="assignment" variable="x"/>
        <action type="assignment" variable="myvar" value="1 2"/>
        <action type="assignment" variable="x" value="1" foo="bar"/>
        <action type="assignment" variable="x" value="1">
            <action type="report" message="Why is this here?"></action>
        </action>
        <action type="assignment" variable="x" value=" 1"/>
        <action type="assignment" variable="x" value="2  "/>
        <action type="assignment" variable="x" value="   "/>
    </protocol>
</monkeyml>'''

        with self.convert(src) as items:
            self.assertEqual(1, len(items))
            self.assertDeclaration(items[0], 'protocol', num_children=15)
            items = items[0].children

            # Simple assignment
            self.assertAssignment(items[0], 'x', '1')

            # Index assignment
            self.assertAssignment(items[1], 'foo[1][2]', '3')

            # Variable not an allowed expression type
            self.assertDeclaration(items[2],
                                   'action/assignment',
                                   params = [('variable', '(1+2)[3][4][5]'),
                                             ('value', '6')])

            # Variable not a valid expression
            self.assertDeclaration(items[3],
                                   'action/assignment',
                                   params = [('variable', "'foo bar'"),
                                             ('value', '1')])

            # Variable contains leading whitespace
            self.assertAssignment(items[4], ' var', '1')

            # Variable contains trailing whitespace
            self.assertAssignment(items[5], 'var  ', '2')

            # Variable contains only whitespace
            self.assertDeclaration(items[6],
                                   'action/assignment',
                                   params = [('variable', "'   '"),
                                             ('value', '3')])

            # Missing variable
            self.assertDeclaration(items[7],
                                   'action/assignment',
                                   params = [('value', '1')])

            # Missing value
            self.assertDeclaration(items[8],
                                   'action/assignment',
                                   params = [('variable', 'x')])

            # Value is not a valid expression
            self.assertDeclaration(items[9],
                                   'action/assignment',
                                   params = [('variable', 'myvar'),
                                             ('value', "'1 2'")])

            # Assignment has extra parameter
            self.assertDeclaration(items[10],
                                   'action/assignment',
                                   params = [('variable', 'x'),
                                             ('value', '1'),
                                             ('foo', 'bar')])

            # Assignment has children
            self.assertDeclaration(items[11],
                                   'action/assignment',
                                   params = [('variable', 'x'),
                                             ('value', '1')],
                                   num_children = 1)
            self.assertDeclaration(items[11].children[0],
                                   'action/report',
                                   params = [('message',
                                              "'Why is this here?'")])

            # Value contains leading whitespace
            self.assertAssignment(items[12], 'x', ' 1')

            # Value contains trailing whitespace
            self.assertAssignment(items[13], 'x', '2  ')

            # Value contains only whitespace
            self.assertDeclaration(items[14],
                                   'action/assignment',
                                   params = [('variable', 'x'),
                                             ('value', "'   '")])
