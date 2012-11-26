import os
import warnings

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

    def test_bool(self):
        # Scarab doesn't have a boolean type, so bool comes back as int
        self.assertReceivedEqualsSent(True, 1)
        self.assertReceivedEqualsSent(False, 0)
