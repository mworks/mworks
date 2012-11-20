try:
    import unittest2 as unittest
except ImportError:
    import unittest


def run_tests():
    tests = unittest.defaultTestLoader.discover('.')
    suite = unittest.TestSuite(tests)
    runner = unittest.TextTestRunner()
    result = runner.run(suite)
    return result.wasSuccessful()


if __name__ == '__main__':
    import sys
    sys.exit(not run_tests())
