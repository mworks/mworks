from contextlib import contextmanager
import functools
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


if os.environ['PLATFORM_NAME'] == 'iphoneos':
    building_for_ios = True
else:
    assert os.environ['PLATFORM_NAME'] == 'macosx'
    building_for_ios = False

all_archs = os.environ['ARCHS'].split()
current_arch = os.environ['CURRENT_ARCH']
assert current_arch in all_archs
build_arch = platform.machine()
cross_building = building_for_ios or (current_arch != build_arch)

assert os.environ['GCC_VERSION'] == 'com.apple.compilers.llvm.clang.1_0'
ar = os.environ['DT_TOOLCHAIN_DIR'] + '/usr/bin/ar'
cc = os.environ['DT_TOOLCHAIN_DIR'] + '/usr/bin/clang'
cxx = os.environ['DT_TOOLCHAIN_DIR'] + '/usr/bin/clang++'
make = os.environ['DEVELOPER_DIR'] + '/usr/bin/make'
pkgutil = '/usr/sbin/pkgutil'
rsync = '/usr/bin/rsync'
xcodebuild = os.environ['DEVELOPER_DIR'] + '/usr/bin/xcodebuild'

num_cores = str(multiprocessing.cpu_count())

common_flags = '-arch %(CURRENT_ARCH)s -isysroot %(SDKROOT)s'
if building_for_ios:
    common_flags += ' -miphoneos-version-min=%(IPHONEOS_DEPLOYMENT_TARGET)s'
else:
    common_flags += ' -mmacosx-version-min=%(MACOSX_DEPLOYMENT_TARGET)s'
common_flags %= os.environ

compile_flags = ('-g -O%(GCC_OPTIMIZATION_LEVEL)s -fexceptions ' +
                 '-fvisibility=hidden -Werror=unguarded-availability ' +
                 common_flags) % os.environ

cflags = '-std=%(GCC_C_LANGUAGE_STANDARD)s' % os.environ

cxxflags = ('-std=%(CLANG_CXX_LANGUAGE_STANDARD)s '
            '-stdlib=%(CLANG_CXX_LIBRARY)s'
            % os.environ)
cxxflags += (' -D_LIBCPP_HARDENING_MODE=_LIBCPP_HARDENING_MODE_' +
             os.environ['CLANG_CXX_STANDARD_LIBRARY_HARDENING'].upper())

link_flags = common_flags

downloaddir = os.path.abspath('download')
patchdir = os.path.abspath('patches')
xcconfigdir = os.path.abspath('../build/xcode')
builddir = os.path.join(os.environ['TARGET_TEMP_DIR'], current_arch)

prefix = os.path.join(os.environ['BUILT_PRODUCTS_DIR'], current_arch)
includedir = prefix + '/include'
libdir = prefix + '/lib'


################################################################################
#
# Build helpers
#
################################################################################


all_builders = []


def builder(func=None, *, universal=False, ios=True, macos=True):
    if func is None:
        return functools.partial(
            builder,
            universal = universal,
            ios = ios,
            macos = macos,
            )

    if universal:
        wrapped_func = func
        @functools.wraps(wrapped_func)
        def func():
            with workdir(os.path.pardir, silent=True):
                wrapped_func()

    if (ios if building_for_ios else macos):
        all_builders.append(func)

    return func


def announce(msg, *args):
    sys.stderr.write((msg + '\n') % args)
    sys.stderr.flush()


def check_call(args, *, silent=False, **kwargs):
    if not silent:
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
def workdir(path, /, *paths, create=False, silent=False):
    old_path = os.getcwd()
    path = os.path.join(path, *paths)
    if create:
        make_directory(path)
    if not silent:
        announce('Entering directory %r', path)
    os.chdir(path)
    try:
        yield
    finally:
        if not silent:
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


