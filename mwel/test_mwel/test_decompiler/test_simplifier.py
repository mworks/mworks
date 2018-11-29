from __future__ import division, print_function, unicode_literals
import unittest

from mwel.decompiler.simplifier import Simplifier
from mwel.decompiler.xmlparser import XMLParser

from . import ElementCheckMixin
from .. import ErrorLoggerMixin


class TestSimplifier(ElementCheckMixin, ErrorLoggerMixin, unittest.TestCase):

    def setUp(self):
        super(TestSimplifier, self).setUp()

        xmlparser = XMLParser(self.error_logger)
        simplifier = Simplifier(self.error_logger)

        def simplify(src_data):
            root = xmlparser.parse(src_data)
            if root is not None:
                simplifier.simplify(root)
                return root

        self.simplify = self.with_error_check(simplify)

    def test_ignored_elements(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml>
    <protocol>
        <task_system>
            <task_system_state tag="Exit State System">
                <action_marker></action_marker>
                <action type="report" message="Leaving state system"></action>
                <transition_marker>
                    <action type="report" message="Not cool!"></action>
                </transition_marker>
                <transition type="yield"></transition>
            </task_system_state>
        </task_system>
    </protocol>
</monkeyml>'''

        with self.simplify(src) as root:
            self.assertElement(root, 'monkeyml', num_children=1)
            root = root.children[0]

            self.assertElement(root, 'protocol', num_children=1)
            root = root.children[0]

            self.assertElement(root, 'task_system', num_children=1)
            root = root.children[0]

            self.assertElement(root,
                               'task_system_state',
                               [('tag', 'Exit State System')],
                               num_children = 2)
            elems = root.children

            self.assertElement(elems[0],
                               'action',
                               [('type', 'report'),
                                ('message', 'Leaving state system')])

            self.assertError("Ignored element 'transition_marker' has "
                             'unexpected children that will also be ignored',
                             lineno = 8,
                             colno = 16)

            self.assertElement(elems[1],
                               'transition',
                               [('type', 'yield')])

    def test_omitted_tags(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <list_replicator tag="New List Replicator" values="1:10" variable="lr_var"/>
    <range_replicator tag="New Range Replicator" from="1" to="10" step="1" variable="rr_var"/>
    <protocol tag="New Protocol">
        <task_system tag="New Task System">
            <task_system_state tag="Exit State System">
                <action type="report" tag="Announce Exit State System" message="Leaving state system"></action>
                <transition tag="Return to parent task system" type="yield"></transition>
            </task_system_state>
        </task_system>
    </protocol>
</monkeyml>'''

        with self.simplify(src) as root:
            self.assertElement(root,
                               'monkeyml',
                               [('version', '1.0')],
                               num_children = 3)
            elems = root.children

            self.assertElement(elems[0],
                               'list_replicator',
                               [('values', '1:10'),
                                ('variable', 'lr_var')])

            self.assertElement(elems[1],
                               'range_replicator',
                               [('from', '1'),
                                ('to', '10'),
                                ('step', '1'),
                                ('variable', 'rr_var')])

            self.assertElement(elems[2],
                               'protocol',
                               [('tag', 'New Protocol')],
                               num_children = 1)
            root = elems[2].children[0]

            self.assertElement(root,
                               'task_system',
                               [('tag', 'New Task System')],
                               num_children = 1)
            root = root.children[0]

            self.assertElement(root,
                               'task_system_state',
                               [('tag', 'Exit State System')],
                               num_children = 2)
            elems = root.children

            self.assertElement(elems[0],
                               'action',
                               [('type', 'report'),
                                ('message', 'Leaving state system')])

            self.assertElement(elems[1],
                               'transition',
                               [('type', 'yield')])

    def test_ignored_attributes(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <protocol _id="abc123" tag="New Protocol" _line_number="45">
        <task_system _location="Somewhere" full_name="Whatever" tag="New Task System"/>
    </protocol>
</monkeyml>'''

        with self.simplify(src) as root:
            self.assertElement(root,
                               'monkeyml',
                               [('version', '1.0')],
                               num_children = 1)
            root = root.children[0]

            self.assertElement(root,
                               'protocol',
                               [('tag', 'New Protocol')],
                               num_children = 1)
            root = root.children[0]

            self.assertElement(root,
                               'task_system',
                               [('tag', 'New Task System')])

    def test_removed_empty_attributes(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <protocol nsamples="" sampling_method="samples">
        <task_system tag="New Task System" interruptible=""/>
    </protocol>
</monkeyml>'''

        with self.simplify(src) as root:
            self.assertElement(root,
                               'monkeyml',
                               [('version', '1.0')],
                               num_children = 1)
            root = root.children[0]

            self.assertElement(root,
                               'protocol',
                               [('sampling_method', 'samples')],
                               num_children = 1)
            root = root.children[0]

            self.assertElement(root,
                               'task_system',
                               [('tag', 'New Task System')])
