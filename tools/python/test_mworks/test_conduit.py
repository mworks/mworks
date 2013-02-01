from Queue import Queue
import random

from mworks.conduit import IPCClientConduit, IPCServerConduit
from mworks._mworks import Event

from test_mworks import unittest, TypeConversionTestMixin


class ConduitTestMixin(object):

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

    def send(self, data):
        self.client.send_data(self.event_code, data)

    def receive(self):
        return self.queue.get(timeout=self.queue_timeout)


class TestConduitTypeConversion(ConduitTestMixin,
                                TypeConversionTestMixin,
                                unittest.TestCase):

    @classmethod
    def event_callback(cls, event):
        try:
            data = event.data
        except Exception, e:
            data = e
        cls.queue.put(data, block=False)


class TestConduits(ConduitTestMixin, unittest.TestCase):

    @classmethod
    def event_callback(cls, event):
        cls.queue.put(event, block=False)

    def assertReceived(self, data):
        evt = self.receive()
        self.assertIsInstance(evt, Event)

        self.assertIsInstance(evt.code, int)
        self.assertEqual(self.event_code, evt.code)

        self.assertIsInstance(evt.time, int)

        self.assertIsInstance(evt.data, type(data))
        self.assertEqual(data, evt.data)

        # Alternative name for data
        self.assertIsInstance(evt.value, type(data))
        self.assertEqual(data, evt.value)

    def test_initialization(self):
        self.assertIsInstance(self.server, self.server_class)
        self.assertIsInstance(self.client, self.client_class)

        self.assertTrue(self.server.initialized)
        self.assertTrue(self.client.initialized)

    def test_basic_send(self):
        data = {'a': 1, 'b': 2.2}
        self.send(data)
        self.assertReceived(data)

    def test_send_float(self):
        self.client.send_float(self.event_code, 1.2)
        self.assertReceived(1.2)
        self.client.send_float(self.event_code, 12)
        self.assertReceived(12.0)

    def test_send_integer(self):
        self.client.send_integer(self.event_code, 12)
        self.assertReceived(12)
        self.assertRaises(TypeError,
                          self.client.send_integer,
                          self.event_code,
                          1.2)

    def test_send_object(self):
        data = [1, 2.0, 'three']
        self.client.send_object(self.event_code, data)
        self.assertReceived(data)
