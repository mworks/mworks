from __future__ import division, print_function, unicode_literals
import io
import unittest

from mwel import ErrorLogger, toxml

from . import TempFilesMixin


class TestErrorLogger(unittest.TestCase):

    def test_basic(self):
        el = ErrorLogger()
        expected_output = '''\
negative lineno, no colno
zero lineno, no colno
positive lineno, negative colno
positive lineno, zero colno
positive lineno, positive colno [line 1, column 1]
positive lineno, no colno
positive lineno, positive colno [line 15, column 9]
positive lineno, positive colno [line 20, column 8]
positive lineno, positive colno [line 20, column 10]
no lineno, no colno
'''

        el('no lineno, no colno', token='?')
        el('negative lineno, no colno', lineno=-1)
        el('zero lineno, no colno', lineno=0)
        el('positive lineno, no colno', lineno=1)
        el('positive lineno, negative colno', lineno=1, colno=-1)
        el('positive lineno, zero colno', lineno=1, colno=0)
        el('positive lineno, positive colno', lineno=1, colno=1)
        el('positive lineno, positive colno', lineno=20, colno=10)
        el('positive lineno, positive colno', lineno=15, colno=9)
        el('positive lineno, positive colno', lineno=20, colno=8)

        file = io.StringIO()
        el.print_errors(file)
        self.assertEqual(expected_output, file.getvalue())

    def test_filenames(self):
        el = ErrorLogger()
        expected_output = '''\
no file 1 [line 1, column 1]
no file 2 [line 2, column 2]
no file 3 [line 8, column 8]
file1 1 [file1.mwel: line 5, column 5]
file1 2 [file1.mwel: line 6, column 6]
file1 3 [file1.mwel]
another file [another_file: line 100, column 100]
file2 1 [file2: line 3, column 3]
file2 2 [file2: line 4, column 4]
file2 3 [file2: line 7, column 7]
'''

        el('no file 1', lineno=1, colno=1)
        el('no file 2', lineno=2, colno=2)
        with el.filename('dir1/file2'):
            el('file2 1', lineno=3, colno=3)
            el('file2 2', lineno=4, colno=4)
            with el.filename('/dir2/dir3/file1.mwel'):
                el('file1 1', lineno=5, colno=5)
                el('file1 2', lineno=6, colno=6)
                el('another file', lineno=100, colno=100,
                   filename='another_file')
                el('file1 3')
                self.assertEqual('/dir2/dir3/file1.mwel', el.current_filename)
            el('file2 3', lineno=7, colno=7)
            self.assertEqual('dir1/file2', el.current_filename)
        el('no file 3', lineno=8, colno=8)
        self.assertEqual('', el.current_filename)

        file = io.StringIO()
        el.print_errors(file)
        self.assertEqual(expected_output, file.getvalue())

    def test_backtraces(self):
        el = ErrorLogger()
        expected_output = '''\
a shorter backtrace [b.mwel: line 2, column 5; via line 3, column 5]
a long backtrace [a.mwel: line 1, column 4; via b.mwel: line 2, column 5; via line 3, column 6]
no backtrace [line 4, column 6]
another backtrace to a file [a.mwel: line 1, column 3; via b.mwel: line 2, column 4]
backtrace to a file [a.mwel: line 1, column 3; via b.mwel: line 3, column 4]
'''

        el('backtrace to a file',
           lineno = (1, 3),
           colno = (3, 4),
           filename = ('a.mwel', 'b.mwel'))
        el('no backtrace', lineno=4, colno=6)
        el('a long backtrace',
           lineno = (1, 2, 3),
           colno = (4, 5, 6),
           filename = ('a.mwel', 'b.mwel', ''))
        el('a shorter backtrace',
           lineno = (2, 3),
           colno = (5, 5),
           filename = ('b.mwel', ''))
        el('another backtrace to a file',
           lineno = (1, 2),
           colno = (3, 4),
           filename = ('a.mwel', 'b.mwel'))

        file = io.StringIO()
        el.print_errors(file)
        self.assertEqual(expected_output, file.getvalue())


