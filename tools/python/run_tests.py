from glob import iglob
import os
import sys
try:
    import unittest2 as unittest
except ImportError:
    import unittest


def main():
    # Remove any files previously created by Boost.Interprocess, as these
    # sometimes cause tests to fail spuriously
    for path in iglob('/tmp/boost_interprocess/mworks_conduit_test_*'):
        if os.path.isfile(path):
            print >>sys.stderr, 'Removing', path
            os.remove(path)

    if len(sys.argv) == 1:
        tests = unittest.defaultTestLoader.discover('.')
    else:
        tests = unittest.defaultTestLoader.loadTestsFromNames(sys.argv[1:])

    suite = unittest.TestSuite(tests)
    runner = unittest.TextTestRunner()
    result = runner.run(suite)
    sys.exit(not result.wasSuccessful())


if __name__ == '__main__':
    main()
