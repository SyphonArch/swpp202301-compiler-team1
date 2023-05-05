#!/bin/bash

# Please modify here
ALIVE_BINARY_DIR= # ex) ../alive2/build/
CMAKE_PREFIX_PATH= # ex) ~/llvm-swpp

set -e

cmake -GNinja -Bbuild -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH} -DALIVE2_BINARY_DIR=${ALIVE_BINARY_DIR}
cmake --build build --target swpp-compiler
ctest --test-dir build
