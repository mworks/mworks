from __future__ import print_function
from glob import iglob
import os
import sys
import unittest


def main():
    print('Running tests for Python %d.%d' % sys.version_info[:2],
          file = sys.stderr)

    # Remove any files previously created by Boost.Interprocess, as these
    # sometimes cause tests to fail spuriously
    for path in iglob('/tmp/boost_interprocess/mworks_conduit_test_*'):
        if os.path.isfile(path):
            print('Removing', path, file=sys.stderr)
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
