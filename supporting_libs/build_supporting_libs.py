from contextlib import contextmanager
import inspect
import multiprocessing
import os
import subprocess
import sys
import urllib


################################################################################
#
# Shared configuration
#
################################################################################


def join_flags(*flags):
    return ' '.join(flags).strip()


building_for_ios = False
if os.environ['PLATFORM_NAME'] in ('iphoneos', 'iphonesimulator'):
    building_for_ios = True
else:
    assert os.environ['PLATFORM_NAME'] == 'macosx'

assert os.environ['GCC_VERSION'] == 'com.apple.compilers.llvm.clang.1_0'
cc = os.environ['DT_TOOLCHAIN_DIR'] + '/usr/bin/clang'
cxx = os.environ['DT_TOOLCHAIN_DIR'] + '/usr/bin/clang++'
make = os.environ['DEVELOPER_DIR'] + '/usr/bin/make'
python = '/usr/bin/python' + os.environ['MW_PYTHON_VERSION']
rsync = '/usr/bin/rsync'
xcodebuild = os.environ['DEVELOPER_DIR'] + '/usr/bin/xcodebuild'

num_cores = str(multiprocessing.cpu_count())

common_flags = ' '.join(('-arch ' + arch) for arch in
                        os.environ['ARCHS'].split())
common_flags += ' -isysroot %(SDKROOT)s'
if building_for_ios:
    common_flags += ' -miphoneos-version-min=%(IPHONEOS_DEPLOYMENT_TARGET)s'
else:
    common_flags += ' -mmacosx-version-min=%(MACOSX_DEPLOYMENT_TARGET)s'
common_flags %= os.environ

compile_flags = ('-g -Os -fexceptions -fvisibility=hidden ' +
                 '-Werror=partial-availability ' +
                 common_flags)

cflags = '-std=%(GCC_C_LANGUAGE_STANDARD)s' % os.environ
cxxflags = ('-std=%(CLANG_CXX_LANGUAGE_STANDARD)s '
            '-stdlib=%(CLANG_CXX_LIBRARY)s'
            % os.environ)

link_flags = common_flags

downloaddir = os.path.abspath('download')
sourcedir = os.path.abspath('source')
patchdir = os.path.abspath('patches')
builddir = os.environ['TARGET_TEMP_DIR']

prefix = os.environ['TARGET_BUILD_DIR']
frameworksdir = prefix + '/Frameworks'
matlabdir = prefix + '/MATLAB'
includedir = prefix + '/include'
libdir = prefix + '/lib'


################################################################################
#
# Build helpers
#
################################################################################


all_builders = []


def builder(func):
    argspec = inspect.getargspec(func)
    defaults = dict(zip(argspec[0], argspec[3] or []))
    if building_for_ios:
        if defaults.get('ios', False):
            all_builders.append(func)
    else:
        if defaults.get('macos', True):
            all_builders.append(func)
    return func


def announce(msg, *args):
    sys.stderr.write((msg + '\n') % args)


def check_call(args, **kwargs):
    announce('Running command: %s', ' '.join(repr(a) for a in args))
    subprocess.check_call(args, **kwargs)


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
    filename = os.path.join(builddir, tag + '.done')
    if os.path.isfile(filename):
        raise DoneFileExists
    yield
    with open(filename, 'w') as fp:
        fp.write('Done!\n')


def download_file(url, filename):
    filepath = downloaddir + '/' + filename
    if os.path.isfile(filepath):
        announce('Already downloaded file %r', filename)
    else:
        check_call(['/usr/bin/curl', '-#', '-L', '-f', '-o', filepath, url])


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


def get_clean_env():
    env = os.environ.copy()

    # The presence of these can break some build tools
    env.pop('IPHONEOS_DEPLOYMENT_TARGET', None)
    env.pop('MACOSX_DEPLOYMENT_TARGET', None)
    if building_for_ios:
        env.pop('SDKROOT', None)

    return env


