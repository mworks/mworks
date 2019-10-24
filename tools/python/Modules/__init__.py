import hashlib
import sys
if sys.version_info[:2] < (3, 5):
    raise ImportError('mworks requires Python 3.5 or later')


def file_hash(filename):
    with open(filename, 'rb') as fp:
        return hashlib.sha1(fp.read()).hexdigest()
