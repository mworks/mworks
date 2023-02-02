import os
import sqlite3

import mwk2reader

from . import mw_python_dir, test_data

assert os.path.dirname(mwk2reader.__file__) == mw_python_dir, \
    'Wrong mwk2reader module!'


class BaseTestMixin:

    @staticmethod
    def init_file(filename):
        return mwk2reader.MWKFile(filename)


class TestFileTypeConversion(BaseTestMixin,
                             test_data.TestMWK2FileTypeConversion):
    pass


class TestFileBasics(BaseTestMixin, test_data.TestMWK2FileBasics):
    pass


class TestFileSelection(BaseTestMixin, test_data.TestMWK2FileSelection):

    def test_by_tag(self):
        self.fp._reader.reverse_codec = {'a':1, 'b':2, 'c':3, 'd':4}

        self.assertSelected([0], codes=['a'])
        self.assertSelected([1], codes=['b'])
        self.assertSelected([2], codes=['c'])
        self.assertSelected([3], codes=['d'])

        self.assertSelected([1,2,3], codes=('b', 3, 'd'))


class TestRealFile(BaseTestMixin, test_data.TestRealMWK2File):
    pass
