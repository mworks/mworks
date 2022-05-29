from contextlib import contextmanager
import io
import unittest

from mwel.analyzer import Analyzer
from mwel.parser import Parser
from mwel.validator import Validator

from mwel.decompiler.beautifier import Beautifier
from mwel.decompiler.converter import Converter
from mwel.decompiler.outputter import Outputter
from mwel.decompiler.simplifier import Simplifier
from mwel.decompiler.xmlparser import XMLParser

from .. import ErrorLoggerMixin


class TestOutputter(ErrorLoggerMixin, unittest.TestCase):

    def setUp(self):
        super(TestOutputter, self).setUp()
        self.maxDiff = None

        xmlparser = XMLParser(self.error_logger)
        simplifier = Simplifier(self.error_logger)
        converter = Converter(self.error_logger)
        beautifier = Beautifier(self.error_logger)
        outputter = Outputter(self.error_logger)

        parser = Parser(self.error_logger)
        analyzer = Analyzer(self.error_logger)
        validator = Validator(self.error_logger)

        @contextmanager
        def output(src_data):
            root = xmlparser.parse(src_data)
            self.assertNoErrors()

            simplifier.simplify(root)
            items = converter.convert(root)
            beautifier.beautify(items)
            output_stream = io.StringIO()
            outputter.output(items, output_stream)
            output = output_stream.getvalue()

            yield output
            self.assertNoErrors()

            # Confirm that the generated MWEL is valid
            tree = parser.parse(output)
            self.assertNoErrors()
            cmpts = analyzer.analyze(tree)
            cmpts = validator.validate(cmpts)
            self.assertNoErrors()

        self.output = output

    def test_comments(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
<!-- This comment is at
     the top level
-->
  <!-- This one is all on one line -->
    <protocol>
        <!-- This comment is
             inside a protocol -->
        <action type="assignment" variable="x" value="1"/> <!--Inline comment-->
    </protocol>
</monkeyml>'''

        expected_output = '''\
/* This comment is at
     the top level
*/
// This one is all on one line 
protocol {
    /* This comment is
             inside a protocol */
    x = 1
    //Inline comment
}
'''

        with self.output(src) as output:
            self.assertEqual(expected_output, output)

    def test_assignments(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <protocol>
        <action type="assignment" variable="x" value="1"/>
        <trial>
            <action type="assignment" variable="my_var" value="2*x + 3"/>
        </trial>
    </protocol>
</monkeyml>'''

        expected_output = '''\
protocol {
    x = 1
    trial {
        my_var = 2*x + 3
    }
}
'''

        with self.output(src) as output:
            self.assertEqual(expected_output, output)

    def test_groups(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <io_devices tag="blah"></io_devices>
    <folder>
        <variables tag="blah">
            <sounds tag="blah">
                <range_replicator from="1" to="10" step="1" variable="rr_var">
                    <stimuli tag="blah">
                        <stimulus type="white_noise_background"/>
                    </stimuli>
                </range_replicator>
            </sounds>
        </variables>
    </folder>
    <filters tag="blah"></filters>
    <optimizers tag="blah"></optimizers>
    <calibrators tag="blah"></calibrators>
    <resources tag="blah"></resources>
    <experiment tag="blah">
        <protocol>
            <action type="assignment" variable="x" value="1"></action>
        </protocol>
    </experiment>
</monkeyml>'''

        expected_output = '''\

//
// I/O Devices
//

group {

    //
    // Variables
    //


    //
    // Sounds
    //

    range_replicator (
        from = 1
        to = 10
        step = 1
        variable = rr_var
        ) {

        //
        // Stimuli
        //

        white_noise_background ()
    }
}

//
// Filters
//


//
// Optimizers
//


//
// Calibrators
//


//
// Resources
//


//
// Protocols
//

protocol {
    x = 1
}
'''

        with self.output(src) as output:
            self.assertEqual(expected_output, output)

    def test_declaration_signatures(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <!-- Base type and subtype-->
    <iodevice type="ne500"/>
    <!-- No subtype-->
    <protocol>
        <!-- Base type omitted-->
        <action type="update_stimulus_display"/>
    </protocol>
</monkeyml>'''

        expected_output = '''\
// Base type and subtype
iodevice/ne500 ()
// No subtype
protocol {
    // Base type omitted
    update_stimulus_display ()
}
'''

        with self.output(src) as output:
            self.assertEqual(expected_output, output)

    def test_declaration_tags(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <!-- No tag-->
    <protocol/>
    <!-- Tag is a valid identifier-->
    <protocol tag="protocol_1"/>
    <!-- Tag is not a valid identifier-->
    <protocol tag="Protocol 2"/>
</monkeyml>'''

        expected_output = '''\
// No tag
protocol ()
// Tag is a valid identifier
protocol protocol_1 ()
// Tag is not a valid identifier
protocol 'Protocol 2' ()
'''

        with self.output(src) as output:
            self.assertEqual(expected_output, output)

    def test_declaration_values(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <!-- No other params-->
    <variable tag="var1" default_value="1"/>
    <!-- One other param-->
    <variable tag="var2" scope="local" default_value="2.0"/>
    <!-- Multiple other params-->
    <variable tag="var3" scope="local" default_value="'three'" persistant="YES"/>
</monkeyml>'''

        expected_output = '''\
// No other params
var var1 = 1
// One other param
var var2 = 2.0 (scope = local)
// Multiple other params
var var3 = 'three' (
    scope = local
    persistant = YES
    )
'''

        with self.output(src) as output:
            self.assertEqual(expected_output, output)

    def test_declaration_parameters_and_children(self):
        src = b'''\
<?xml version="1.0"?>
<monkeyml version="1.0">
    <!-- Multiple params, with children-->
    <protocol nsamples="10" sampling_method="samples">
        <!-- Multiple params, without children-->
        <action type="assert" condition="x != 3" message="Not good!" stop_on_failure="YES"/>
        <!-- Single param, with children-->
        <trial nsamples="20">
            <!-- Single param, without children-->
            <list nsamples="30"/>
            <!-- Single param, name omitted, with children-->
            <action type="if" condition="y == 7">
                <!-- Single param, name omitted, without children-->
                <action type="assert" condition="z &lt; 8"/>
                <!-- No params, with children-->
                <action type="else">
                    <!-- No params, without children-->
                    <action type="update_stimulus_display"/>
                </action>
            </action>
        </trial>
    </protocol>
</monkeyml>'''

        expected_output = '''\
// Multiple params, with children
protocol (
    nsamples = 10
    sampling_method = samples
    ) {
    // Multiple params, without children
    assert (
        condition = x != 3
        message = 'Not good!'
        stop_on_failure = YES
        )
    // Single param, with children
    trial (nsamples = 20) {
        // Single param, without children
        list (nsamples = 30)
        // Single param, name omitted, with children
        if (y == 7) {
            // Single param, name omitted, without children
            assert (z < 8)
            // No params, with children
            else {
                // No params, without children
                update_stimulus_display ()
            }
        }
    }
}
'''

        with self.output(src) as output:
            self.assertEqual(expected_output, output)
