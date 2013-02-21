import collections
import math
import os
import sys
import unittest
import warnings

import numpy

mw_python_dir = os.environ.get(
    'MW_PYTHON_DIR',
    '/Library/Application Support/MWorks/Scripting/Python',
    )
sys.path.insert(0, mw_python_dir)

import mworks

assert (os.path.dirname(mworks.__file__) ==
        os.path.join(mw_python_dir, 'mworks')), 'Wrong mworks package!'


class TypeConversionTestMixin(object):

    def send(self, data):
        raise NotImplementedError

    def receive(self):
        raise NotImplementedError

    def assertReceivedIsSent(self, sent):
        self.send(sent)
        self.assertIs(sent, self.receive())

    def assertReceivedEqualsSent(self, sent, expected=None):
        if expected is None:
            expected_type = type(sent)
            expected = sent
        elif isinstance(expected, type):
            expected_type = expected
            expected = sent
        else:
            expected_type = type(expected)

        self.send(sent)
        received = self.receive()
        self.assertIsInstance(received, expected_type)
        self.assertEqual(expected, received)

    def assertReceivedIsInf(self, sent):
        self.send(sent)
        received = self.receive()
        self.assertIsInstance(received, float)
        self.assertTrue(math.isinf(received))

    def test_none(self):
        self.assertReceivedIsSent(None)

    def test_bool(self):
        self.assertReceivedIsSent(True)
        self.assertReceivedIsSent(False)

    def test_int(self):
        self.assertReceivedEqualsSent(0)
        self.assertReceivedEqualsSent(1)
        self.assertReceivedEqualsSent(-2)

        int_info = numpy.iinfo(int)
        self.assertReceivedEqualsSent(int_info.max)
        self.assertReceivedEqualsSent(int_info.min)

    def test_long(self):
        self.assertReceivedEqualsSent(0L, int)
        self.assertReceivedEqualsSent(1L, int)
        self.assertReceivedEqualsSent(-2L, int)

        # mw::Datum stores integers as long long's, so we should be
        # able to send the full range of long long values, but not
        # values outside that range
        longlong_info = numpy.iinfo(numpy.longlong)
        self.assertReceivedEqualsSent(longlong_info.max)
        self.assertReceivedEqualsSent(longlong_info.min)
        self.assertRaises(OverflowError, self.send, longlong_info.max+1)
        self.assertRaises(OverflowError, self.send, longlong_info.min-1)

    def test_float(self):
        self.assertReceivedEqualsSent(0.0)
        self.assertReceivedEqualsSent(1.0)
        self.assertReceivedEqualsSent(-2.2)

    def test_float_inf(self):
        self.assertReceivedIsInf(numpy.inf)

    def test_float_nan(self):
        self.send(numpy.nan)
        received = self.receive()
        self.assertIsInstance(received, float)
        self.assertTrue(math.isnan(received))

    def test_str(self):
        self.assertReceivedEqualsSent('')
        self.assertReceivedEqualsSent('foo')
        self.assertReceivedEqualsSent(' Foo \n Bar ')
        self.assertReceivedEqualsSent('foo\0bar')  # Embedded NUL

        # If the string ends with NUL and contains no other NUL's, the
        # NUL will be stripped in the conversion from Datum to
        # PyObject.  This happens because Datum doesn't distinguish
        # between C strings (i.e. text) and binary data, so we have to
        # use the presence of a single, terminal NUL to identify text
        # strings.
        self.assertReceivedEqualsSent('foo\0', 'foo')

    def test_unicode(self):
        self.assertReceivedEqualsSent(u'', str)
        self.assertReceivedEqualsSent(u'foo', str)
        self.assertReceivedEqualsSent(u' Foo \n Bar ', str)
        self.assertReceivedEqualsSent(u'foo\0bar', str)  # Embedded NUL

        # Try some real Unicode
        sent = u'a\U0001d11eb'  # U+1D11E is the G clef character
        self.send(sent)
        received = self.receive()
        self.assertIsInstance(received, str)
        sent_encoded = sent.encode('utf-8')
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', UnicodeWarning)
            self.assertNotEqual(sent, sent_encoded)
        self.assertEqual(sent_encoded, received)
        self.assertEqual(sent, received.decode('utf-8'))

        # See note in test_str about terminating NUL's
        self.assertReceivedEqualsSent(u'foo\0', 'foo')

    def _test_sequence(self, seq_type):
        def test(list_val):
            self.assertReceivedEqualsSent(seq_type(list_val), list_val)
        test([])
        test([1])
        test([1, 2.0, 'three', {'four': 4}])

        # Unconvertible item
        self.assertRaises(TypeError, self.send, seq_type([1, 2, 3, 4j, 5]))

    def test_list(self):
        self._test_sequence(list)
        self.assertReceivedEqualsSent([1, [2, [3, [4, 5]]]])

    def test_tuple(self):
        self._test_sequence(tuple)
        self.assertReceivedEqualsSent((1, (2, (3, (4, 5)))),
                                      [1, [2, [3, [4, 5]]]])

    def _test_mapping(self, map_type):
        def test(dict_val):
            self.assertReceivedEqualsSent(map_type(dict_val), dict_val)
        test({})
        test({'a': 1})
        test({'a': 1, 'b': 2.0, 'c': 'three'})
        test({1: 'a', 2: 'b', 3: ['c', 'd', 'e']})

        # Unconvertible key
        self.assertRaises(TypeError, self.send, {1: 'a', 2: 'b', 3j: 'c'})

        # Unconvertible value
        self.assertRaises(TypeError, self.send, {'a': 1, 'b': 2, 'c': 3j})

    def test_dict(self):
        self._test_mapping(dict)
        self.assertReceivedEqualsSent({'a': {'b': {'c': {'d': 'e'}}}})

    def test_custom_mapping(self):
        class MyMapping(collections.Mapping):
            def __init__(self, d):
                self._d = d
            def __getitem__(self, key):
                return self._d[key]
            def __iter__(self):
                return iter(self._d)
            def __len__(self):
                return len(self._d)

        self.assertFalse(issubclass(MyMapping, dict))
        self._test_mapping(MyMapping)

    def test_unconvertible_object(self):
        self.assertRaises(TypeError, self.send, object())

    def test_bad_dict_key(self):
        # The key (1, 2) is converted on the other end into [1, 2],
        # which is unhashable and can't be used as a key
        self.send({(1, 2): 3})
        self.assertIsInstance(self.receive(), TypeError)

    def test_infinite_recursion(self):
        l = []
        l.append(l)
        self.assertRaises(RuntimeError, self.send, l)

    def test_numpy_bool_(self):
        self.assertReceivedEqualsSent(numpy.bool_(True), True)
        self.assertReceivedEqualsSent(numpy.bool_(False), False)

    def _test_numpy_integer(self, itype):
        type_info = numpy.iinfo(itype)
        longlong_info = numpy.iinfo(numpy.longlong)

        self.assertReceivedEqualsSent(itype(0), 0)
        self.assertReceivedEqualsSent(itype(1), 1)
        if type_info.min < 0:
            self.assertReceivedEqualsSent(itype(-2), -2)

        if type_info.max <= longlong_info.max:
            self.assertReceivedEqualsSent(itype(type_info.max),
                                          int(type_info.max))
        else:
            self.assertReceivedEqualsSent(itype(longlong_info.max),
                                          int(longlong_info.max))
            self.assertRaises(OverflowError,
                              self.send,
                              itype(longlong_info.max+1))

        if type_info.min >= longlong_info.min:
            self.assertReceivedEqualsSent(itype(type_info.min),
                                          int(type_info.min))
        else:
            self.assertReceivedEqualsSent(itype(longlong_info.min),
                                          int(longlong_info.min))
            self.assertRaises(OverflowError,
                              self.send,
                              itype(longong_info.min-1))

    def test_numpy_byte(self):
        self._test_numpy_integer(numpy.byte)

    def test_numpy_short(self):
        self._test_numpy_integer(numpy.short)

    def test_numpy_intc(self):
        self._test_numpy_integer(numpy.intc)

    def test_numpy_int_(self):
        self._test_numpy_integer(numpy.int_)

    def test_numpy_longlong(self):
        self._test_numpy_integer(numpy.longlong)

    def test_numpy_ubyte(self):
        self._test_numpy_integer(numpy.ubyte)

    def test_numpy_ushort(self):
        self._test_numpy_integer(numpy.ushort)

    def test_numpy_uintc(self):
        self._test_numpy_integer(numpy.uintc)

    def test_numpy_uint(self):
        self._test_numpy_integer(numpy.uint)

    def test_numpy_ulonglong(self):
        self._test_numpy_integer(numpy.ulonglong)

    def _test_numpy_floating(self, ftype):
        self.assertReceivedEqualsSent(ftype(0.0), 0.0)
        self.assertReceivedEqualsSent(ftype(1.0), 1.0)
        self.assertReceivedEqualsSent(ftype(-2.2), float(ftype(-2.2)))

        type_info = numpy.finfo(ftype)
        float_info = numpy.finfo(float)

        if type_info.max <= float_info.max:
            self.assertReceivedEqualsSent(ftype(type_info.max),
                                          float(type_info.max))
            self.assertReceivedEqualsSent(ftype(type_info.min),
                                          float(type_info.min))
        else:
            self.assertReceivedEqualsSent(ftype(float_info.max),
                                          float(float_info.max))
            self.assertReceivedIsInf(ftype(float_info.max) * 2.0)
            self.assertReceivedEqualsSent(ftype(float_info.min),
                                          float(float_info.min))
            self.assertReceivedIsInf(ftype(float_info.min) * 2.0)

    @unittest.skipUnless(hasattr(numpy, 'half'), 'numpy.half not found')
    def test_numpy_half(self):
        self._test_numpy_floating(numpy.half)

    def test_numpy_single(self):
        self._test_numpy_floating(numpy.single)

    def test_numpy_float_(self):
        self._test_numpy_floating(numpy.float_)

    def test_numpy_longfloat(self):
        self._test_numpy_floating(numpy.longfloat)

    def test_numpy_str_(self):
        self.assertReceivedEqualsSent(numpy.str_('foo'), str)

    def test_numpy_unicode_(self):
        self.assertReceivedEqualsSent(numpy.unicode_(u'foo'), str)

    def test_numpy_array(self):
        self.assertReceivedEqualsSent(numpy.array([]), [])
        self.assertReceivedEqualsSent(numpy.array([], dtype=numpy.single), [])
        self.assertReceivedEqualsSent(numpy.arange(10, dtype=numpy.ushort),
                                      range(10))
        self.assertReceivedEqualsSent(numpy.arange(10, dtype=numpy.single),
                                      [float(x) for x in xrange(10)])
