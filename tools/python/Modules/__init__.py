import hashlib
import plistlib
import sys

if sys.version_info[:2] < (3, 8):
    raise ImportError('mworks requires Python 3.8 or later')


def _get_version():
    with open('/Library/Frameworks/MWorksCore.framework/Resources/Info.plist',
              'rb') as fp:
        plist = plistlib.load(fp)
    return plist['CFBundleShortVersionString'], plist['CFBundleVersion']


def get_version():
    def parse_value(value):
        for x in value.split('.'):
            try:
                yield int(x)
            except ValueError:
                # Stop parsing when we reach a non-numeric component
                # (e.g. ".dev")
                break
    version, build_date = _get_version()
    return tuple(parse_value(version)), tuple(parse_value(build_date))


def require_version(min_version=(0, 0), min_build_date=(0, 0, 0)):
    version, build_date = get_version()
    assert version >= min_version, 'MWorks version < %s' % (min_version,)
    assert build_date >= min_build_date, ('MWorks build date < %s' %
                                          (min_build_date,))


def file_hash(filename):
    with open(filename, 'rb') as fp:
        return hashlib.sha1(fp.read()).hexdigest()


__version__ = _get_version()[0]
