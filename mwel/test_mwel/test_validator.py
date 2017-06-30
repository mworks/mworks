from __future__ import division, print_function, unicode_literals
import unittest

from mwel.analyzer import Analyzer, Component
from mwel.parser import Parser
from mwel.validator import Validator

from . import ErrorLoggerMixin, TempFilesMixin


class ValidatorTestMixin(ErrorLoggerMixin):

    def setUp(self):
        super(ValidatorTestMixin, self).setUp()

        parser = Parser(self.error_logger)
        analyzer = Analyzer(self.error_logger)
        validator = Validator(self.error_logger)

        def validate(*args):
            tree = parser.parse(*args)
            if tree:
                cmpts = analyzer.analyze(tree)
                return validator.validate(cmpts)

        self.validate = self.with_error_check(validate)

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


class TestValidator(ValidatorTestMixin, unittest.TestCase):

    def assertDefaultExperimentAndProtocol(self, cmpts):
            cmpts = self.assertComponent(cmpts[-1], -1, -1,
                                         name = 'experiment',
                                         tag = 'Experiment')
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], -1, -1,
                                            name = 'protocol',
                                            tag = 'Empty Protocol')
            self.assertEqual(0, len(children))

    def test_default_experiment_and_protocol(self):
        # No experiment or protocol
        with self.validate('') as cmpts:
            self.assertEqual(1, len(cmpts))
            self.assertDefaultExperimentAndProtocol(cmpts)

        # Experiment, no protocol
        with self.validate('''
                           experiment 'My Experiment' {}
                           ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            cmpts = self.assertComponent(cmpts[0], 2, 28,
                                         name = 'experiment',
                                         tag = 'My Experiment')
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], -1, -1,
                                            name = 'protocol',
                                            tag = 'Empty Protocol')
            self.assertEqual(0, len(children))

        # Protocol, no experiment
        with self.validate('''
                           protocol 'My Protocol' {}
                           ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            cmpts = self.assertComponent(cmpts[0], -1, -1,
                                         name = 'experiment',
                                         tag = 'Experiment')
            self.assertEqual(1, len(cmpts))
            children = self.assertComponent(cmpts[0], 2, 28,
                                            name = 'protocol',
                                            tag = 'My Protocol')
            self.assertEqual(0, len(children))

        # Experiment and protocols (with top-level protocol)
        with self.validate('''
                           protocol 'Protocol 3' {}
                           experiment 'My Experiment' {
                               protocol 'Protocol 1' {}
                               protocol 'Protocol 2' {}
                           }
                           ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            cmpts = self.assertComponent(cmpts[0], 3, 28,
                                         name = 'experiment',
                                         tag = 'My Experiment')
            self.assertEqual(3, len(cmpts))

            children = self.assertComponent(cmpts[0], 4, 32,
                                            name = 'protocol',
                                            tag = 'Protocol 1')
            self.assertEqual(0, len(children))

            children = self.assertComponent(cmpts[1], 5, 32,
                                            name = 'protocol',
                                            tag = 'Protocol 2')
            self.assertEqual(0, len(children))

            children = self.assertComponent(cmpts[2], 2, 28,
                                            name = 'protocol',
                                            tag = 'Protocol 3')
            self.assertEqual(0, len(children))

        # Experiment and protocols (without top-level protocol)
        with self.validate('''
                           experiment 'My Experiment' {
                               protocol 'Protocol 1' {}
                               protocol 'Protocol 2' {}
                           }
                           ''') as cmpts:
            self.assertEqual(1, len(cmpts))
            cmpts = self.assertComponent(cmpts[0], 2, 28,
                                         name = 'experiment',
                                         tag = 'My Experiment')
            self.assertEqual(2, len(cmpts))

            children = self.assertComponent(cmpts[0], 3, 32,
                                            name = 'protocol',
                                            tag = 'Protocol 1')
            self.assertEqual(0, len(children))

            children = self.assertComponent(cmpts[1], 4, 32,
                                            name = 'protocol',
                                            tag = 'Protocol 2')
            self.assertEqual(0, len(children))

    def test_multiple_experiments(self):
        with self.validate('''
                           experiment 'Experiment 1' {}
                           experiment 'Experiment 2' {}
                           '''):
            self.assertError('Experiment cannot contain more than one '
                             "'experiment' component",
                             lineno = 3,
                             colno = 28)

    def test_toplevel(self):
        with self.validate('''
                           // Allowed
                           var x = 1

                           // Not allowed
                           x = 2

                           // Unknown
                           floop {}
                           ''') as cmpts:
            self.assertEqual(4, len(cmpts))

            children = self.assertComponent(cmpts[0], 3, 28,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = '1')
            self.assertEqual(0, len(children))

            self.assertError("Component 'Assign Variable' is not allowed "
                             "at the top level",
                             lineno = 6,
                             colno = 30)
            children = self.assertComponent(cmpts[1], 6, 30,
                                            name = 'action',
                                            type = 'assignment',
                                            variable = 'x',
                                            value = '2')
            self.assertEqual(0, len(children))

            children = self.assertComponent(cmpts[2], 9, 28,
                                            name = 'floop')
            self.assertEqual(0, len(children))

            self.assertDefaultExperimentAndProtocol(cmpts)

    def test_toplevel_via_transient(self):
        with self.validate('''
                           list_replicator (variable = foo; values = 1,2,3) {
                               // Allowed
                               stimulus/blank_screen bg ()

                               list_replicator (variable = bar; values = 4,5) {
                                   // Not allowed
                                   block 'A Block' {}

                                   // Unknown
                                   floop {}
                               }
                           }
                           ''') as cmpts:
            self.assertEqual(2, len(cmpts))
            self.assertDefaultExperimentAndProtocol(cmpts)

            cmpts = self.assertComponent(cmpts[0], 2, 28,
                                         name = 'list_replicator',
                                         variable = 'foo',
                                         values = '1, 2, 3')
            self.assertEqual(2, len(cmpts))

            children = self.assertComponent(cmpts[0], 4, 32,
                                            name = 'stimulus',
                                            type = 'blank_screen',
                                            tag = 'bg')
            self.assertEqual(0, len(children))

            cmpts = self.assertComponent(cmpts[1], 6, 32,
                                         name = 'list_replicator',
                                         variable = 'bar',
                                         values = '4, 5')
            self.assertEqual(2, len(cmpts))

            self.assertError("Component 'Block' is not allowed "
                             "at the top level",
                             lineno = 8,
                             colno = 36)
            children = self.assertComponent(cmpts[0], 8, 36,
                                            name = 'block',
                                            tag = 'A Block')
            self.assertEqual(0, len(children))

            children = self.assertComponent(cmpts[1], 11, 36,
                                            name = 'floop')
            self.assertEqual(0, len(children))

    def test_parent(self):
        with self.validate('''
                           stimulus_group 'The Stims' {
                               // Allowed
                               stimulus/blank_screen bg {}

                               // Not allowed
                               var x = 3

                               // Unknown
                               floop {
                                   // Not allowed (unknown parent)
                                   stimulus/blank_screen bg2 {}
                               }
                           }
                           ''') as cmpts:
            self.assertEqual(2, len(cmpts))
            self.assertDefaultExperimentAndProtocol(cmpts)

            cmpts = self.assertComponent(cmpts[0], 2, 28,
                                         name = 'stimulus_group',
                                         tag = 'The Stims')
            self.assertEqual(3, len(cmpts))

            children = self.assertComponent(cmpts[0], 4, 32,
                                            name = 'stimulus',
                                            type = 'blank_screen',
                                            tag = 'bg')
            self.assertEqual(0, len(children))

            self.assertError("Component 'Variable' is not allowed inside "
                             "component 'Stimulus Group'",
                             lineno = 7,
                             colno = 32)
            children = self.assertComponent(cmpts[1], 7, 32,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = '3')
            self.assertEqual(0, len(children))

            cmpts = self.assertComponent(cmpts[2], 10, 32,
                                         name = 'floop')
            self.assertEqual(1, len(cmpts))

            self.assertError("Component 'Blank Fullscreen Stimulus' is not "
                             "allowed inside component 'floop'",
                             lineno = 12,
                             colno = 36)
            children = self.assertComponent(cmpts[0], 12, 36,
                                            name = 'stimulus',
                                            type = 'blank_screen',
                                            tag = 'bg2')
            self.assertEqual(0, len(children))

    def test_parent_via_transient(self):
        with self.validate('''
                           stimulus_group 'The Stims' {
                               list_replicator (variable = i; values = 1:3) {
                                   // Allowed
                                   stimulus/blank_screen bg {}

                                   list_replicator (variable=j; values=4:5) {
                                       // Not allowed
                                       block 'A Block' {}

                                       // Unknown
                                       floop {}
                                   }
                               }
                           }
                           ''') as cmpts:
            self.assertEqual(2, len(cmpts))
            self.assertDefaultExperimentAndProtocol(cmpts)

            cmpts = self.assertComponent(cmpts[0], 2, 28,
                                         name = 'stimulus_group',
                                         tag = 'The Stims')
            self.assertEqual(1, len(cmpts))

            cmpts = self.assertComponent(cmpts[0], 3, 32,
                                         name = 'list_replicator',
                                         variable = 'i',
                                         values = '1:3')
            self.assertEqual(2, len(cmpts))

            children = self.assertComponent(cmpts[0], 5, 36,
                                            name = 'stimulus',
                                            type = 'blank_screen',
                                            tag = 'bg')
            self.assertEqual(0, len(children))

            cmpts = self.assertComponent(cmpts[1], 7, 36,
                                         name = 'list_replicator',
                                         variable = 'j',
                                         values = '4:5')
            self.assertEqual(2, len(cmpts))

            self.assertError("Component 'Block' is not allowed inside "
                             "component 'Stimulus Group'",
                             lineno = 9,
                             colno = 40)
            children = self.assertComponent(cmpts[0], 9, 40,
                                            name = 'block',
                                            tag = 'A Block')
            self.assertEqual(0, len(children))

            children = self.assertComponent(cmpts[1], 12, 40,
                                            name = 'floop')
            self.assertEqual(0, len(children))


class TestIncludes(ValidatorTestMixin, TempFilesMixin, unittest.TestCase):

    def test_filenames_in_errors(self):
        base_src = '''\
experiment foo {}
%include include1
'''

        include1_src = '''\
%include include2
protocol bar {
    var x = 1
}
'''
        include1_path = self.write_file('include1.mwel', include1_src)

        include2_src = '''\
%include include3
experiment bar {}
'''
        include2_path = self.write_file('include2.mwel', include2_src)

        include3_src = '''\
x = 2
'''
        include3_path = self.write_file('include3.mwel', include3_src)

        with self.validate(base_src, self.tmpdir):
            self.assertError("Component 'Assign Variable' is not allowed "
                             "at the top level",
                             lineno = 1,
                             colno = 3,
                             filename = include3_path)
            self.assertError('Experiment cannot contain more than one '
                             "'experiment' component",
                             lineno = 2,
                             colno = 1,
                             filename = include2_path)
            self.assertError("Component 'Variable' is not allowed inside "
                             "component 'Protocol'",
                             lineno = 3,
                             colno = 5,
                             filename = include1_path)
