#!/bin/zsh

# Don't use setuptools' bundled distutils, as this seems to break cross builds
export SETUPTOOLS_USE_DISTUTILS=stdlib

for current_arch in ${=ARCHS}; do
    echo "\n**** Building for $current_arch ****\n"
    CURRENT_ARCH=$current_arch $MW_PYTHON_3 build_supporting_libs.py || exit
done
