import os
import warnings

import numpy

import mworks.data
from mworks.data import MWKFile, MWKStream
from mworks._mworks import EventWrapper

from test_mworks import unittest, TypeConversionTestMixin


class DataTestMixin(object):

    def assertEvent(self, evt, code, time, value):
        self.assertIsInstance(evt, EventWrapper)
        self.assertFalse(evt.empty)
        self.assertIsInstance(evt.code, int)
        self.assertEqual(code, evt.code)
        self.assertIsInstance(evt.time, (int, long))
        self.assertEqual(time, evt.time)
        self.assertIsInstance(evt.value, type(value))
        self.assertEqual(value, evt.value)

        # Alternative name for value
        self.assertIsInstance(evt.data, type(evt.value))
        self.assertEqual(evt.value, evt.data)


class MWKStreamTestMixin(DataTestMixin):

    @staticmethod
    def create_file():
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', RuntimeWarning)
            filename = os.tempnam()
        fp = MWKStream._create_file(filename)
        return filename, fp

    @classmethod
    def setUpClass(cls):
        cls.filename, cls.outstream = cls.create_file()
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

    def read_event(self):
        return self.instream.read_event()

    def write_event(self, evt):
        self.outstream._write_event(evt)
        self.outstream._flush()

    def test_basic(self):
        code = 123
        time = 456
        value = {'a': 1.0, 'b': [None, 2.2]}

        # Write a "raw" event
        self.write([code, time, value])
        evt = self.read_event()
        self.assertEvent(evt, code, time, value)

        # Write an EventWrapper instance
        self.write_event(evt)
        evt2 = self.read_event()
        self.assertIsNot(evt2, evt)
        self.assertEvent(evt2, code, time, value)

    def test_no_payload(self):
        code = 123
        time = 456

        # Write a "raw" event
        self.write([code, time])
        evt = self.read_event()
        self.assertEvent(evt, code, time, None)

        # Write an EventWrapper instance
        self.write_event(evt)
        evt2 = self.read_event()
        self.assertIsNot(evt2, evt)
        self.assertEvent(evt2, code, time, None)

    def test_bad_event(self):
        def test(e):
            self.write(e)
            self.assertRaises(IOError, self.instream.read_event)

        test(1)  # Not a list
        test(None)  # Not a list
        test([1])  # Too few elements
        test([1,2,3,4])  # Too many elements
        test([1.0, 2, 3])  # Bad code
        test([1, 'two', 3])  # Bad time

    def test_iteration_and_context_management(self):
        evts = [
            [1, 11, 'one'],
            [2, 22, 'two'],
            [3, 33, 'three'],
            [4, 44, 'four'],
            ]

        filename, outstream = self.create_file()
        with outstream:
            for e in evts:
                outstream._write(e)

        # Write to closed file should fail
        self.assertRaises(IOError, outstream._write, evts[0])

        with MWKStream.open_file(filename) as instream:
            for i, e in enumerate(instream):
                self.assertEvent(e, *evts[i])
            self.assertEqual(len(evts)-1, i)
            # Once we've hit EOF, read_event should return None
            self.assertIs(instream.read_event(), None)
            self.assertIs(instream.read_event(), None)

        # Read from closed file should fail
        self.assertRaises(IOError, instream._read)


class TestMWKFile(DataTestMixin, unittest.TestCase):

    def setUp(self):
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', RuntimeWarning)
            self.filename = os.tempnam() + '.mwk'
        self.fp = MWKFile(self.filename)

    def tearDown(self):
        self.fp.close()  # OK to close even if not opened
        if os.path.exists(self.filename):
            try:
                self.fp.unindex()
            except mworks.data.IndexingException:
                pass
            os.remove(self.filename)

    def create_file(self, events=()):
        with MWKStream._create_file(self.filename) as fp:
            for evt in events:
                fp._write(evt)

    def test_unopened_file(self):
        self.assertIs(False, self.fp.exists)
        self.assertIs(False, self.fp.loaded)
        self.assertIs(False, self.fp.valid)

        self.assertRaises(IOError, (lambda: self.fp.num_events))
        self.assertRaises(IOError, (lambda: self.fp.minimum_time))
        self.assertRaises(IOError, (lambda: self.fp.maximum_time))

        self.assertEqual(self.filename, self.fp.file)

        self.assertRaises(IOError, (lambda: self.fp.codec))
        self.assertRaises(IOError, (lambda: self.fp.reverse_codec))

        self.assertRaises(IOError, self.fp.get_events_iter().next)
        self.assertRaises(IOError, self.fp.get_events)
        self.assertRaises(IOError, self.fp.reindex)
        self.assertRaises(IOError, self.fp.unindex)

    def test_nonexistent_file(self):
        self.assertRaises(RuntimeError, self.fp.open)

    def test_context_manager(self):
        self.assertFalse(self.fp.exists)

        self.create_file()
        
        self.assertTrue(self.fp.exists)
        self.assertFalse(self.fp.loaded)
        self.assertFalse(self.fp.valid)

        with self.fp:
            self.assertTrue(self.fp.loaded)
            self.assertTrue(self.fp.valid)

        self.assertFalse(self.fp.loaded)
        self.assertFalse(self.fp.valid)

    def test_simple(self):
        events = (
            [1, 11, 1.0],
            [2, 22, 'two'],
            [3, 33, [1, 2.2, 'three']],
            [4, 44, {'four': [1, 2, 3, 4]}],
            )

        self.create_file(events)

        with self.fp:
            self.assertEqual(len(events), self.fp.num_events)
            self.assertEqual(11, self.fp.minimum_time)
            self.assertEqual(44, self.fp.maximum_time)

            # Iterating through events one by one
            for i, evt in enumerate(self.fp.get_events_iter()):
                self.assertEvent(evt, *events[i])
            self.assertEqual(len(events)-1, i)

            # All events in a single array
            for i, evt in enumerate(self.fp.get_events()):
                self.assertEvent(evt, *events[i])
            self.assertEqual(len(events)-1, i)
