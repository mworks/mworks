import os
import sys

mw_python_dir = os.environ.get(
    'MW_PYTHON_DIR',
    '/Library/Application Support/MWorks/Scripting/Python',
    )
sys.path.insert(0, mw_python_dir)

import mworks

assert (os.path.dirname(mworks.__file__) ==
        os.path.join(mw_python_dir, 'mworks')), 'Wrong mworks package!'
