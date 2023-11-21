import _posixsubprocess
import os
import posix
import sys
import tempfile
import unittest


def replace_forbidden_funcs():
    def install_stub_func(mod, funcname):
        def stub(*args, **kwargs):
            raise unittest.SkipTest('%s is not allowed on iOS' % funcname)
        setattr(mod, funcname, stub)

    def replace_func(mod, funcname):
        if hasattr(mod, funcname):
            install_stub_func(mod, funcname)

    replace_func(_posixsubprocess, 'fork_exec')

    # Some tests assume os.system is present, so always install a stub
    install_stub_func(os, 'system')

    for mod in (os, posix):
        for name in ('openpty', 'popen'):
            replace_func(mod, name)
        for name in dir(mod):
            if name.startswith(('exec', 'fork', 'posix_spawn', 'spawn')):
                install_stub_func(mod, name)


def run_tests():
    if not hasattr(os, 'system'):
        # Running on iOS.  Replace forbidden functions with test-skipping
        # stubs.
        replace_forbidden_funcs()

    # Run Python test suite
    from test.libregrtest import main
    try:
        main()
    except SystemExit:
        pass

    # Run NumPy test suite
    import numpy
    sys.path.append(os.path.join(sys.prefix, 'numpy_test'))
    with tempfile.TemporaryDirectory() as tmpdir:
        origdir = os.getcwd()
        os.chdir(tmpdir)
        try:
            numpy.test('full', extra_argv=['--color=no'])
        finally:
            os.chdir(origdir)


if __name__ == '__main__':
    run_tests()
