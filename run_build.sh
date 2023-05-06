#!/bin/bash

# This file is designed for local build configurations
# Update the following variables according to your environment
ALIVE_BINARY_DIR=../alive2/build # ex) ../alive2/build/
CMAKE_PREFIX_PATH=~/llvm-swpp # ex) ~/llvm-swpp

set -e

cmake -GNinja -Bbuild -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} -DALIVE2_BINARY_DIR=${ALIVE_BINARY_DIR}
cmake --build build --target swpp-compiler
ctest --test-dir build
