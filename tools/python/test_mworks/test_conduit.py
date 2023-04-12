from queue import Queue
import random
import unittest

import numpy

from mworks.conduit import IPCClientConduit, IPCServerConduit
from mworks.data import ReservedEventCode

from . import TypeConversionTestMixin


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

        cls.event_code = random.randrange(max(ReservedEventCode) + 1, 128)
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
        data = self.queue.get(timeout=self.queue_timeout)
        if isinstance(data, Exception):
            raise data
        return data


class TestConduitTypeConversion(ConduitTestMixin,
                                TypeConversionTestMixin,
                                unittest.TestCase):

    @classmethod
    def event_callback(cls, event):
        try:
            data = event.data
        except Exception as e:
            data = e
        cls.queue.put(data, block=False)

    #
    # As of msgpack-cxx 4.1.2, floats are packed as integers if the conversion
    # from one to the other is exact (i.e. lossless):
    # https://github.com/msgpack/msgpack-c/pull/1018
    #

    def test_float(self):
        self.assertReceivedEqualsSent(0.0, 0)
        self.assertReceivedEqualsSent(0.5)
        self.assertReceivedEqualsSent(1.0, 1)
        self.assertReceivedEqualsSent(1.5)
        self.assertReceivedEqualsSent(-2.5)

    def _test_numpy_floating(self, ftype):
        self.assertReceivedEqualsSent(ftype(0.0), 0)
        self.assertReceivedEqualsSent(ftype(0.5), 0.5)
        self.assertReceivedEqualsSent(ftype(1.0), 1)
        self.assertReceivedEqualsSent(ftype(1.5), 1.5)
        self.assertReceivedEqualsSent(ftype(-2.5), -2.5)

        type_info = numpy.finfo(ftype)
        longlong_info = numpy.iinfo(numpy.longlong)
        float_info = numpy.finfo(float)

        if type_info.max <= longlong_info.max:
            self.assertReceivedEqualsSent(ftype(type_info.max),
                                          int(type_info.max))
            self.assertReceivedEqualsSent(ftype(type_info.min),
                                          int(type_info.min))
        elif type_info.max <= float_info.max:
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


class TestConduits(ConduitTestMixin, unittest.TestCase):

    @classmethod
    def event_callback(cls, event):
        cls.queue.put(event, block=False)

    def assertReceived(self, data):
        evt = self.receive()

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
        self.assertReceived(12)

    def test_send_integer(self):
        self.client.send_integer(self.event_code, 12)
        self.assertReceived(12)
        self.client.send_integer(self.event_code, 1.2)
        self.assertReceived(1.2)

    def test_send_object(self):
        data = [1, 2.0, 'three']
        self.client.send_object(self.event_code, data)
        self.assertReceived(data)