def download_numpy(destdir, *, arch=current_arch, install=False):
    if building_for_ios:
        platform = 'ios'
        platform_variant = 'iphoneos'
        os_version = os.environ['IPHONEOS_DEPLOYMENT_TARGET']
    else:
        platform = 'macosx'
        platform_variant = None
        os_version = os.environ['MACOSX_DEPLOYMENT_TARGET']

    os_version = '_'.join(str(v) for v in os_version.split('.')[:2])
    platform += f'_{os_version}_{arch}'
    if platform_variant:
        platform += f'_{platform_variant}'

    args = [sys.executable, '-m', 'pip']

    if install:
        args.extend(['install', '--target', destdir, '--no-compile'])
    else:
        args.extend(['download', '--dest', destdir])

    if building_for_ios:
        args.extend([
            # BeeWare provides binary wheels for iOS
            '--extra-index-url', 'https://pypi.anaconda.org/beeware/simple',
            ])

    args.extend([
        '--platform', platform,
        '--python-version', os.environ['MW_PYTHON_3_VERSION'],
        '--only-binary', ':all:',
        'numpy',
        ])

    check_call(args)


def make_directory(path):
    if not os.path.isdir(path):
        check_call(['/bin/mkdir', '-p', path])


def unpack_tarfile(filename):
    check_call(['/usr/bin/tar', 'xf', downloaddir + '/' + filename])


def unpack_zipfile(filename, outputdir):
    check_call([
        '/usr/bin/unzip',
        '-q',
        downloaddir + '/' + filename,
        '-d', outputdir,
        ])


def apply_patch(patchfile, strip=1):
    announce('Applying patch file %r', patchfile)
    with open(patchdir + '/' + patchfile) as fp:
        check_call(
            args = ['/usr/bin/patch', '-p%d' % strip],
            stdin = fp,
            silent = True,
            )


def make_symlink(source, target):
    check_call(['/bin/ln', '-fhs', source, target])


def get_arch_name(arch):
    return {
        'arm64': 'aarch64',
        }.get(arch, arch)


def get_platform(arch, ios_ok=False):
    os_name = ('ios' if building_for_ios and ios_ok else 'darwin')
    return '%s-apple-%s' % (get_arch_name(arch), os_name)


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
    *,
    extra_compile_flags = '',
    extra_cflags = '',
    extra_cxxflags = '',
    extra_ldflags = '',
    extra_cppflags = '',
    extra_env = {},
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
    env.update(extra_env)
    return env


def run_make(targets=[]):
    check_call([make, '-j', num_cores] + targets)


