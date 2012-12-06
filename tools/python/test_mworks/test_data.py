from collections import defaultdict
from itertools import izip
import math
import os
import random
import warnings

import numpy

import mworks.data
from mworks.data import MWKFile, MWKStream, ReservedEventCode
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

    def test_nonexistent_file(self):
        self.assertRaises(IOError, MWKStream.open_file, 'not_a_file.mwk')

    def test_unreadable_file(self):
        filename, fp = self.create_file()
        fp.close()
        os.chmod(filename, 0)
        try:
            self.assertRaises(IOError, MWKStream.open_file, filename)
        finally:
            os.remove(filename)

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
            # Attempt to open already-open stream should fail
            self.assertRaises(IOError, instream.open)

            for i, e in enumerate(instream):
                self.assertEvent(e, *evts[i])
            self.assertEqual(len(evts)-1, i)

            # Once we've hit EOF, read_event should return None
            self.assertIs(instream.read_event(), None)
            self.assertIs(instream.read_event(), None)

        # Read from closed file should fail
        self.assertRaises(IOError, instream._read)


class MWKFileTestMixin(DataTestMixin):

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

    def open_file(self):
        self.fp.open()


class TestMWKFileBasics(MWKFileTestMixin, unittest.TestCase):

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
        self.assertRaises(IOError, self.fp.open)

    def test_unreadable_file(self):
        self.create_file()
        os.chmod(self.filename, 0)
        self.assertRaises(IOError, self.open_file)

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

    def test_basic_selection(self):
        events = (
            [1, 11, 1.0],
            [2, 22, 'two'],
            [3, 33, [1, 2.2, 'three']],
            [4, 44, {'four': [1, 2, 3, 4]}],
            )

        self.create_file(events)
        self.open_file()

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

    def test_bad_event(self):
        events = (
            [1, 11, 1.0],
            [2, 22, 'two'],
            3,
            [4, 44, {'four': [1, 2, 3, 4]}],
            )

        self.create_file(events)
        self.open_file()

        self.assertEqual(len(events)-1, self.fp.num_events)

        for i, evt in izip((0, 1, 3), self.fp.get_events_iter()):
            self.assertEvent(evt, *events[i])


class TestMWKFileSelection(MWKFileTestMixin, unittest.TestCase):

    def setUp(self):
        super(TestMWKFileSelection, self).setUp()
        self.events = (
            [1, 11, 1.0],
            [2, 22, 'two'],
            [3, 33, [1, 2.2, 'three']],
            [4, 44, {'four': [1, 2, 3, 4]}],
            )
        self.all = range(len(self.events))
        self.create_file(self.events)
        self.open_file()

    def assertSelected(self, indices, codes=(), min_time=None, max_time=None):
        expected = [self.events[i] for i in indices]
        selected = self.select(codes, min_time, max_time)

        self.assertEqual(len(indices), len(selected))
        for evt, values in izip(selected, expected):
            self.assertEvent(evt, *values)

    def select(self, codes=(), min_time=None, max_time=None):
        return list(self.fp.get_events_iter(codes = codes,
                                            time_range = (min_time, max_time)))

    def test_by_code(self):
        self.assertSelected([0], codes=(1,))
        self.assertSelected([1], codes=(2,))
        self.assertSelected([2], codes=(3,))
        self.assertSelected([3], codes=(4,))
        self.assertSelected([0, 2], codes=(1, 3))
        self.assertSelected(self.all, codes=(1, 2, 3, 4))

    def test_by_time(self):
        self.assertSelected(self.all, min_time=10)
        self.assertSelected(self.all, min_time=11)
        self.assertSelected([1,2,3], min_time=22)
        self.assertSelected([2,3], min_time=33)
        self.assertSelected([3], min_time=44)
        self.assertSelected([], min_time=45, max_time=46)

        self.assertSelected([], min_time=9, max_time=10)
        self.assertSelected([0], max_time=11)
        self.assertSelected([0,1], max_time=22)
        self.assertSelected([0,1,2], max_time=33)
        self.assertSelected(self.all, max_time=44)
        self.assertSelected(self.all, max_time=45)

        self.assertSelected(self.all, min_time=11, max_time=44)
        self.assertSelected([1,2], min_time=22, max_time=33)
        self.assertSelected([], min_time=23, max_time=32)
        self.assertSelected([1], min_time=22, max_time=22)
        self.assertRaises(IOError, self.select, min_time=23, max_time=22)

    def test_by_code_and_time(self):
        self.assertSelected([1,2,3], codes=(2,3,4), min_time=22)
        self.assertSelected([2,3], codes=(2,3,4), min_time=23)
        self.assertSelected([1,2], codes=(2,3), min_time=22, max_time=33)
        self.assertSelected([1], codes=(2,3), min_time=22, max_time=32)

    def test_by_tag(self):
        self.fp._reverse_codec = {'a':1, 'b':2, 'c':3, 'd':4}

        self.assertSelected([0], codes=['a'])
        self.assertSelected([1], codes=['b'])
        self.assertSelected([2], codes=['c'])
        self.assertSelected([3], codes=['d'])

        self.assertSelected([1,2,3], codes=('b', 3, 'd'))

        self.assertRaises(TypeError, self.select, codes=('a', 'c', 'e'))


