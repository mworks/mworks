from contextlib import contextmanager
import inspect
import multiprocessing
import os
import platform
import subprocess
import sys
import urllib.parse


################################################################################
#
# Shared configuration
#
################################################################################


def join_flags(*flags):
    return ' '.join(flags).strip()


if os.environ['PLATFORM_NAME'] in ('iphoneos', 'iphonesimulator'):
    building_for_ios = True
else:
    assert os.environ['PLATFORM_NAME'] == 'macosx'
    building_for_ios = False

assert os.environ['CURRENT_ARCH'] in os.environ['ARCHS'].split()
build_arch = platform.machine()
cross_building = building_for_ios or (os.environ['CURRENT_ARCH'] != build_arch)

assert os.environ['GCC_VERSION'] == 'com.apple.compilers.llvm.clang.1_0'
ar = os.environ['DT_TOOLCHAIN_DIR'] + '/usr/bin/ar'
cc = os.environ['DT_TOOLCHAIN_DIR'] + '/usr/bin/clang'
cxx = os.environ['DT_TOOLCHAIN_DIR'] + '/usr/bin/clang++'
make = os.environ['DEVELOPER_DIR'] + '/usr/bin/make'
rsync = '/usr/bin/rsync'
xcodebuild = os.environ['DEVELOPER_DIR'] + '/usr/bin/xcodebuild'

num_cores = str(multiprocessing.cpu_count())

common_flags = '-arch %(CURRENT_ARCH)s -isysroot %(SDKROOT)s'
if building_for_ios:
    common_flags += ' -miphoneos-version-min=%(IPHONEOS_DEPLOYMENT_TARGET)s'
else:
    common_flags += ' -mmacosx-version-min=%(MACOSX_DEPLOYMENT_TARGET)s'
common_flags %= os.environ

compile_flags = ('-g -Os -fexceptions -fvisibility=hidden ' +
                 '-Werror=unguarded-availability ' +
                 common_flags)
if os.environ['ENABLE_BITCODE'] == 'YES':
    compile_flags += {
        'bitcode': ' -fembed-bitcode',
        'marker': ' -fembed-bitcode-marker',
        }.get(os.environ['BITCODE_GENERATION_MODE'], '')

cflags = '-std=%(GCC_C_LANGUAGE_STANDARD)s' % os.environ
cxxflags = ('-std=%(CLANG_CXX_LANGUAGE_STANDARD)s '
            '-stdlib=%(CLANG_CXX_LIBRARY)s'
            % os.environ)

link_flags = common_flags

downloaddir = os.path.abspath('download')
patchdir = os.path.abspath('patches')
xcconfigdir = os.path.abspath('../build/xcode')
builddir = os.path.join(os.environ['TARGET_TEMP_DIR'],
                        os.environ['CURRENT_ARCH'])

prefix = os.path.join(os.environ['BUILT_PRODUCTS_DIR'],
                      os.environ['CURRENT_ARCH'])
frameworksdir = prefix + '/Frameworks'
includedir = prefix + '/include'
libdir = prefix + '/lib'
python_stdlib_dir = libdir + '/python' + os.environ['MW_PYTHON_3_VERSION']


################################################################################
#
# Build helpers
#
################################################################################


all_builders = []


def builder(func):
    argspec = inspect.getfullargspec(func)
    defaults = dict(zip(argspec[0], argspec[3] or []))
    if building_for_ios:
        if defaults.get('ios', True):
            all_builders.append(func)
    else:
        if defaults.get('macos', True):
            all_builders.append(func)
    return func


def announce(msg, *args):
    sys.stderr.write((msg + '\n') % args)
    sys.stderr.flush()


def check_call(args, **kwargs):
    announce('Running command: %s', ' '.join(repr(a) for a in args))
    cmd = subprocess.Popen(args,
                           stdout = subprocess.PIPE,
                           stderr = subprocess.STDOUT,
                           **kwargs)
    output = cmd.communicate()[0]
    if 0 != cmd.returncode:
        announce('Command exited with status %d and output:\n%s',
                 cmd.returncode,
                 output.decode())
        sys.exit(1)


@contextmanager
def workdir(path):
    old_path = os.getcwd()
    announce('Entering directory %r', path)
    os.chdir(path)
    yield
    announce('Leaving directory %r', path)
    os.chdir(old_path)


