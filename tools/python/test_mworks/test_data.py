from collections import defaultdict
import math
import os
import random
import tempfile
import unittest
import warnings

import numpy

import mworks.data
from mworks.data import ReservedEventCode, MWKFile, _MWKWriter, _MWK2Writer

from . import TypeConversionTestMixin
from .readers.mwk import MWKReader
from .readers.mwk2 import MWK2Reader


class BaseTestMixin:

    @staticmethod
    def init_file(filename):
        return MWKFile(filename)


class MWKTestMixin(BaseTestMixin):

    file_extension = '.mwk'
    file_writer = _MWKWriter


class MWK2TestMixin(BaseTestMixin):

    file_extension = '.mwk2'
    file_writer = _MWK2Writer


class FileTypeConversionTestMixin(TypeConversionTestMixin):

    def setUp(self):
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', RuntimeWarning)
            self.filename = tempfile.mktemp(suffix=self.file_extension)

    def send(self, data):
        fp = self.file_writer(self.filename)
        try:
            fp.write_event(1, 2, data)
        except:
            os.remove(self.filename)
            raise

    def receive(self):
        try:
            with self.init_file(self.filename) as fp:
                evts = fp.get_events()
                self.assertEqual(1, len(evts))
                e = evts[0]
                self.assertEqual(1, e.code)
                self.assertEqual(2, e.time)
                return e.data
        finally:
            try:
                fp.unindex()
            except mworks.data.IndexingException:
                pass
            os.remove(self.filename)

    #
    # Neither Scarab nor SQLite has a boolean type, so bool and numpy.bool_
    # come back as int
    #

    def test_bool(self):
        self.assertReceivedEqualsSent(True, 1)
        self.assertReceivedEqualsSent(False, 0)

    def test_numpy_bool_(self):
        self.assertReceivedEqualsSent(numpy.bool_(True), 1)
        self.assertReceivedEqualsSent(numpy.bool_(False), 0)


class TestMWKFileTypeConversion(MWKTestMixin,
                                FileTypeConversionTestMixin,
                                unittest.TestCase):

    #
    # If a string ends with NUL and contains no other NUL's, the NUL will be
    # stripped in the conversion from Datum to ScarabDatum and back.  This
    # happens because ScarabDatum doesn't distinguish between C strings (i.e.
    # text) and binary data, so we have to use the presence of a single,
    # terminal NUL to identify text strings.
    #

    def test_bytes_with_trailing_nul(self):
        self.assertReceivedEqualsSent(b'foo\0', 'foo')

    def test_unicode_with_trailing_nul(self):
        self.assertReceivedEqualsSent(u'foo\0', 'foo')


class TestMWK2FileTypeConversion(MWK2TestMixin,
                                 FileTypeConversionTestMixin,
                                 unittest.TestCase):

    #
    # SQLite stores NaN as NULL
    #

    def test_float_nan(self):
        self.send(numpy.nan)
        received = self.receive()
        self.assertIsNone(received)


class FileTestMixin:

    def setUp(self):
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', RuntimeWarning)
            self.filename = tempfile.mktemp(suffix=self.file_extension)
        self.fp = self.init_file(self.filename)

    def tearDown(self):
        self.fp.close()  # OK to close even if not opened
        if os.path.exists(self.filename):
            try:
                self.fp.unindex()
            except mworks.data.IndexingException:
                pass
            os.remove(self.filename)

    def create_file(self, events=()):
        fp = self.file_writer(self.filename)
        for code, time, data in events:
            fp.write_event(code, time, data)

    def open_file(self):
        self.fp.open()

    def assertEvent(self, evt, code, time, data):
        self.assertIsInstance(evt.code, int)
        self.assertEqual(code, evt.code)
        self.assertIsInstance(evt.time, int)
        self.assertEqual(time, evt.time)
        self.assertIsInstance(evt.data, type(data))
        self.assertEqual(data, evt.data)

        # Alternative name for data
        self.assertIsInstance(evt.value, type(data))
        self.assertEqual(data, evt.value)


class FileBasicsTestMixin(FileTestMixin):

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

        self.assertRaises(IOError, (lambda: next(self.fp.get_events_iter())))
        self.assertRaises(IOError, self.fp.get_events)

    def test_nonexistent_file(self):
        self.assertRaises(RuntimeError, self.fp.open)

    def test_unreadable_file(self):
        self.create_file()
        os.chmod(self.filename, 0)
        self.assertRaises(RuntimeError, self.open_file)

    def test_already_opened_file(self):
        self.create_file()
        self.open_file()
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


class TestMWKFileBasics(MWKTestMixin, FileBasicsTestMixin, unittest.TestCase):
    pass


class TestMWK2FileBasics(MWK2TestMixin, FileBasicsTestMixin, unittest.TestCase):
    pass


class FileSelectionTestMixin(FileTestMixin):

    def setUp(self):
        super(FileSelectionTestMixin, self).setUp()
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
        for evt, values in zip(selected, expected):
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
        self.assertRaises(RuntimeError, self.select, min_time=23, max_time=22)

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

        self.assertRaises(ValueError, self.select, codes=('a', 'c', 'e'))


class TestMWKFileSelection(MWKTestMixin,
                           FileSelectionTestMixin,
                           unittest.TestCase):
    pass


class TestMWK2FileSelection(MWK2TestMixin,
                            FileSelectionTestMixin,
                            unittest.TestCase):
    pass


class RealFileTestMixin(BaseTestMixin):

    @classmethod
    def setUpClass(cls):
        cls.event_counts = defaultdict(lambda: 0)
        cls.event_times = []

        with cls.file_reader(cls.filename) as reader:
            for evt in reader:
                cls.event_counts[evt[0]] += 1
                cls.event_times.append(evt[1])

        cls.event_counts = dict(cls.event_counts)
        cls.event_times = numpy.array(cls.event_times)

        cls.fp = cls.init_file(cls.filename)

    @classmethod
    def tearDownClass(cls):
        try:
            cls.fp.unindex()
        except mworks.data.IndexingException:
            pass

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
        self.assertEqual(len(self.event_counts) - len(ReservedEventCode),
                         len(codec))

        for c in self.event_counts:
            self.assertTrue((c in codec) or (c in tuple(ReservedEventCode)),
                            'Code %d not in codec' % c)

        reverse_codec = self.fp.reverse_codec
        self.assertIs(reverse_codec, self.fp.reverse_codec)  # Value is cached
        self.assertIsInstance(reverse_codec, dict)
        self.assertEqual(len(reverse_codec), len(codec))

        for code, tagname in codec.items():
            self.assertEqual(code, reverse_codec[tagname])

    def test_select_by_code(self):
        total_count = 0

        for code, expected_count in self.event_counts.items():
            count = self.count(codes=[code])
            self.assertEqual(expected_count, count)
            total_count += count

        self.assertEqual(self.fp.num_events, total_count)

    def test_select_by_tag(self):
        total_count = 0

        for code, tag in self.fp.codec.items():
            count = self.count(codes=[tag])
            self.assertEqual(self.event_counts[code], count)
            total_count += count

        reserved_code_count = sum(self.event_counts[c]
                                  for c in ReservedEventCode)
        self.assertEqual(self.fp.num_events - reserved_code_count,
                         total_count)

    def test_select_multiple_codes(self):
        codes = random.sample(list(self.event_counts.keys()), 5)
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


class TestRealMWKFile(RealFileTestMixin, unittest.TestCase):

    filename = 'data.mwk'
    file_reader = MWKReader


class TestRealMWK2File(RealFileTestMixin, unittest.TestCase):

    filename = 'data.mwk2'
    file_reader = MWK2Reader
