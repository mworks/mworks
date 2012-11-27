import os
import warnings

import numpy

from mworks.data import MWKStream

from test_mworks import unittest, TypeConversionTestMixin


class TestMWKStreamTypeConversion(TypeConversionTestMixin, unittest.TestCase):

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

    def send(self, data):
        self.outstream._write(data)
        self.outstream._flush()

    def receive(self):
        try:
            return self.instream._read()
        except Exception, e:
            return e

    # Scarab doesn't have a boolean type, so bool and numpy.bool_ come
    # back as int

    def test_bool(self):
        self.assertReceivedEqualsSent(True, 1)
        self.assertReceivedEqualsSent(False, 0)

    def test_numpy_bool_(self):
        self.assertReceivedEqualsSent(numpy.bool_(True), 1)
        self.assertReceivedEqualsSent(numpy.bool_(False), 0)