def get_updated_env(
    extra_compile_flags = '',
    extra_cflags = '',
    extra_cxxflags = '',
    extra_ldflags = '',
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
        })
    return env


def run_configure_and_make(
    extra_args = [],
    command = ['./configure'],
    extra_compile_flags = '',
    extra_cflags = '',
    extra_cxxflags = '',
    extra_ldflags = '',
    ):

    args = [
        '--prefix=' + prefix,
        '--includedir=' + includedir,
        '--libdir=' + libdir,
        '--disable-dependency-tracking',
        '--disable-shared',
        '--enable-static',
        ]

    # Force configure into cross-compilation mode when building for an
    # iOS device or simulator
    if building_for_ios:
        args.append('--host=arm-apple-darwin')

    check_call(
        args = command + args + extra_args,
        env = get_updated_env(extra_compile_flags,
                              extra_cflags,
                              extra_cxxflags,
                              extra_ldflags),
        )
    
    check_call([make, '-j', num_cores, 'install'])


################################################################################
#
# Library builders
#
################################################################################


@builder
def boost(ios=True):
    version = '1.64.0'
    srcdir = 'boost_' + version.replace('.', '_')
    tarfile = srcdir + '.tar.bz2'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive_from_sf('boost/boost', version, tarfile)
            unpack_tarfile(tarfile, srcdir)
            with workdir(srcdir):
                os.symlink('boost', 'mworks_boost')
                apply_patch('boost_clock_gettime.patch')
                check_call([
                    './bootstrap.sh',
                    '--with-toolset=clang',
                    '--without-icu',
                    '--with-python=' + python,
                    ],
                    env = get_clean_env(),
                    )
            
        with workdir(srcdir):
            b2_args = [
                './b2',
                #'-d', '2',  # Show actual commands run,
                '-j', num_cores,
                '--build-dir=' + builddir,
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
            libraries = ['filesystem', 'random', 'regex', 'system', 'thread']
            if not building_for_ios:
                libraries += ['python', 'serialization']
            b2_args += ['--with-' + l for l in libraries]
            b2_args.append('install')
            check_call(b2_args)

        with workdir(includedir):
            if not os.path.islink('mworks_boost'):
                os.symlink('boost', 'mworks_boost')


@builder
def zeromq(ios=True):
    version = '4.1.6'
    srcdir = 'zeromq-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/zeromq/zeromq4-1/releases/download/v%s/' % version, tarfile)
            unpack_tarfile(tarfile, srcdir)
            with workdir(srcdir):
                apply_patch('zeromq_clock_gettime.patch')

        zeromq_builddir = os.path.join(builddir, 'zeromq')
        make_directory(zeromq_builddir)

        with workdir(zeromq_builddir):
            run_configure_and_make(
                command = [os.path.join(sourcedir, srcdir, 'configure')],
                extra_args = ['--disable-silent-rules', '--disable-curve'],
                extra_ldflags = '-lc++',
                )


@builder
def msgpack(ios=True):
    version = '2.1.1'
    srcdir = 'msgpack-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/msgpack/msgpack-c/releases/download/cpp-%s/' % version, tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            check_call([rsync, '-a', 'include/', includedir])


@builder
def libxslt(macos=False, ios=True):
    version = '1.1.29'
    srcdir = 'libxslt-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('ftp://xmlsoft.org/libxslt/', tarfile)
            unpack_tarfile(tarfile, srcdir)

        libxslt_builddir = os.path.join(builddir, 'libxslt')
        make_directory(libxslt_builddir)

        with workdir(libxslt_builddir):
            run_configure_and_make(
                command = [os.path.join(sourcedir, srcdir, 'configure')],
                extra_args = [
                    '--disable-silent-rules',
                    '--without-python',
                    '--without-crypto',
                    '--with-libxml-include-prefix=%(SDKROOT)s/usr/include/libxml2' % os.environ,
                    '--with-libxml-libs-prefix=%(SDKROOT)s/usr/lib' % os.environ,
                    '--without-plugins',
                    ],
                )


