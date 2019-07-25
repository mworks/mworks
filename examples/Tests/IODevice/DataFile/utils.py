import glob
import os
import tempfile


def get_base_name():
    return os.path.join(tempfile.gettempdir(), 'mworks_data_file_test-')


def remove_previous_files():
    for filename in glob.glob(get_base_name() + '*.mwk2'):
        message('Removing file "%s"' % filename)
        os.remove(filename)


# Remove any previous files, so that nightly automated tests don't cause a
# ton of test files to accumulate
remove_previous_files()
