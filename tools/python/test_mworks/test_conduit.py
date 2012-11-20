import Queue
import random
try:
    import unittest2 as unittest
except ImportError:
    import unittest

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
        cls.queue = Queue.Queue(maxsize=1)
        cls.server.register_callback_for_code(cls.event_code,
                                              cls.event_callback)

    @classmethod
    def tearDownClass(cls):
        cls.client.finalize()
        cls.server.finalize()

    @classmethod
    def event_callback(cls, event):
        cls.queue.put(event.data, block=False)

    def send(self, data):
        self.client.send_data(self.event_code, data)

    def receive(self):
        return self.queue.get(timeout=self.queue_timeout)

    def assertReceivedIsSent(self, sent):
        self.send(sent)
        self.assertIs(sent, self.receive())

    def assertReceivedEqualsSent(self, sent, expected_type=None):
        if expected_type is None:
            expected_type = type(sent)
        self.send(sent)
        received = self.receive()
        self.assertIsInstance(received, expected_type)
        self.assertEqual(sent, received)

    def test_initialization(self):
        self.assertIsInstance(self.server, self.server_class)
        self.assertIsInstance(self.client, self.client_class)

        self.assertTrue(self.server.initialized)
        self.assertTrue(self.client.initialized)

    def test_none(self):
        self.assertReceivedIsSent(None)

    def test_boolean(self):
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