class DoneFileExists(Exception):
    pass


@contextmanager
def done_file(tag):
    filename = tag + '.done'
    if os.path.isfile(filename):
        raise DoneFileExists
    yield
    with open(filename, 'w') as fp:
        fp.write('Done!\n')


def always_download_file(url, filepath):
    check_call(['/usr/bin/curl', '-#', '-L', '-f', '-o', filepath, url])


def download_file(url, filename):
    filepath = downloaddir + '/' + filename
    if os.path.isfile(filepath):
        announce('Already downloaded file %r', filename)
    else:
        always_download_file(url, filepath)


def download_archive(url_path, filename):
    download_file(url_path + filename, filename)


def download_archive_from_sf(path, version, filename):
    url = (('http://downloads.sourceforge.net/project/%s/%s/%s'
            '?use_mirror=autoselect') % (path, version, filename))
    return download_file(url, filename)


def make_directory(path):
    if not os.path.isdir(path):
        check_call(['/bin/mkdir', '-p', path])


def make_directories(*args):
    for path in args:
        make_directory(path)


def remove_directory(path):
    if os.path.isdir(path):
        check_call(['/bin/rm', '-Rf', path])


def remove_directories(*args):
    for path in args:
        remove_directory(path)


def unpack_tarfile(filename, outputdir):
    check_call(['/usr/bin/tar', 'xf', downloaddir + '/' + filename])


def unpack_zipfile(filename, outputdir):
    check_call([
        '/usr/bin/unzip',
        '-q',
        downloaddir + '/' + filename,
        '-d', outputdir,
        ])


def apply_patch(patchfile, strip=1):
    with open(patchdir + '/' + patchfile) as fp:
        check_call(
            args = ['/usr/bin/patch', '-p%d' % strip],
            stdin = fp,
            )


def get_platform(arch):
    return '%s-apple-darwin' % {
        'arm64': 'aarch64',
        }.get(arch, arch)


def get_clean_env():
    env = os.environ.copy()

    # The presence of these can break some build tools
    env.pop('IPHONEOS_DEPLOYMENT_TARGET', None)
    env.pop('MACOSX_DEPLOYMENT_TARGET', None)
    if cross_building:
        env.pop('SDKROOT', None)

    return env


def run_b2(libraries, clean=False):
    b2_args = [
        './b2',
        #'-d', '2',  # Show actual commands run,
        '-j', num_cores,
        '--prefix=' + prefix,
        '--includedir=' + includedir,
        '--libdir=' + libdir,
        'variant=release',
        'optimization=space',
        'debug-symbols=on',
        'link=static',
        'threading=multi',
        'define=boost=mworks_boost',
        'cflags=' + compile_flags,
        'cxxflags=' + cxxflags,
        'linkflags=' + link_flags,
        ]
    b2_args += ['--with-' + l for l in libraries]
    if clean:
        b2_args.append('--clean')
    else:
        b2_args.append('install')
    check_call(b2_args)


def get_updated_env(
    extra_compile_flags = '',
    extra_cflags = '',
    extra_cxxflags = '',
    extra_ldflags = '',
    extra_cppflags = '',
    ):

    env = get_clean_env()
    env.update({
        'CC': cc,
        'CXX': cxx,
        'CFLAGS': join_flags(compile_flags, extra_compile_flags,
                             cflags, extra_cflags),
        'CXXFLAGS': join_flags(compile_flags, extra_compile_flags,
                               cxxflags, extra_cxxflags),
        'LDFLAGS': join_flags(link_flags, extra_ldflags),
        'CPPFLAGS': join_flags(common_flags, extra_cppflags),
        })
    return env


def run_make(targets=[]):
    check_call([make, '-j', num_cores] + targets)