def run_configure_and_make(
    *,
    ios_ok = False,
    command = ['./configure'],
    extra_args = [],
    extra_compile_flags = '',
    extra_cflags = '',
    extra_cxxflags = '',
    extra_ldflags = '',
    extra_cppflags = '',
    extra_env = {},
    targets = ['install'],
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
        opts.append('--host=' + get_platform(current_arch, ios_ok))

    args = command + opts + extra_args

    if building_for_ios and not ios_ok:
        # Even if the build and host architectures are the same, as they are
        # when we're building for iOS arm64 on a macOS arm64 (aka Apple
        # Silicon) system, we must run configure in cross-compilation mode in
        # order to build for iOS
        args.append('cross_compiling=yes')

    env = get_updated_env(
        extra_compile_flags = extra_compile_flags,
        extra_cflags = extra_cflags,
        extra_cxxflags = extra_cxxflags,
        extra_ldflags = extra_ldflags,
        extra_cppflags = extra_cppflags,
        extra_env = extra_env,
        )
    check_call(args, env=env)
    run_make(targets)


################################################################################
#
# Library builders
#
################################################################################


@builder(universal=True)
def python():
    version = '3.15.2'
    srcdir = 'python-' + version
    archive = srcdir + ('-iOS-XCframework.tar.gz' if building_for_ios else
                        '-macos11.pkg')

    minor_version = version[:version.rfind('.')]
    assert minor_version == os.environ['MW_PYTHON_3_VERSION']

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://www.python.org/ftp/python/%s/' % version, archive)
            if building_for_ios:
                with workdir(srcdir, create=True):
                    unpack_tarfile(archive)
            else:
                check_call([
                    pkgutil,
                    '--expand',
                    os.path.join(downloaddir, archive),
                    srcdir,
                    ])

        if not building_for_ios:
            with workdir(srcdir):
                installers_dir = os.path.join(os.environ['MW_APP_SUPPORT_DIR'],
                                              '.installers')
                make_directory(installers_dir)
                for pkg_name in ('Python_Framework', 'Python_Install_Pip'):
                    pkg_file = pkg_name + '.pkg'
                    check_call([
                        pkgutil,
                        '--flatten',
                        pkg_file,
                        os.path.join(installers_dir, pkg_file),
                        ])
        else:
            with workdir(srcdir):
                download_numpy('numpy', install=True)
                check_call([
                    rsync,
                    '-a',
                    '--exclude', '*.a',
                    '--exclude', '*.pxd',
                    '--exclude', '*.pyi',
                    '--exclude', '*tests*',
                    'numpy/numpy',
                    f'Python.xcframework/ios-{current_arch}/lib-{current_arch}/python{minor_version}/site-packages',
                    ])
                check_call([
                    rsync,
                    '-a',
                    '--exclude', '*test*.so',
                    '--exclude', '*xx*.so',
                    '--exclude', '__pycache__',
                    '--exclude', 'idle_test/',
                    '--exclude', 'test/',
                    'Python.xcframework',
                    os.environ['BUILT_PRODUCTS_DIR'],
                    ])

            with workdir(os.environ['BUILT_PRODUCTS_DIR'], 'Python.xcframework'):
                app_path = os.path.join(os.environ['BUILT_PRODUCTS_DIR'], 'MWorks.app')

                # utils.sh expects the path to Python.xcframework to be
                # relative to the Xcode project directory
                xcframework_path = os.path.relpath(os.getcwd(),
                                                   os.environ['PROJECT_DIR'])
                env = os.environ.copy()
                env['CODESIGNING_FOLDER_PATH'] = app_path
                # utils.sh uses EXPANDED_CODE_SIGN_IDENTITY to sign the
                # extension module frameworks that it creates, but Xcode
                # doesn't set it when running this script.  Use
                # CODE_SIGN_IDENTITY instead.
                env['EXPANDED_CODE_SIGN_IDENTITY'] = env['CODE_SIGN_IDENTITY']
                check_call([
                    '/bin/sh',
                    '-c', f'''\
set -e
. build/utils.sh
install_python '{xcframework_path}' 'python/lib/python{minor_version}/site-packages'
''',
                    ],
                    env = env,
                    )

                check_call([
                    sys.executable,
                    '-m', 'compileall',
                    '-f',
                    # Prevent Python from trying to recompile source files
                    # bundled with MWorks.app
                    '--invalidation-mode', 'unchecked-hash',
                    os.path.join(app_path, 'python'),
                    ])


@builder
def boost():
    version = '1.92.0'
    srcdir = 'boost_' + version.replace('.', '_')
    tarfile = srcdir + '.tar.bz2'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://archives.boost.io/release/%s/source/' % version, tarfile)
            unpack_tarfile(tarfile)
            with workdir(srcdir):
                apply_patch('boost_exception_type_info.patch')
                make_symlink('boost', 'mworks_boost')
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
            make_symlink('boost', 'mworks_boost')


@builder
def zeromq():
    version = '4.3.5'
    srcdir = 'zeromq-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/zeromq/libzmq/releases/download/v%s/' % version, tarfile)
            unpack_tarfile(tarfile)

        with workdir(srcdir):
            run_configure_and_make(
                extra_args = [
                    '--disable-silent-rules',
                    '--disable-Werror',
                    '--disable-perf',
                    '--disable-curve-keygen',
                    '--disable-curve',
                    ],
                extra_compile_flags = '-Wno-deprecated-declarations',
                extra_ldflags = '-lc++',
                )


