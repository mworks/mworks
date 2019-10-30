import glob
import math
import os
import pickle
import shutil
import subprocess
import sys
import tempfile
import warnings

mw_python_dir = os.environ.get(
    'MW_PYTHON_DIR',
    '/Library/Application Support/MWorks/Scripting/Python',
    )
sys.path.insert(0, mw_python_dir)

from mworks.data import _MWKWriter, _MWK2Writer


mw_matlab_dir = os.environ.get(
    'MW_MATLAB_DIR',
    '/Library/Application Support/MWorks/Scripting/MATLAB',
    )


int64_min = -1 << 63
int64_max = -(int64_min + 1)


test_data = (
    ('undefined', None),
    ('bool_true', True),
    ('bool_false', False),
    ('int_zero', 0),
    ('int_pos', 1),
    ('int_neg', -2),
    ('int_max', int64_max),
    ('int_min', int64_min),
    ('float_zero', 0.0),
    ('float_pos', 1.0),
    ('float_neg', -2.2),
    ('float_inf', math.inf),
    ('float_nan', math.nan),
    ('str_empty', ''),
    ('str_text', 'foo bar'),
    ('str_binary', 'foo\0bar'),
    ('list_empty', []),
    ('list_simple', [1]),
    ('list_complex', [1, 2.0, 'three', {'four': 4}]),
    ('list_nested', [1, [2, [3, [4, 5]]]]),
    ('dict_empty', {}),
    ('dict_simple', {'a': 1}),
    ('dict_complex', {'one': 1, 'Two': 2.0, 'ThReE_3': '3.0'}),
    ('dict_int_key', {1: 'a'}),
    ('dict_empty_str_key', {'': 1}),
    ('dict_str_key_starts_with_non_alpha', {'1a': 1}),
    ('dict_str_key_contains_non_alphanum', {'foo.1': 1}),
    ('dict_mixed_keys', {1: 'a', 'two': 2}),
    ('dict_binary_str_key', {'a\0b': 1}),
    )


def create_test_file(extension):
    with warnings.catch_warnings():
        warnings.simplefilter('ignore', RuntimeWarning)
        filename = tempfile.mktemp(suffix=extension)

    fp = (_MWK2Writer if extension == '.mwk2' else _MWKWriter)(filename)

    with open('tests/codec.dat', 'rb') as codec_fp:
        fp.write_event(0, 0, pickle.load(codec_fp))

    tagmap = dict((item[0], i+2) for i, item in enumerate(test_data))
    fp.write_event(1, 1, tagmap)

    code = 2
    for tag, data in test_data:
        fp.write_event(code, code, data)
        code += 1

    return filename


def remove_test_file(filename):
    if os.path.isdir(filename):
        shutil.rmtree(filename)
    else:
        os.remove(filename)


def run_matlab(path, test_file_extension):
    print('\nRunning %s tests for %s' % (test_file_extension, path))
    sys.stdout.flush()
    filename = create_test_file(test_file_extension)
    try:
        args = (
            '%s/bin/matlab' % path,
            '-nodisplay',
            '-nojvm',
            '-r', 'runTests',
            )
        env = os.environ.copy()
        env['MW_MATLAB_DIR'] = mw_matlab_dir
        env['MW_MATLAB_TEST_FILENAME'] = filename
        cmd = subprocess.Popen(args=args, env=env)
        cmd.wait()
        print()  # Add a newline to the output
        return cmd.returncode
    finally:
        remove_test_file(filename)


def main():
    status = 0
    for path in sorted(glob.iglob('/Applications/MATLAB_R*.app')):
        for test_file_extension in ('.mwk', '.mwk2'):
            status = run_matlab(path, test_file_extension) or status
    sys.exit(status)


if __name__ == '__main__':
    main()
