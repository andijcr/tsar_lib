#!/usr/bin/env bash

set -e

SOURCE_PATH=/source
BUILD_PATH=$HOME/build

echo "Recreating build directory $BUILD_PATH"
rm -rf $BUILD_PATH && mkdir -p $BUILD_PATH
echo "Transferring the source: $SOURCE_PATH -> $BUILD_PATH"
cd $BUILD_PATH && cp -rp $SOURCE_PATH . && cd source
echo

echo "Executing cmake"
cmake .
echo
echo "Executing make..."
make
echo

echo "Executing test"
test/testTSAR
echo

