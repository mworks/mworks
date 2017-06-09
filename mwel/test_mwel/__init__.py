from __future__ import division, print_function, unicode_literals
from contextlib import contextmanager
import io
import os
import shutil
import tempfile

from mwel import ErrorLogger


class ErrorLoggerMixin(object):

    def setUp(self):
        super(ErrorLoggerMixin, self).setUp()
        self.error_logger = ErrorLogger()

    def with_error_check(self, func):
        @contextmanager
        def func_wrapper(*args, **kwargs):
            yield func(*args, **kwargs)
            self.assertNoErrors()
        return func_wrapper

    def assertNoErrors(self):
        if self.error_logger.errors:
            msg = io.StringIO()
            msg.write('input contained unexpected errors:\n')
            self.error_logger.print_errors(msg)
            self.fail(msg.getvalue())

    def assertError(self, msg=None, token=None, lineno=None, colno=None,
                    filename=''):
        if not self.error_logger.errors:
            self.fail('expected error was not detected')

        e = self.error_logger.errors.popleft()
        self.assertEqual(5, len(e))

        if msg:
            self.assertEqual(msg, e.msg)
        elif token:
            self.assertEqual('Invalid syntax at %r' % str(token), e.msg)

        if token:
            self.assertEqual(token, e.token)

        if lineno is not None:
            self.assertEqual(lineno, e.lineno)

        if colno is not None:
            self.assertEqual(colno, e.colno)

        self.assertEqual(filename, e.filename)


class TempFilesMixin(object):

    def setUp(self):
        super(TempFilesMixin, self).setUp()
        self.tmpdir = tempfile.mkdtemp()

    def tearDown(self):
        shutil.rmtree(self.tmpdir)
        super(TempFilesMixin, self).tearDown()

    def write_file(self, filepath, src):
        filepath = os.path.join(self.tmpdir, filepath)
        dirpath = os.path.dirname(filepath)
        if not os.path.isdir(dirpath):
            os.makedirs(dirpath)
        with open(filepath, 'w') as fp:
            fp.write(src)
        return filepath
