import os
import py_compile
import shutil
import tempfile
import zipfile


# If we're building for macOS, copy the Python files from the build directory
# for the native architecture.  Otherwise, copy them from the directory for the
# first architecture in ARCHS.
if os.environ['PLATFORM_NAME'] == 'macosx':
    arch_to_use = os.environ['NATIVE_ARCH']
else:
    arch_to_use = os.environ['ARCHS'].split()[0]

stdlib_path = os.path.join(os.environ['BUILT_PRODUCTS_DIR'],
                           arch_to_use,
                           'lib',
                           'python' + os.environ['MW_PYTHON_3_VERSION'])

zipfile_path = os.path.join(os.environ['BUILT_PRODUCTS_DIR'],
                            os.environ['UNLOCALIZED_RESOURCES_FOLDER_PATH'],
                            'python.zip')

cacert_file = 'cacert.pem'
cacert_path = os.path.join(os.environ['BUILT_PRODUCTS_DIR'],
                           os.environ['UNLOCALIZED_RESOURCES_FOLDER_PATH'],
                           cacert_file)


def add_file(fp, path):
    with tempfile.NamedTemporaryFile() as tmp:
        py_compile.compile(path,
                           cfile = tmp.name,
                           dfile = os.path.join(os.path.basename(zipfile_path),
                                                path),
                           doraise = True,
                           optimize = 0)
        fp.write(tmp.name, path[:-3] + '.pyc')


def add_files(fp):
    for dirpath, dirnames, filenames in os.walk('.'):
        dirnames_to_remove = []

        for name in dirnames:
            if (name in ('idle_test', 'test', 'tests') or
                not os.path.isfile(os.path.join(dirpath, name, '__init__.py'))):
                dirnames_to_remove.append(name)

        for name in dirnames_to_remove:
            dirnames.remove(name)

        for name in filenames:
            path = os.path.join(dirpath, name)[2:]  # Remove leading ./
            if name.endswith('.py'):
                add_file(fp, path)


# If the ZIP file already exists, don't overrwrite it, as this will break code
# signing on iOS
if not os.path.isfile(zipfile_path):
    with zipfile.ZipFile(zipfile_path, 'w', zipfile.ZIP_DEFLATED) as fp:
        os.chdir(stdlib_path)
        add_files(fp)

        os.chdir(os.path.join(stdlib_path, 'site-packages'))
        add_files(fp)

        os.chdir(os.path.join(os.environ['SRCROOT'], 'MWorksPython'))
        add_file(fp, 'mworkspython.py')
        # This is actually part of the Python plugin, but it simplifies
        # packaging to include it here
        add_file(fp, 'mworkscore.py')

# Ditto for root certificates file
if not os.path.isfile(cacert_path):
    shutil.copyfile(os.path.join(stdlib_path, cacert_file), cacert_path)
