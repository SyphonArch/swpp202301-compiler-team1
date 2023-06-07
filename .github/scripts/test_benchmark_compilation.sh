#!/bin/bash

LLVM_DIR=/opt/llvm/bin/
ALIVE_TV_BINARY=$(realpath "$1"/alive-tv)

cd swpp202301-benchmarks/

# Build lls
python3 build-lls.py "${LLVM_DIR}"

# Build asms and capture output
output=$(python3 build-asms.py ../build/swpp-compiler 2>&1)
exit_code=$?

if [ $exit_code -ne 0 ] || [[ $output == *"Failed to build ll from"* ]]; then
    echo "Failed to build ll using swpp-compiler."
    exit 1
fi