def run_configure_and_make(
    extra_args = [],
    command = ['./configure'],
    extra_compile_flags = '',
    extra_cflags = '',
    extra_cxxflags = '',
    extra_ldflags = '',
    extra_cppflags = '',
    ):

    opts = [
        '--prefix=' + prefix,
        '--includedir=' + includedir,
        '--libdir=' + libdir,
        '--disable-dependency-tracking',
        '--disable-shared',
        '--enable-static',
        '--build=' + get_platform(build_arch),
        ]

    if cross_building:
        opts.append('--host=' + get_platform(os.environ['CURRENT_ARCH']))

    args = command + opts + extra_args

    if building_for_ios:
        # Even if the build and host architectures are the same, as they are
        # when we're building for iOS arm64 on a macOS arm64 (aka Apple
        # Silicon) system, we must run configure in cross-compilation mode in
        # order to build for iOS
        args.append('cross_compiling=yes')

    check_call(
        args = args,
        env = get_updated_env(extra_compile_flags,
                              extra_cflags,
                              extra_cxxflags,
                              extra_ldflags,
                              extra_cppflags),
        )
    
    run_make(['install'])


def add_object_files_to_libpythonall(exclude=()):
    object_files = []
    for dirpath, dirnames, filenames in os.walk('.'):
        for name in filenames:
            if name.endswith('.o') and name not in exclude:
                object_files.append(os.path.join(dirpath, name))
    check_call([
        ar,
        'rcs',
        libdir + ('/libpython%s_all.a' % os.environ['MW_PYTHON_3_VERSION']),
        ] + object_files)


################################################################################
#
# Library builders
#
################################################################################


@builder
def libffi():
    version = '3.4.3'
    srcdir = 'libffi-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/libffi/libffi/releases/download/v%s/' % version, tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            run_configure_and_make(
                extra_args = ['--enable-portable-binary'],
                extra_cflags = '-std=gnu11',
                )


@builder
def openssl():
    version = '3.0.5'
    srcdir = 'openssl-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://www.openssl.org/source/', tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            if building_for_ios:
                assert os.environ['CURRENT_ARCH'] == 'arm64'
                config_name = 'ios64-cross'
            else:
                config_name = 'darwin64-%s-cc' % os.environ['CURRENT_ARCH']

            env = get_clean_env()
            env['AR'] = ar
            env['CC'] = cc

            check_call([
                './Configure',
                config_name,
                '--prefix=' + prefix,
                'no-shared',
                join_flags(compile_flags, cflags, '-std=gnu11'),
                ],
                env = env)

            run_make(['install_dev'])


@builder
def python():
    version = '3.10.7'
    srcdir = 'Python-' + version
    tarfile = srcdir + '.tgz'

    assert version[:version.rfind('.')] == os.environ['MW_PYTHON_3_VERSION']

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://www.python.org/ftp/python/%s/' % version, tarfile)
            unpack_tarfile(tarfile, srcdir)
            with workdir(srcdir):
                apply_patch('python_cross_build.patch')
                apply_patch('python_ctypes.patch')
                apply_patch('python_static_zlib.patch')
                if building_for_ios:
                    apply_patch('python_ios_build.patch')
                    apply_patch('python_ios_disabled_modules.patch')
                    apply_patch('python_ios_fixes.patch')
                    apply_patch('python_ios_private_api.patch')
                    apply_patch('python_ios_test_fixes.patch')
                else:
                    apply_patch('python_macos_11_0_required.patch')
                    apply_patch('python_macos_disabled_modules.patch')
                    apply_patch('python_macos_test_fixes.patch')

        with workdir(srcdir):
            extra_args = [
                '--enable-optimizations',
                '--without-ensurepip',
                '--with-openssl=' + prefix,
                ]
            if cross_building:
                extra_args += [
                    '--enable-ipv6',
                    'PYTHON_FOR_BUILD=' + os.environ['MW_PYTHON_3'],
                    'ac_cv_file__dev_ptmx=no',
                    'ac_cv_file__dev_ptc=no',
                    ]
            if not building_for_ios:
                # Set MACOSX_DEPLOYMENT_TARGET, so that the correct value is
                # recorded in the installed sysconfig data
                extra_args.append('MACOSX_DEPLOYMENT_TARGET=' +
                                  os.environ['MACOSX_DEPLOYMENT_TARGET'])

            run_configure_and_make(
                extra_args = extra_args,
                # This is required to keep numpy's extension module init funcs
                # public
                extra_compile_flags = '-fvisibility=default',
                )

            add_object_files_to_libpythonall(
                exclude = ['_testembed.o', 'python.o']
                )

            # Generate list of trusted root certificates (for ssl module)
            always_download_file(
                url = 'https://mkcert.org/generate/',
                filepath = os.path.join(python_stdlib_dir, 'cacert.pem'),
                )


