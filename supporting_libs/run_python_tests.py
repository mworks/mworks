import _posixsubprocess
import os
import sys
import unittest


def replace_forbidden_funcs():
    def replace_func(mod, funcname):
        def stub(*args, **kwargs):
            raise unittest.SkipTest('%s is not allowed on iOS' % funcname)
        setattr(mod, funcname, stub)

    replace_func(_posixsubprocess, 'fork_exec')

    for name in ('popen', 'openpty', 'system'):
        replace_func(os, name)
    for name in dir(os):
        if name.startswith(('exec', 'fork', 'spawn')):
            replace_func(os, name)


def run_tests():
    if not hasattr(os, 'system'):
        # Running on iOS.  Replace forbidden functions with test-skipping
        # stubs.
        replace_forbidden_funcs()

    # Run Python test suite
    from test.libregrtest.main import main
    try:
        main()
    except SystemExit:
        pass

    # Run NumPy test suite
    import numpy
    sys.path.append(os.path.join(sys.prefix, 'pytest'))
    numpy.test('full')


if __name__ == '__main__':
    run_tests()
