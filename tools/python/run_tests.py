import sys
import unittest


def main():
    print('Running tests for Python %d.%d' % sys.version_info[:2],
          file = sys.stderr)

    if len(sys.argv) == 1:
        tests = unittest.defaultTestLoader.discover('.')
    else:
        tests = unittest.defaultTestLoader.loadTestsFromNames(sys.argv[1:])

    suite = unittest.TestSuite(tests)
    runner = unittest.TextTestRunner()
    result = runner.run(suite)
    sys.exit(not result.wasSuccessful())


if __name__ == '__main__':
    if not sys.warnoptions:
        import os, warnings
        warnings.simplefilter('error')
        os.environ['PYTHONWARNINGS'] = 'error'
    main()
