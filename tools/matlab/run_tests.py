import os
from StringIO import StringIO
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


def run_matlab(arch):
    args = (
        '/Applications/MATLAB_%s.app/bin/matlab' % mw_matlab_version,
        '-' + {'i386': 'maci', 'x86_64': 'maci64'}[arch],
        '-nodisplay',
        '-nojvm',
        )
    cmd = subprocess.Popen(args=args, stdin=subprocess.PIPE)
    cmd.stdin.write(matlab_script)
    cmd.wait()
    print  # Add a newline to the output
    return cmd.returncode


def main():
    status = 0
    for arch in (sys.argv[1:] or ['x86_64']):
        status = run_matlab(arch) or status
    sys.exit(status)


if __name__ == '__main__':
    main()
