import sys
try:
    import unittest2 as unittest
except ImportError:
    import unittest


def main():
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
