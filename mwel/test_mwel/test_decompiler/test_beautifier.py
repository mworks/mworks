from __future__ import division, print_function, unicode_literals
import unittest

from mwel.decompiler.beautifier import Beautifier
from mwel.decompiler.converter import Converter
from mwel.decompiler.simplifier import Simplifier
from mwel.decompiler.xmlparser import XMLParser

from . import DeclarationCheckMixin
from .. import ErrorLoggerMixin


class TestBeautifier(DeclarationCheckMixin, ErrorLoggerMixin, unittest.TestCase):

    def setUp(self):
        super(TestBeautifier, self).setUp()

        xmlparser = XMLParser(self.error_logger)
        simplifier = Simplifier(self.error_logger)
        converter = Converter(self.error_logger)
        beautifier = Beautifier(self.error_logger)

        def beautify(src_data):
            root = xmlparser.parse(src_data)
            if root is not None:
                simplifier.simplify(root)
                items = converter.convert(root)
                beautifier.beautify(items)
                return items

        self.beautify = self.with_error_check(beautify)

    def test_shortened_type_names(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <iodevice type="ne500"/>
    <protocol>
        <action type="update_stimulus_display"/>
        <action type="update_display"/>
        <florb type="blorb"/>
        <florb type="ne500"/>
        <florb type="update_stimulus_display"/>
    </protocol>
</monkeyml>'''

        with self.beautify(src) as items:
            self.assertEqual(2, len(items))

            # Non-unique subtype
            self.assertDeclaration(items[0], 'iodevice/ne500')

            # No subtype
            self.assertDeclaration(items[1], 'protocol', num_children=5)
            items = items[1].children

            # Unique subtype
            self.assertDeclaration(items[0], 'update_stimulus_display')

            # Unique subtype (alias)
            self.assertDeclaration(items[1], 'update_display')

            # Unknown base type with unknown subtype
            self.assertDeclaration(items[2], 'florb/blorb')

            # Unknown base type with known, non-unique subtype
            self.assertDeclaration(items[3], 'florb/ne500')

            # Unknown base type with known, unique subtype
            self.assertDeclaration(items[4], 'florb/update_stimulus_display')

    def test_omitted_parameter_names(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <protocol nsamples="5" sampling_method="samples"/>
    <protocol nsamples="10">
        <action type="assert" condition="!x" message="Uh, oh!"/>
        <action type="assert" message="Not ok!"/>
        <action type="assert" condition="x == y"/>
        <action type="update_stimulus_display" foo="bar"/>
        <action type="update_stimulus_display" predicted_output_time="update_time"/>
        <action type="start_device_io" foo="bar"/>
        <action type="start_device_io" device="the_device"/>
        <action type="take_calibration_sample" calibrator="foo" calibratable_object="bar"/>
        <action type="take_calibration_sample" calibrator="foo"/>
        <florb type="blorb" morb="glorb"/>
    </protocol>
</monkeyml>'''

        with self.beautify(src) as items:
            self.assertEqual(2, len(items))

            # Multiple parameters, none required; more than one provided
            self.assertDeclaration(items[0],
                                   'protocol',
                                   params = [('nsamples', '5'),
                                             ('sampling_method', 'samples')])

            # Multiple parameters, none required; one provided
            self.assertDeclaration(items[1],
                                   'protocol',
                                   params = [('nsamples', '10')],
                                   num_children = 10)
            items = items[1].children

            # Multiple parameters, one required; more than one provided
            self.assertDeclaration(items[0],
                                   'assert',
                                   params = [('condition', '!x'),
                                             ('message', "'Uh, oh!'")])

            # Multiple parameters, one required; wrong parameter provided
            self.assertDeclaration(items[1],
                                   'assert',
                                   params = [('message', "'Not ok!'")])

            # Multiple parameters, one required; correct parameter provided
            self.assertDeclaration(items[2],
                                   'assert',
                                   params = 'x == y')

            # One parameter, not required; wrong parameter provided
            self.assertDeclaration(items[3],
                                   'update_stimulus_display',
                                   params = [('foo', 'bar')])

            # One parameter, not required; correct parameter provided
            self.assertDeclaration(items[4],
                                   'update_stimulus_display',
                                   params = [('predicted_output_time',
                                              'update_time')])

            # One parameter, required; wrong parameter provided
            self.assertDeclaration(items[5],
                                   'start_device_io',
                                   params = [('foo', 'bar')])

            # One parameter, required; correct parameter provided
            self.assertDeclaration(items[6],
                                   'start_device_io',
                                   params = 'the_device')

            # Multiple parameters, multiple required; multiple provided
            self.assertDeclaration(items[7],
                                   'take_calibration_sample',
                                   params = [('calibrator', 'foo'),
                                             ('calibratable_object', 'bar')])

            # Multiple parameters, multiple required; one provided
            self.assertDeclaration(items[8],
                                   'take_calibration_sample',
                                   params = [('calibrator', 'foo')])

            # Unknown parameters; one provided
            self.assertDeclaration(items[9],
                                   'florb/blorb',
                                   params = [('morb', 'glorb')])

    def test_comment_and_assignment_passthrough(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <!-- Here's a comment -->
    <protocol>
        <action type="assignment" variable="x" value="1"/>
    </protocol>
</monkeyml>'''

        with self.beautify(src) as items:
            self.assertEqual(2, len(items))

            self.assertComment(items[0], " Here's a comment ")

            self.assertDeclaration(items[1], 'protocol', num_children=1)
            items = items[1].children

            self.assertAssignment(items[0], 'x', '1')

    def test_preferred_aliases(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
<folder tag="My Vars">
    <variable tag="var1" default_value="1"/>
    <variable tag="var2" type="selection" values="1:5"/>
</folder>
<protocol>
    <task_system tag="New Task System">
        <task_system_state tag="Exit State System">
            <action type="report" message="Leaving state system"></action>
            <transition type="yield"/>
        </task_system_state>
    </task_system>
</protocol>
</monkeyml>'''

        with self.beautify(src) as items:
            self.assertEqual(2, len(items))

            self.assertDeclaration(items[0],
                                   'group',
                                   "'My Vars'",
                                   num_children = 2)

            self.assertDeclaration(items[0].children[0],
                                   'var',
                                   'var1',
                                   value = '1')

            self.assertDeclaration(items[0].children[1],
                                   'selection',
                                   'var2',
                                   params = '1:5')

            self.assertDeclaration(items[1], 'protocol', num_children=1)
            items = items[1].children

            self.assertDeclaration(items[0],
                                   'task',
                                   "'New Task System'",
                                   num_children = 1)
            items = items[0].children

            self.assertDeclaration(items[0],
                                   'state',
                                   "'Exit State System'",
                                   num_children = 2)
            items = items[0].children

            self.assertDeclaration(items[0],
                                   'report',
                                   params = "'Leaving state system'")

            self.assertDeclaration(items[1], 'yield')

    def test_transition_conversion(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <transition type="conditional" condition="!x" target="x is false"/>
    <transition type="direct" target="always go here"/>
    <transition type="goto" target="x equals y" when="x == y"/>
    <transition type="timer_expired" target="timer expired" timer="my timer"/>
    <transition type="yield"/>
    <transition type="conditional" target="Huh?"/>
    <transition type="direct" flarget="What?"/>
</monkeyml>'''

        with self.beautify(src) as items:
            self.assertEqual(7, len(items))

            # conditional: converted
            self.assertDeclaration(items[0],
                                   'goto',
                                   params = [('target', "'x is false'"),
                                             ('when', '!x')])

            # direct: converted
            self.assertDeclaration(items[1],
                                   'goto',
                                   params = "'always go here'")

            # goto: not converted
            self.assertDeclaration(items[2],
                                   'goto',
                                   params = [('target', "'x equals y'"),
                                             ('when', 'x == y')])

            # timer_expired: not converted
            self.assertDeclaration(items[3],
                                   'timer_expired',
                                   params = [('target', "'timer expired'"),
                                             ('timer', "'my timer'")])

            # yield: not converted
            self.assertDeclaration(items[4], 'yield')

            # Invalid conditional: not converted
            self.assertDeclaration(items[5],
                                   'conditional',
                                   params = [('target', "'Huh?'")])

            # Invalid direct: not converted
            self.assertDeclaration(items[6],
                                   'direct',
                                   params = [('flarget', "'What?'")])

    def test_default_valued_parameter_removal(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <protocol nsamples="1" sampling_method="samples" selection="sequential" interruptible="Yes">
        <action type="assert" condition="!x" stop_on_failure="NO"></action>
    </protocol>
</monkeyml>'''

        with self.beautify(src) as items:
            self.assertEqual(1, len(items))

            self.assertDeclaration(items[0],
                                   'protocol',
                                   params = [('sampling_method', 'samples'),
                                             ('interruptible', 'Yes')],
                                   num_children=1)
            items = items[0].children

            self.assertDeclaration(items[0], 'assert', params = '!x')