def skipUnlessFileExists(filename):
    if not os.path.exists(filename):
        return unittest.skip('Required file %r not found' % filename)
    def filename_setter(test):
        test.filename = filename
        return test
    return filename_setter


@skipUnlessFileExists('data.mwk')
class TestRealMWKFile(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.event_counts = defaultdict(lambda: 0)
        cls.event_times = []

        with MWKStream.open_file(cls.filename) as stream:
            for evt in stream:
                cls.event_counts[evt.code] += 1
                cls.event_times.append(evt.time)

        cls.event_counts = dict(cls.event_counts)
        cls.event_times = numpy.array(cls.event_times)

        cls.fp = MWKFile(cls.filename)

    @classmethod
    def tearDownClass(cls):
        cls.fp.unindex()

    def setUp(self):
        self.fp.open()

    def tearDown(self):
        # Close file at the end of each test so subsequent tests will
        # exercise re-opening a previously-indexed file
        self.fp.close()

    def count(self, codes=(), min_time=None, max_time=None):
        return sum(1 for _ in
                   self.fp.get_events_iter(codes=codes,
                                           time_range=(min_time, max_time)))

    def test_codec(self):
        codec_code = ReservedEventCode.RESERVED_CODEC_CODE
        self.assertEqual(1, self.event_counts[codec_code])

        codec = self.fp.codec
        self.assertIs(codec, self.fp.codec)  # Value is cached
        self.assertIsInstance(codec, dict)
        self.assertTrue(len(codec) > 0)
        self.assertEqual(len(self.event_counts) - len(ReservedEventCode.values),
                         len(codec))

        for c in self.event_counts:
            self.assertTrue((c in codec) or (c in ReservedEventCode.values),
                            'Code %d not in codec' % c)

        reverse_codec = self.fp.reverse_codec
        self.assertIs(reverse_codec, self.fp.reverse_codec)  # Value is cached
        self.assertIsInstance(reverse_codec, dict)
        self.assertEqual(len(reverse_codec), len(codec))

        for code, tagname in codec.iteritems():
            self.assertEqual(code, reverse_codec[tagname])

    def test_select_by_code(self):
        total_count = 0

        for code, expected_count in self.event_counts.iteritems():
            count = self.count(codes=[code])
            self.assertEqual(expected_count, count)
            total_count += count

        self.assertEqual(self.fp.num_events, total_count)

    def test_select_by_tag(self):
        total_count = 0

        for code, tag in self.fp.codec.iteritems():
            count = self.count(codes=[tag])
            self.assertEqual(self.event_counts[code], count)
            total_count += count

        reserved_code_count = sum(self.event_counts[c]
                                  for c in ReservedEventCode.values)
        self.assertEqual(self.fp.num_events - reserved_code_count,
                         total_count)

    def test_select_multiple_codes(self):
        codes = random.sample(self.event_counts.keys(), 5)
        expected_count = sum(self.event_counts[c] for c in codes)
        self.assertEqual(expected_count, self.count(codes=codes))

    def test_select_by_time(self):
        median = numpy.median(self.event_times)
        if isinstance(median, numpy.floating):
            median_low = int(math.floor(median))
            median_high = int(math.ceil(median))
        else:
            median_low = median
            median_high = median

        self.assertEqual(len(numpy.where(self.event_times >= median_high)[0]),
                         self.count(min_time=median_high))

        self.assertEqual(len(numpy.where(self.event_times <= median_low)[0]),
                         self.count(max_time=median_low))