@builder
def cppunit():
    version = '1.13.2'
    srcdir = 'cppunit-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('http://dev-www.libreoffice.org/src/', tarfile)
            unpack_tarfile(tarfile, srcdir)

        cppunit_builddir = os.path.join(builddir, 'cppunit')
        make_directory(cppunit_builddir)

        with workdir(cppunit_builddir):
            run_configure_and_make(
                command = [os.path.join(sourcedir, srcdir, 'configure')],
                extra_compile_flags = '-g0',
                )


@builder
def numpy():
    # NOTE: We need to use the version of numpy that's distributed with
    # MACOSX_DEPLOYMENT_TARGET
    version = {
        '10.10': '1.8.0',
        '10.11': '1.8.0',
        '10.12': '1.8.0',
        }[os.environ['MACOSX_DEPLOYMENT_TARGET']]
    srcdir = 'numpy-' + version
    tarfile = srcdir + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive_from_sf('numpy/NumPy', version, tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            env = get_updated_env()
            env['NPY_NUM_BUILD_JOBS'] = num_cores

            check_call([
                python,
                'setup.py',
                'install',
                '--install-lib=install',
                '--install-scripts=install',
                ],
                env=env)

            check_call([
                rsync,
                '-a',
                'install/numpy/core/include/numpy',
                includedir,
                ])


@builder
def matlab_xunit():
    version = '4.1.0'
    tag = 'matlab-xunit-'
    srcdir = tag * 2 + version
    tarfile = tag + version + '.tar.gz'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive('https://github.com/psexton/matlab-xunit/archive/', tarfile)
            unpack_tarfile(tarfile, srcdir)

        with workdir(srcdir):
            make_directory(matlabdir)
            check_call([rsync, '-a', 'src/', matlabdir + '/xunit'])


@builder
def narrative():
    version = '0.1.2'
    srcdir = 'Narrative-' + version
    zipfile = srcdir + '.zip'

    with done_file(srcdir):
        if not os.path.isdir(srcdir):
            download_archive_from_sf('narrative/narrative',
                                     urllib.quote(srcdir.replace('-', ' ')),
                                     zipfile)
            unpack_zipfile(zipfile, srcdir)

        with workdir('/'.join([srcdir, srcdir, 'Narrative'])):
            check_call([
                xcodebuild,
                '-project', 'Narrative.xcodeproj',
                '-configuration', 'Release',
                '-xcconfig', os.environ['MW_XCODE_DIR'] + '/Development.xcconfig',
                'clean',
                'build',
                'INSTALL_PATH=@loader_path/../Frameworks',
                'SDKROOT=%s' + os.environ['SDKROOT'],
                'SDKROOT_i386=$(SDKROOT)',
                'OTHER_CFLAGS=-fvisibility=default',
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
    if os.environ['ACTION'] == 'clean':
        remove_directories(sourcedir,
                           builddir,
                           frameworksdir,
                           matlabdir,
                           prefix + '/bin',
                           includedir,
                           libdir,
                           prefix + '/share')
        return

    requested_builders = sys.argv[1:]
    builder_names = set(buildfunc.__name__ for buildfunc in all_builders)

    for name in requested_builders:
        if name not in builder_names:
            announce('ERROR: invalid builder: %r', name)
            sys.exit(1)

    make_directories(downloaddir, sourcedir, builddir)

    with workdir(sourcedir):
        for buildfunc in all_builders:
            if ((not requested_builders) or
                (buildfunc.__name__ in requested_builders)):
                try:
                    buildfunc()
                except DoneFileExists:
                    pass

    if not building_for_ios:
        # Install files
        check_call([
            rsync,
            '-a',
            '--exclude=*.la',
            '--exclude=pkgconfig',
            frameworksdir,
            matlabdir,
            includedir,
            libdir,
            os.environ['MW_DEVELOPER_DIR'],
            ])


if __name__ == '__main__':
    main()