class TestToXML(TempFilesMixin, unittest.TestCase):

    def toxml(self, *argv):
        self.stdout = io.BytesIO()
        self.stderr = io.StringIO()
        return toxml(argv, self.stdout, self.stderr)

    def assertOutput(self, stdout='', stderr=''):
        self.assertEqual(stdout, self.stdout.getvalue().decode('utf-8'))
        self.assertEqual(stderr, self.stderr.getvalue())

    def test_usage_error(self):
        # No args
        self.assertEqual(2, self.toxml('my_script'))
        self.assertOutput(stderr='Usage: my_script file\n')

        # Extra args
        self.assertEqual(2, self.toxml('/path/to/my_script', 'foo', 'bar'))
        self.assertOutput(stderr='Usage: my_script file\n')

    def test_file_read_error(self):
        self.assertEqual(1, self.toxml('my_script', '/path/to/not_a_file'))
        self.assertOutput(stderr=("Failed to open file '/path/to/not_a_file': "
                                  "No such file or directory\n"))

    def test_parser_error(self):
        src = '''\
var x = 1 +
var y = 7
'''
        src_path = self.write_file('experiment.mwel', src)

        self.assertEqual(1, self.toxml('my_script', src_path))
        self.assertOutput(stderr='''\
Line ended unexpectedly [line 1, column 12]
''')

    def test_analyzer_and_validator_errors(self):
        src = '''\
var x = 1
floop (3)  // Flagged by analyzer
x = 4      // Flagged by validator
var y = 7
'''
        src_path = self.write_file('experiment.mwel', src)

        self.assertEqual(1, self.toxml('my_script', src_path))
        self.assertOutput(stderr='''\
Cannot infer parameter name for component 'floop' [line 2, column 1]
Component 'action/assignment' is not allowed at the top level [line 3, column 3]
''')

    def test_success(self):
        src = '''\
var x = 2

folder 'Other Vars' {
    var foo = 12
    var bar = 'This is a string'
    var blah = [1.5, 2.5, 3.5]
}

protocol 'Test Protocol' {
    if (x > 1) {
        report ('x is greater than 1!')
    }
    x = 'foo'
    report ('x = $x')
}

experiment 'My Experiment' {}
'''
        src_path = self.write_file('experiment.mwel', src)

        self.assertEqual(0, self.toxml('my_script', src_path))
        self.assertOutput(stdout='''\
<?xml version='1.0' encoding='UTF-8'?>
<monkeyml version="1.0">
  <variable _location="line 1, column 1" default_value="2" tag="x" />
  <folder _location="line 3, column 1" tag="Other Vars">
    <variable _location="line 4, column 5" default_value="12" tag="foo" />
    <variable _location="line 5, column 5" default_value="'This is a string'" tag="bar" />
    <variable _location="line 6, column 5" default_value="[1.5, 2.5, 3.5]" tag="blah" />
  </folder>
  <experiment _location="line 17, column 1" tag="My Experiment">
    <protocol _location="line 9, column 1" tag="Test Protocol">
      <action _location="line 10, column 5" condition="x &gt; 1" type="if">
        <action _location="line 11, column 9" message="x is greater than 1!" type="report" />
      </action>
      <action _location="line 13, column 7" type="assignment" value="'foo'" variable="x" />
      <action _location="line 14, column 5" message="x = $x" type="report" />
    </protocol>
  </experiment>
  <variable_assignments>
    <variable_assignment variable="#loadedExperiment">
      <dictionary>
        <dictionary_element>
          <key>%s</key>
          <value type="string">var x = 2

folder 'Other Vars' {
    var foo = 12
    var bar = 'This is a string'
    var blah = [1.5, 2.5, 3.5]
}

protocol 'Test Protocol' {
    if (x &gt; 1) {
        report ('x is greater than 1!')
    }
    x = 'foo'
    report ('x = $x')
}

experiment 'My Experiment' {}
</value>
        </dictionary_element>
      </dictionary>
    </variable_assignment>
  </variable_assignments>
</monkeyml>
''' % src_path)

    def test_includes(self):
        experiment_src = '''\
var x = 2

folder 'Other Vars' {
    var foo = 12
    var bar = 'This is a string'
    var blah = [1.5, 2.5, 3.5]
}

%include protocol

experiment 'My Experiment' {}

protocol 'Another Protocol' {
    set_x('bar')
}
'''
        experiment_path = self.write_file('experiment.mwel', experiment_src)

        protocol_src = '''\
%define set_x(value)
    x = value
%end
protocol 'Test Protocol' {
    if (x > 1) {
        report ('x is greater than 1!')
    }
    set_x('foo')
    report ('x = $x')
}
'''
        protocol_path = self.write_file('protocol.mwel', protocol_src)

        self.assertEqual(0, self.toxml('my_script', experiment_path))
        self.assertOutput(stdout='''\
<?xml version='1.0' encoding='UTF-8'?>
<monkeyml version="1.0">
  <variable _location="line 1, column 1" default_value="2" tag="x" />
  <folder _location="line 3, column 1" tag="Other Vars">
    <variable _location="line 4, column 5" default_value="12" tag="foo" />
    <variable _location="line 5, column 5" default_value="'This is a string'" tag="bar" />
    <variable _location="line 6, column 5" default_value="[1.5, 2.5, 3.5]" tag="blah" />
  </folder>
  <experiment _location="line 11, column 1" tag="My Experiment">
    <protocol _location="protocol.mwel: line 4, column 1" tag="Test Protocol">
      <action _location="protocol.mwel: line 5, column 5" condition="x &gt; 1" type="if">
        <action _location="protocol.mwel: line 6, column 9" message="x is greater than 1!" type="report" />
      </action>
      <action _location="protocol.mwel: line 2, column 7; via protocol.mwel: line 8, column 5" type="assignment" value="'foo'" variable="x" />
      <action _location="protocol.mwel: line 9, column 5" message="x = $x" type="report" />
    </protocol>
    <protocol _location="line 13, column 1" tag="Another Protocol">
      <action _location="protocol.mwel: line 2, column 7; via line 14, column 5" type="assignment" value="'bar'" variable="x" />
    </protocol>
  </experiment>
  <variable_assignments>
    <variable_assignment variable="#loadedExperiment">
      <dictionary>
        <dictionary_element>
          <key>%s</key>
          <value type="string">var x = 2

folder 'Other Vars' {
    var foo = 12
    var bar = 'This is a string'
    var blah = [1.5, 2.5, 3.5]
}

%%include protocol

experiment 'My Experiment' {}

protocol 'Another Protocol' {
    set_x('bar')
}
</value>
        </dictionary_element>
        <dictionary_element>
          <key>%s</key>
          <value type="string">%%define set_x(value)
    x = value
%%end
protocol 'Test Protocol' {
    if (x &gt; 1) {
        report ('x is greater than 1!')
    }
    set_x('foo')
    report ('x = $x')
}
</value>
        </dictionary_element>
      </dictionary>
    </variable_assignment>
  </variable_assignments>
</monkeyml>
''' % (experiment_path, protocol_path))