@builder
def numpy():
    version = '1.23.3'
    srcdir = 'numpy-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/numpy/numpy/releases/download/v%s/' % version, tarfile)
            unpack_tarfile(tarfile, srcdir)
            with workdir(srcdir):
                apply_patch('numpy_build.patch')
                apply_patch('numpy_test_fixes.patch')
                if building_for_ios:
                    apply_patch('numpy_ios_fixes.patch')
                    apply_patch('numpy_ios_test_fixes.patch')

        with workdir(srcdir):
            env = get_clean_env()
            env['PYTHONPATH'] = python_stdlib_dir

            # Don't use Accelerate, as it seems to make things worse rather
            # than better
            env['NPY_BLAS_ORDER'] = ''
            env['NPY_LAPACK_ORDER'] = ''

            if cross_building:
                env.update({
                    '_PYTHON_HOST_PLATFORM': 'darwin-%s' % os.environ['CURRENT_ARCH'],
                    '_PYTHON_SYSCONFIGDATA_NAME': '_sysconfigdata__darwin_darwin',
                    # numpy's configuration tests link test executuables using
                    # bare cc (without cflags).  Add common_flags to ensure that
                    # linking uses the correct architecture and SDK.
                    'CC': join_flags(cc, common_flags)
                    })

            check_call([
                os.environ['MW_PYTHON_3'],
                'setup.py',
                'build',
                '-j', num_cores,
                'install',
                '--prefix=' + prefix,
                # Force egg info in to a separate directory.  (Not sure why
                # including --root has this affect, but whatever.)
                '--root=/',
                ],
                env = env)

            add_object_files_to_libpythonall()

        # The numpy test suite requires hypothesis, pytest, and setuptools, so
        # install them and their dependencies (but outside of any standard
        # location, because we don't want to distribute them)
        check_call([
            os.environ['MW_PYTHON_3'],
            '-m', 'pip',
            'install',
            '--target', os.path.join(prefix, 'pytest'),
            'hypothesis',
            'pytest',
            'setuptools',
            ])


@builder
def boost():
    version = '1.80.0'
    srcdir = 'boost_' + version.replace('.', '_')
    tarfile = srcdir + '.tar.bz2'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://boostorg.jfrog.io/artifactory/main/release/%s/source/' % version, tarfile)
            unpack_tarfile(tarfile, srcdir)
            with workdir(srcdir):
                os.symlink('boost', 'mworks_boost')
                env = get_clean_env()
                if cross_building:
                    # SDKROOT must be set to compile the build system
                    env['SDKROOT'] = subprocess.check_output([
                        '/usr/bin/xcrun',
                        '--sdk', 'macosx',
                        '--show-sdk-path',
                        ]).strip()
                check_call([
                    './bootstrap.sh',
                    '--with-toolset=clang',
                    '--without-icu',
                    '--without-libraries=python',
                    ],
                    env = env,
                    )
            
        with workdir(srcdir):
            libraries = ['filesystem', 'random', 'regex', 'thread']
            if not building_for_ios:
                libraries += ['serialization']
            run_b2(libraries)

        with workdir(includedir):
            if not os.path.islink('mworks_boost'):
                os.symlink('boost', 'mworks_boost')


@builder
def zeromq():
    version = '4.3.4'
    srcdir = 'zeromq-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/zeromq/libzmq/releases/download/v%s/' % version, tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            run_configure_and_make(
                extra_args = [
                    '--disable-silent-rules',
                    '--disable-perf',
                    '--disable-curve-keygen',
                    '--disable-curve',
                    ],
                extra_ldflags = '-lc++',
                )


@builder
def msgpack():
    version = '4.1.1'
    srcdir = 'msgpack-cxx-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/msgpack/msgpack-c/releases/download/cpp-%s/' % version, tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            check_call([rsync, '-a', 'include/', includedir])


@builder
def libxslt(macos=False):
    version = '1.1.34'
    srcdir = 'libxslt-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('ftp://xmlsoft.org/libxslt/', tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            run_configure_and_make(
                extra_args = [
                    '--disable-silent-rules',
                    '--without-python',
                    '--without-crypto',
                    '--without-plugins',
                    'LIBXML_CFLAGS=-I%(SDKROOT)s/usr/include' % os.environ,
                    'LIBXML_LIBS=-L%(SDKROOT)s/usr/lib -lxml2' % os.environ,
                    ],
                )


