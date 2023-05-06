#!/bin/bash

LLVM_DIR=~/llvm-swpp/bin
ALIVE_TV_BINARY=$(realpath "$1"/alive-tv)

git clone https://github.com/snu-sf-class/swpp202301-benchmarks
cd swpp202301-benchmarks/

# Build lls and ams
python3 build-lls.py ${LLVM_DIR}
python3 build-asms.py ../build/swpp-compiler 2>&1 || exit 1

# Read the CSV file (skipping the header)
readarray -t lines < <(tail -n +2 ../unit_tests/entries.csv)

# Loop through each line in the CSV file
for line in "${lines[@]}"; do
    # Parse the Pass Name and Class Name using awk
    PASS_NAME=$(echo "$line" | awk -F, '{print $3}')
    CLASS_NAME=$(echo "$line" | awk -F, '{print $2}')

    # Print the variables to check the values
    echo "Pass Name: $PASS_NAME"
    echo "Class Name: $CLASS_NAME"
    echo "---"

    for dir in *; do
        if [ -d "${dir}/src" ]; then
            # Find the .ll file
            ll_file=$(find "${dir}/src" -name "*.ll")

            echo "Checking ${ll_file}..."

            ${LLVM_DIR}/opt ${ll_file} \
                -load-pass-plugin=../build/lib${CLASS_NAME}.so \
                -passes=${PASS_NAME} \
                -S -o ${ll_file}.out

            ${ALIVE_TV_BINARY} ${ll_file} ${ll_file}.out --quiet || exit 1
        fi
    done
done
