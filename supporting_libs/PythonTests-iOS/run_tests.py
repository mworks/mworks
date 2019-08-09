import _posixsubprocess
import os
import unittest


def run_tests():
    def forbid_func(mod, funcname):
        def stub(*args, **kwargs):
            raise unittest.SkipTest('%s is not allowed on iOS' % funcname)
        setattr(mod, funcname, stub)

    forbid_func(_posixsubprocess, 'fork_exec')
    for name in ('popen', 'openpty', 'system'):
        forbid_func(os, name)
    for name in dir(os):
        if name.startswith(('exec', 'fork', 'spawn')):
            forbid_func(os, name)

    from test.libregrtest.main import main
    try:
        main()
    except SystemExit:
        pass


if __name__ == '__main__':
    run_tests()