@builder
def sqlite():
    release_year = 2022
    version = '3390300'  # 3.39.3
    srcdir = 'sqlite-autoconf-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://sqlite.org/%d/' % release_year, tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            extra_compile_flags = '-DSQLITE_DQS=0'  # Recommended as of 3.29.0
            if building_for_ios:
                extra_compile_flags = join_flags(extra_compile_flags,
                                                 '-DSQLITE_NOHAVE_SYSTEM')
            run_configure_and_make(
                extra_compile_flags = extra_compile_flags,
                )


@builder
def libusb(ios=False):
    version = '1.0.26'
    srcdir = 'libusb-' + version
    tarfile = srcdir + '.tar.bz2'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/libusb/libusb/releases/download/v%s/' % version, tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            run_configure_and_make(
                extra_args = ['--disable-silent-rules'],
                extra_cflags = '-std=gnu11',
                )


@builder
def cppunit(ios=False):
    version = '1.15.1'
    srcdir = 'cppunit-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('http://dev-www.libreoffice.org/src/', tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            run_configure_and_make(
                extra_compile_flags = '-g0',
                )


@builder
def narrative(ios=False):
    version = '0.1.2'
    srcdir = 'Narrative-' + version
    zipfile = srcdir + '.zip'

    # Xcode will create a universal binary, so build only once
    if cross_building:
        return

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive_from_sf('narrative/narrative',
                                     urllib.parse.quote(srcdir.replace('-', ' ')),
                                     zipfile)
            unpack_zipfile(zipfile, srcdir)

        with workdir('/'.join([srcdir, srcdir, 'Narrative'])):
            check_call([
                xcodebuild,
                '-project', 'Narrative.xcodeproj',
                '-configuration', 'Release',
                '-xcconfig', os.path.join(xcconfigdir, 'macOS.xcconfig'),
                'clean',
                'build',
                'INSTALL_PATH=@loader_path/../Frameworks',
                'OTHER_CFLAGS=-fno-objc-arc -fno-objc-weak -fvisibility=default',
                ])

            check_call([
                rsync,
                '-a',
                'build/Release/Narrative.framework',
                frameworksdir
                ])


################################################################################
#
# Main function
#
################################################################################


def main():
    requested_builders = sys.argv[1:]
    builder_names = set(buildfunc.__name__ for buildfunc in all_builders)

    for name in requested_builders:
        if name not in builder_names:
            announce('ERROR: invalid builder: %r', name)
            sys.exit(1)

    make_directories(downloaddir, builddir)

    with workdir(builddir):
        for buildfunc in all_builders:
            if ((not requested_builders) or
                (buildfunc.__name__ in requested_builders)):
                try:
                    buildfunc()
                except DoneFileExists:
                    pass

    if not building_for_ios:
        # Install headers
        header_installdir = os.path.join(os.environ['MW_INCLUDE_DIR'],
                                         os.environ['CURRENT_ARCH'])
        make_directory(header_installdir)
        check_call([
            rsync,
            '-a',
            '-m',
            '--exclude=ffi*.h',
            '--exclude=openssl/',
            includedir + '/',
            header_installdir,
            ])

        # Install libraries
        lib_installdir = os.path.join(os.environ['MW_LIB_DIR'],
                                      os.environ['CURRENT_ARCH'])
        make_directory(lib_installdir)
        check_call([
            rsync,
            '-a',
            '-m',
            '--exclude=cmake/',
            '--exclude=lib*.la',
            '--exclude=libcrypto.a',
            '--exclude=libffi.a',
            '--exclude=libpython*.a',
            '--exclude=libssl.a',
            '--exclude=pkgconfig/',
            '--exclude=python*/',
            libdir + '/',
            lib_installdir,
            ])

        # Install frameworks
        if os.path.isdir(frameworksdir):
            check_call([
                rsync,
                '-a',
                '-m',
                frameworksdir,
                os.environ['MW_DEVELOPER_DIR'],
                ])


if __name__ == '__main__':
    main()
