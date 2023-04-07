#!/usr/bin/env bash

# ------------------------------------------------------------------------------

# Remove output 'docs' directory, before writing anything new into it
rm -rf ./docs/

# ------------------------------------------------------------------------------

# (Re)Make 'build' directory (to make clean build) and go into it
rm -rf ./build/
mkdir -p ./build/
cd ./build/

# ------------------------------------------------------------------------------

# NOTE: instead of usage 'CMAKE_INSTALL_PREFIX' for specifying install directory
#
#   -DCMAKE_INSTALL_PREFIX:PATH=`pwd`/../docs
#
# we use 'CMAKE_RUNTIME_OUTPUT_DIRECTORY', since in case of
# 'CMAKE_INSTALL_PREFIX' html files will be installed into <specified path>/bin/
# for example: '../docs/bin/', but we need to copy them into '../docs/' itself.
#
# Reference: https://stackoverflow.com/a/6594959/

emcmake cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=`pwd`/../docs \
    ../samples/

make VERBOSE=1 -j4
make install

# ------------------------------------------------------------------------------

# Go out of 'build' directory
cd ..

# Remove 'build' directory, since it not needed anymore
rm -rf ./build/
