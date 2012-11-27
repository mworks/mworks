from Queue import Queue
import random

from mworks.conduit import IPCClientConduit, IPCServerConduit

from test_mworks import unittest, TypeConversionTestMixin


class TestConduitTypeConversion(TypeConversionTestMixin, unittest.TestCase):

    # boost::serialization apparently doesn't support serialization of
    # inf/nan to text archives
    can_convert_inf = False
    can_convert_nan = False

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

    def test_initialization(self):
        self.assertIsInstance(self.server, self.server_class)
        self.assertIsInstance(self.client, self.client_class)

        self.assertTrue(self.server.initialized)
        self.assertTrue(self.client.initialized)
