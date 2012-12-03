import os
import warnings

import numpy

from mworks.data import MWKStream
from mworks._mworks import EventWrapper

from test_mworks import unittest, TypeConversionTestMixin


class MWKStreamTestMixin(object):

    @classmethod
    def setUpClass(cls):
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', RuntimeWarning)
            cls.filename = os.tempnam()
        cls.outstream = MWKStream._create_file(cls.filename)
        cls.instream = MWKStream.open_file(cls.filename)

    @classmethod
    def tearDownClass(cls):
        cls.instream.close()
        cls.outstream.close()
        os.remove(cls.filename)

    def write(self, data):
        self.outstream._write(data)
        self.outstream._flush()

    def read(self):
        try:
            return self.instream._read()
        except Exception, e:
            return e

    send = write
    receive = read


class TestMWKStreamTypeConversion(MWKStreamTestMixin,
                                  TypeConversionTestMixin,
                                  unittest.TestCase):

    # Scarab doesn't have a boolean type, so bool and numpy.bool_ come
    # back as int

    def test_bool(self):
        self.assertReceivedEqualsSent(True, 1)
        self.assertReceivedEqualsSent(False, 0)

    def test_numpy_bool_(self):
        self.assertReceivedEqualsSent(numpy.bool_(True), 1)
        self.assertReceivedEqualsSent(numpy.bool_(False), 0)


class TestMWKStreamEventIO(MWKStreamTestMixin, unittest.TestCase):

    def write_event(self, evt):
        self.outstream._write_event(evt)
        self.outstream._flush()

    def assertEvent(self, code, time, value):
        evt = self.instream.read_event()

        self.assertIsInstance(evt, EventWrapper)
        self.assertFalse(evt.empty)
        self.assertIsInstance(evt.code, int)
        self.assertEqual(code, evt.code)
        self.assertIsInstance(evt.time, long)
        self.assertEqual(time, evt.time)
        self.assertIsInstance(evt.value, type(value))
        self.assertEqual(value, evt.value)

        # Alternative name for value
        self.assertIsInstance(evt.data, type(evt.value))
        self.assertEqual(evt.value, evt.data)

        return evt

    def test_basic(self):
        code = 123
        time = 456
        value = {'a': 1.0, 'b': [None, 2.2]}

        # Write a "raw" event
        self.write([code, time, value])
        evt = self.assertEvent(code, time, value)

        # Write an EventWrapper instance
        self.write_event(evt)
        self.assertEvent(code, time, value)

    def test_no_payload(self):
        code = 123
        time = 456

        # Write a "raw" event
        self.write([code, time])
        evt = self.assertEvent(code, time, None)

        # Write an EventWrapper instance
        self.write_event(evt)
        self.assertEvent(code, time, None)

    def test_bad_event(self):
        def test(e):
            self.write(e)
            self.assertRaises(ValueError, self.instream.read_event)

        test(1)  # Not a list
        test(None)  # Not a list
        test([1])  # Too few elements
        test([1,2,3,4])  # Too many elements
        test([1.0, 2, 3])  # Bad code
        test([1, 'two', 3])  # Bad time
