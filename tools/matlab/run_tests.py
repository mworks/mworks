import os
import shutil
import subprocess
import sys
import warnings

from mworks.data import MWKStream


mw_developer_dir = os.environ.get(
    'MW_DEVELOPER_DIR',
    '/Library/Application Support/MWorks/Developer',
    )

mw_matlab_dir = os.environ.get(
    'MW_MATLAB_DIR',
    '/Library/Application Support/MWorks/Scripting/Matlab',
    )

mw_matlab_version = os.environ.get('MW_MATLAB_VERSION', 'R2010a')


matlab_script = '''\
try
    addpath('%s/MATLAB/xunit')
    addpath('%s')
    cd tests
    passed = runtests;
catch ME
    disp(ME.getReport)
    passed = false;
end
quit(~passed)
''' % (mw_developer_dir, mw_matlab_dir)


def create_test_file():
    with warnings.catch_warnings():
        warnings.simplefilter('ignore', RuntimeWarning)
        filename = os.tempnam()

    with MWKStream._create_file(filename) as fp:
        code = 1
        tagmap = {}

        for tag, data in (
                ('undefined', None),
                ('int_zero', 0),
                ('int_pos', 1),
                ('int_neg', -2),
                ):
            tagmap[tag] = code
            fp._write([code, code, data])
            code += 1

        fp._write([0, 0, tagmap])

    return filename


def remove_test_file(filename):
    if os.path.isdir(filename):
        shutil.rmtree(filename)
    else:
        os.remove(filename)


def run_matlab(arch):
    filename = create_test_file()
    try:
        env = os.environ.copy()
        env['MW_MATLAB_TEST_FILENAME'] = filename
        args = (
            '/Applications/MATLAB_%s.app/bin/matlab' % mw_matlab_version,
            '-' + {'i386': 'maci', 'x86_64': 'maci64'}[arch],
            '-nodisplay',
            '-nojvm',
            )
        cmd = subprocess.Popen(args=args, stdin=subprocess.PIPE, env=env)
        cmd.stdin.write(matlab_script)
        cmd.wait()
        print  # Add a newline to the output
        return cmd.returncode
    finally:
        remove_test_file(filename)


def main():
    status = 0
    for arch in (sys.argv[1:] or ['x86_64']):
        status = run_matlab(arch) or status
    sys.exit(status)


if __name__ == '__main__':
    main()
