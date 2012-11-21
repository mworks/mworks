import collections
import math
from Queue import Queue
import random
try:
    import unittest2 as unittest
except ImportError:
    import unittest
import warnings

import numpy

from mworks.conduit import IPCClientConduit, IPCServerConduit


class TestConduitTypeConversion(unittest.TestCase):

    server_class = IPCServerConduit
    client_class = IPCClientConduit
    resource_name = 'mworks_conduit_test'
    queue_timeout = 5

    @classmethod
    def setUpClass(cls):
        cls.server = cls.server_class(cls.resource_name)
        cls.server.initialize()
        
        cls.client = cls.client_class(cls.resource_name)
        cls.client.initialize()

        cls.event_code = random.randrange(128)
        cls.queue = Queue(maxsize=1)
        cls.server.register_callback_for_code(cls.event_code,
                                              cls.event_callback)

    @classmethod
    def tearDownClass(cls):
        cls.client.finalize()
        cls.server.finalize()

    @classmethod
    def event_callback(cls, event):
        try:
            data = event.data
        except Exception, e:
            data = e
        cls.queue.put(data, block=False)

    def send(self, data):
        self.client.send_data(self.event_code, data)

    def receive(self):
        return self.queue.get(timeout=self.queue_timeout)

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

    def test_initialization(self):
        self.assertIsInstance(self.server, self.server_class)
        self.assertIsInstance(self.client, self.client_class)

        self.assertTrue(self.server.initialized)
        self.assertTrue(self.client.initialized)

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

    # boost::serialization apparently doesn't support serialization of
    # inf/nan to text archives, so we expect the next two tests to
    # fail

    @unittest.expectedFailure
    def test_float_inf(self):
        self.send(numpy.inf)
        self.assertTrue(math.isinf(self.receive()))

    @unittest.expectedFailure
    def test_float_nan(self):
        self.send(numpy.nan)
        self.assertTrue(math.isnan(self.receive()))

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
        # This should always raise RuntimeError, but under Python 2.6
        # it raises TypeError.  Hopefully this means the runaway
        # recursion is being caught (and possible misidentified)
        # somewhere else in the Python interpreter, but it may be a
        # bug.
        self.assertRaises((RuntimeError, TypeError), self.send, l)