@builder
def msgpack():
    version = '9.0.0'
    srcdir = 'msgpack-cxx-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/msgpack/msgpack-c/releases/download/cpp-%s/' % version, tarfile)
            unpack_tarfile(tarfile)

        with workdir(srcdir):
            check_call([rsync, '-a', 'include/', includedir])


@builder(macos=False)
def libxslt():
    version = '1.1.45'
    srcdir = 'libxslt-' + version
    tarfile = srcdir + '.tar.xz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://download.gnome.org/sources/libxslt/%s/' % '.'.join(version.split('.')[:2]),
                             tarfile)
            unpack_tarfile(tarfile)

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
    release_year = 2026
    version = '3540100'  # 3.54.1
    srcdir = 'sqlite-autoconf-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://sqlite.org/%d/' % release_year, tarfile)
            unpack_tarfile(tarfile)

        with workdir(srcdir):
            extra_compile_flags = [
                '-DSQLITE_DQS=0',             # Recommended as of 3.29.0
                '-DSQLITE_STRICT_SUBTYPE=1',  # Recommended as of 3.45.0
                ]
            if building_for_ios:
                extra_compile_flags.append('-DSQLITE_NOHAVE_SYSTEM')
            run_configure_and_make(
                extra_compile_flags = join_flags(*extra_compile_flags),
                )


@builder(ios=False)
def libusb():
    version = '1.0.30'
    srcdir = 'libusb-' + version
    tarfile = srcdir + '.tar.bz2'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/libusb/libusb/releases/download/v%s/' % version, tarfile)
            unpack_tarfile(tarfile)
            with workdir(srcdir):
                apply_patch('libusb_macos_27_0_required.patch')

        with workdir(srcdir):
            run_configure_and_make(
                extra_args = ['--disable-silent-rules'],
                extra_cflags = '-std=gnu11',
                )


@builder(ios=False)
def cppunit():
    version = '1.15.1'
    srcdir = 'cppunit-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('http://dev-www.libreoffice.org/src/', tarfile)
            unpack_tarfile(tarfile)

        with workdir(srcdir):
            run_configure_and_make(
                extra_compile_flags = '-g0',
                )


@builder(universal=True, ios=False)
def narrative():
    version = '0.1.2'
    srcdir = 'Narrative-' + version
    zipfile = srcdir + '.zip'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive_from_sf('narrative/narrative',
                                     urllib.parse.quote(srcdir.replace('-', ' ')),
                                     zipfile)
            unpack_zipfile(zipfile, srcdir)

        with workdir(srcdir, srcdir, 'Narrative'):
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
                os.environ['MW_DEVELOPER_DIR'] + '/Frameworks',
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

    make_directory(downloaddir)

    with workdir(builddir, create=True):
        for buildfunc in all_builders:
            if ((not requested_builders) or
                (buildfunc.__name__ in requested_builders)):
                try:
                    buildfunc()
                except DoneFileExists:
                    pass

    if not building_for_ios:
        # Install headers
        current_arch_dir = os.path.join(os.environ['MW_INCLUDE_DIR'],
                                        current_arch)
        for arch in all_archs:
            if arch != current_arch:
                arch_dir = os.path.join(os.environ['MW_INCLUDE_DIR'], arch)
                if os.path.isdir(arch_dir) and not os.path.islink(arch_dir):
                    # Use diff to confirm that all files are identical
                    check_call([
                        '/usr/bin/diff',
                        '-r',
                        '--no-dereference',
                        arch_dir,
                        includedir,
                        ])
                    make_symlink(arch, current_arch_dir)
                    break
        else:
            check_call([
                rsync,
                '-a',
                '-m',
                includedir + '/',
                current_arch_dir,
                ])

        # Install libraries
        check_call([
            rsync,
            '-a',
            '-m',
            '--exclude', 'cmake/',
            '--exclude', 'lib*.la',
            '--exclude', 'pkgconfig/',
            libdir + '/',
            os.path.join(os.environ['MW_LIB_DIR'], current_arch),
            ])


if __name__ == '__main__':
    main()
