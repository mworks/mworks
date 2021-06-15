import subprocess
import sys
import time

if 'mworkscore' not in sys.modules:
    # This is an external conduit process.  Ensure that the MWorks Python
    # package is importable.
    mw_python_path = '/Library/Application Support/MWorks/Scripting/Python'
    if mw_python_path not in sys.path:
        sys.path.insert(0, mw_python_path)
    del mw_python_path


resource_name = 'serverside_conduit_test'


def start(script_path):
    global client_conduit_process

    client_conduit_process = subprocess.Popen([
        '/usr/local/bin/python3',
        script_path,
        resource_name,
        ],
        stdout = subprocess.PIPE,
        stderr = subprocess.PIPE,
        text = True,
        )

    time.sleep(1)


def finish():
    global client_conduit_process

    rc = client_conduit_process.wait(5)
    stdout = client_conduit_process.stdout.read().strip()
    if stdout:
        message('Client conduit process produced output:\n' + stdout)
    stderr = client_conduit_process.stderr.read().strip()
    if stderr:
        error('Client conduit process produced errors:\n' + stderr)
    if rc != 0:
        error('Client conduit process exited with status code %d' % rc)

    del client_conduit_process
