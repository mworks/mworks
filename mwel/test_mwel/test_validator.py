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

    def test_invalid_signature(self):
        with self.validate('''
                           action/floop ()  // OK
                           floop ()         // Not OK
                           floop/list ()    // Not OK
                           '''):
            self.assertError("'floop' is not a valid component type signature",
                             lineno = 3,
                             colno = 28)
            self.assertError("'floop/list' is not a valid component type "
                             'signature',
                             lineno = 4,
                             colno = 28)

    def test_toplevel(self):
        with self.validate('''
                           // Allowed
                           var x = 1
                           x = 2  // Converted to variable_assignment

                           // Not allowed
                           block {}

                           // Allowed (unknown)
                           action/floop {}
                           ''') as cmpts:
            self.assertEqual(5, len(cmpts))

            children = self.assertComponent(cmpts[0], 3, 28,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = '1')
            self.assertEqual(0, len(children))

            children = self.assertComponent(cmpts[1], 4, 30,
                                            name = 'variable_assignment',
                                            variable = 'x',
                                            value = '2',
                                            type = 'any')
            self.assertEqual(0, len(children))

            self.assertError("Component 'block' is not allowed "
                             "at the top level",
                             lineno = 7,
                             colno = 28)
            children = self.assertComponent(cmpts[2], 7, 28,
                                            name = 'block')
            self.assertEqual(0, len(children))

            children = self.assertComponent(cmpts[3], 10, 28,
                                            name = 'action',
                                            type = 'floop')
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

                                   // Allowed (unknown)
                                   action/floop {}
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

            self.assertError("Component 'block' is not allowed "
                             "at the top level",
                             lineno = 8,
                             colno = 36)
            children = self.assertComponent(cmpts[0], 8, 36,
                                            name = 'block',
                                            tag = 'A Block')
            self.assertEqual(0, len(children))

            children = self.assertComponent(cmpts[1], 11, 36,
                                            name = 'action',
                                            type = 'floop')
            self.assertEqual(0, len(children))

    def test_parent(self):
        with self.validate('''
                           stimulus_group 'The Stims' {
                               // Allowed
                               stimulus/blank_screen bg {}

                               // Not allowed
                               var x = 3

                               // Allowed (unknown)
                               action/floop {
                                   // Allowed (unknown parent)
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

            self.assertError("Component 'variable' is not allowed inside "
                             "component 'stimulus_group'",
                             lineno = 7,
                             colno = 32)
            children = self.assertComponent(cmpts[1], 7, 32,
                                            name = 'variable',
                                            tag = 'x',
                                            default_value = '3')
            self.assertEqual(0, len(children))

            cmpts = self.assertComponent(cmpts[2], 10, 32,
                                         name = 'action',
                                         type = 'floop')
            self.assertEqual(1, len(cmpts))

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

                                       // Allowed (unknown)
                                       action/floop {}
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

            self.assertError("Component 'block' is not allowed inside "
                             "component 'stimulus_group'",
                             lineno = 9,
                             colno = 40)
            children = self.assertComponent(cmpts[0], 9, 40,
                                            name = 'block',
                                            tag = 'A Block')
            self.assertEqual(0, len(children))

            children = self.assertComponent(cmpts[1], 12, 40,
                                            name = 'action',
                                            type = 'floop')
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
%include include4
block {}
'''
        include3_path = self.write_file('include3.mwel', include3_src)

        include4_src = '''\
floop ()
'''
        include4_path = self.write_file('include4.mwel', include4_src)

        with self.validate(base_src, self.tmpdir):
            self.assertError("'floop' is not a valid component type signature",
                             lineno = 1,
                             colno = 1,
                             filename = include4_path)
            self.assertError("Component 'block' is not allowed "
                             "at the top level",
                             lineno = 2,
                             colno = 1,
                             filename = include3_path)
            self.assertError('Experiment cannot contain more than one '
                             "'experiment' component",
                             lineno = 2,
                             colno = 1,
                             filename = include2_path)
            self.assertError("Component 'variable' is not allowed inside "
                             "component 'protocol'",
                             lineno = 3,
                             colno = 5,
                             filename = include1_path)
